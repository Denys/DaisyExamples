// Daisy Pod local demonstrator. Source ownership and behavior: CONTRACT.md.
#include "Ui.h"
#include "daisy_pod.h"
using namespace daisy;

// On-target health record, read over SWD by the system check (symbol g_health).
extern "C"
{
    struct PodHealth
    {
        uint32_t magic, version;
        uint32_t oledAck, pagesSent, i2cErrors, oledInits;
        uint32_t maxLoopUs, maxI2cUs, audioMaxCycles, audioLastCycles, loops;
    };
    __attribute__((used)) volatile PodHealth g_health
        = {0x35444F50 /* "POD5" */, 0x00000500, 0, 0, 0, 0, 0, 0, 0, 0, 0};
}

namespace
{
DaisyPod            hw;
poddemo::Demo       demo;
poddemo::Controls   controls;
poddemo::Snapshot   published;
poddemo::Ui         ui;
poddemo::Frame      frame;
poddemo::PageDiff   sent;
float DSY_SDRAM_BSS history[2 * poddemo::kHistory];
float DSY_SDRAM_BSS freezeLoop[2 * poddemo::kFreeze];

// SSD1306/SSD1315 128x64 on I2C1 (SCL D11 = PB8, SDA D12 = PB9) at 400 kHz, 0x3C.
// One bounded transaction per main-loop tick: a 3-byte page address or a 32-byte
// data chunk (about 0.8 ms at 400 kHz), so controls keep their 1 kHz scan.
class OledLink
{
    static constexpr uint16_t kAddr  = 0x3C;
    static constexpr int      kChunk = 32;
    I2CHandle                 i2c_;
    bool                      ok_   = false;
    int                       page_ = -1, chunk_ = 0, errorsInRow_ = 0;
    uint32_t                  retryAt_ = 0;
    uint8_t                   page_buf_[poddemo::Frame::kW];
    uint8_t                   tx_[kChunk + 1];

    bool Send(uint8_t *data, uint16_t size)
    {
        const uint32_t t0 = System::GetUs();
        const bool     ok = i2c_.TransmitBlocking(kAddr, data, size, 2)
                        == I2CHandle::Result::OK;
        const uint32_t dt = System::GetUs() - t0;
        if(dt > g_health.maxI2cUs)
            g_health.maxI2cUs = dt;
        if(!ok)
            ++g_health.i2cErrors;
        return ok;
    }
    bool Configure()
    {
        // Page addressing; charge pump on; segment/COM remap as libDaisy's SSD130x init.
        static uint8_t init[]
            = {0x00, 0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
               0x8D, 0x14, 0x20, 0x02, 0xA1, 0xC8, 0xDA, 0x12, 0x81,
               0x8F, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0xAF};
        ++g_health.oledInits;
        return Send(init, sizeof(init));
    }

  public:
    void Init()
    {
        I2CHandle::Config c;
        c.periph         = I2CHandle::Config::Peripheral::I2C_1;
        c.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
        c.mode           = I2CHandle::Config::Mode::I2C_MASTER;
        c.pin_config.scl = {DSY_GPIOB, 8};
        c.pin_config.sda = {DSY_GPIOB, 9};
        i2c_.Init(c);
        ok_              = Configure();
        g_health.oledAck = ok_ ? 1 : 0;
        sent.Invalidate();
    }
    bool Ok() const { return ok_; }
    // Without a display (or after three failures in a row) retry every 2 s.
    void Service(uint32_t now)
    {
        if(!ok_)
        {
            if(now < retryAt_)
                return;
            retryAt_         = now + 2000;
            ok_              = Configure();
            g_health.oledAck = ok_ ? 1 : 0;
            if(ok_)
            {
                errorsInRow_ = 0;
                page_        = -1;
                sent.Invalidate();
            }
            return;
        }
        bool ok = true;
        if(page_ < 0)
        {
            for(int p = 0; p < poddemo::Frame::kPages && page_ < 0; ++p)
                if(sent.Dirty(frame, p))
                    page_ = p;
            if(page_ < 0)
                return;
            // Freeze this page's bytes so a re-render cannot tear what is in flight.
            std::memcpy(page_buf_,
                        frame.px + page_ * poddemo::Frame::kW,
                        poddemo::Frame::kW);
            uint8_t cmd[4]
                = {0x00, static_cast<uint8_t>(0xB0 | page_), 0x00, 0x10};
            chunk_ = 0;
            ok     = Send(cmd, sizeof(cmd));
        }
        else
        {
            tx_[0] = 0x40;
            std::memcpy(tx_ + 1, page_buf_ + chunk_ * kChunk, kChunk);
            ok = Send(tx_, sizeof(tx_));
            if(ok && ++chunk_ * kChunk >= poddemo::Frame::kW)
            {
                sent.MarkSent(page_, page_buf_);
                ++g_health.pagesSent;
                page_ = -1;
            }
        }
        if(ok)
            errorsInRow_ = 0;
        else
        {
            page_ = -1;
            sent.Invalidate();
            if(++errorsInRow_ >= 3)
            {
                ok_              = false;
                g_health.oledAck = 0;
                retryAt_         = now + 2000;
            }
        }
    }
};
OledLink oled;

// Main cannot preempt audio. Publication masks IRQs only for the fixed-size copy.
void Publish(const poddemo::Snapshot &s)
{
    const auto primask = __get_PRIMASK();
    __disable_irq();
    published = s;
    __DMB();
    __set_PRIMASK(primask);
}
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    const uint32_t c0       = DWT->CYCCNT;
    const auto     snapshot = published;
    demo.Process(in[0], out[0], out[1], size, snapshot);
    const uint32_t dc        = DWT->CYCCNT - c0;
    g_health.audioLastCycles = dc;
    if(dc > g_health.audioMaxCycles)
        g_health.audioMaxCycles = dc;
}
void Leds(bool shift)
{
    const float colors[5][3]
        = {{0, 1, 0}, {1, 0.3f, 0}, {0, 0.3f, 1}, {0.7f, 0, 1}, {0, 1, 1}};
    float b = controls.s.bypass ? 0.025f : 0.2f;
    if(controls.s.drive.on && (System::GetNow() / 250) % 2)
        b *= .4f;
    const auto *c = colors[controls.s.mode];
    hw.led1.Set(c[0] * b, c[1] * b, c[2] * b);
    if(controls.inDrive)
    {
        const float d = controls.s.drive.on ? .2f : .025f;
        hw.led2.Set(d, d, 0);
    }
    else if(demo.Paused())
        hw.led2.Set(0.2f, 0, 0);
    else if(shift)
    {
        int n = controls.s.mode == 0
                    ? controls.s.config
                    : static_cast<int>(demo.freezeStatus.load());
        n             = std::clamp(n, 0, 4);
        const auto *f = colors[n];
        hw.led2.Set(f[0] * 0.2f, f[1] * 0.2f, f[2] * 0.2f);
    }
    else
    {
        const int p = controls.page;
        hw.led2.Set(p == 1 ? 0.2f : 0, p == 0 ? 0.2f : 0, p == 2 ? 0.2f : 0);
    }
    hw.UpdateLeds();
}
} // namespace
int main()
{
    hw.Init();
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    hw.SetAudioBlockSize(poddemo::kBlock);
    controls.Init();
    demo.Init(history, freezeLoop, controls.s);
    Publish(controls.s);
    oled.Init();
    const uint32_t adcStarted = System::GetNow();
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    uint32_t last = System::GetNow(), lastRender = 0,
             lastLoopUs = System::GetUs();
    for(;;)
    {
        const auto now = System::GetNow();
        if(now - last < 1)
            continue;
        last               = now;
        const uint32_t us  = System::GetUs();
        const uint32_t gap = us - lastLoopUs;
        lastLoopUs         = us;
        if(gap > g_health.maxLoopUs && g_health.loops > 1000)
            g_health.maxLoopUs = gap;
        ++g_health.loops;
        hw.ProcessAllControls();
        poddemo::Events e;
        e.now         = now;
        e.pots[0]     = hw.GetKnobValue(DaisyPod::KNOB_1);
        e.pots[1]     = hw.GetKnobValue(DaisyPod::KNOB_2);
        e.turn        = hw.encoder.Increment();
        e.encClick    = hw.encoder.RisingEdge();
        e.b1Rise      = hw.button1.RisingEdge();
        e.b1Fall      = hw.button1.FallingEdge();
        e.b1          = hw.button1.Pressed();
        e.b2Rise      = hw.button2.RisingEdge();
        e.b2Fall      = hw.button2.FallingEdge();
        e.b2          = hw.button2.Pressed();
        e.freezeState = demo.freezeStatus.load(std::memory_order_relaxed);
        // Give ADC smoothing 100 ms to settle before arming knob movement detection.
        if(now - adcStarted >= 100)
        {
            controls.Step(e);
            Publish(controls.s);
        }
        demo.Service(controls.s);
        Leds(e.b2);
        if(oled.Ok() && now - lastRender >= 33)
        {
            lastRender = now;
            poddemo::UiStatus st;
            st.now         = now;
            st.freezeState = e.freezeState;
            st.paused      = demo.Paused();
            st.shift       = e.b2;
            ui.Render(frame, controls, st);
        }
        oled.Service(now);
    }
}
