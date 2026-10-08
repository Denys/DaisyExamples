#include "Controls.h"
#include "daisy_pod.h"
#include <algorithm>
#include <cmath>
#include <cstring>
using namespace daisy;
constexpr unsigned cells = 7, samples = 2048, warm = 128;
struct Result
{
    uint32_t mode, config, count, average, p999, maximum, overruns, nonzero,
        tail, freeze;
};
struct Report
{
    uint32_t magic, schema, state, cpu, uid[3], reset, uptime, heartbeat,
        callbacks, cell, block, doneMask, oracleFailures, oracleChecks,
        nonfinite, monoErrors, guardErrors, abort, scans, b1, b2, turns, pot1,
        pot2;
    Result results[cells];
};
extern "C"
{
    volatile Report g_pod_test __attribute__((section(".dtcmram_bss"), used));
}
uint32_t timings[cells][samples] __attribute__((section(".dtcmram_bss")));
float DSY_SDRAM_BSS history[2 * poddemo::kHistory + 2];
float DSY_SDRAM_BSS capture[2 * poddemo::kFreeze + 2];
DaisyPod            hw;
poddemo::Demo       demo;
poddemo::Snapshot   snapshot;
volatile uint32_t   stage = 1, block = 0,
                  cell = 0; // 0 transition, 1 measured run, 2 wait, 3 complete
uint32_t consecutive   = 0;
float    phase         = 0;
void     Publish(const poddemo::Snapshot &s)
{
    auto irq = __get_PRIMASK();
    __disable_irq();
    snapshot = s;
    __DMB();
    __set_PRIMASK(irq);
}
void Select(unsigned index)
{
    poddemo::Controls c;
    c.Init();
    poddemo::Events e;
    e.turn = index < 3 ? 0 : static_cast<int>(index - 2);
    c.Step(e);
    c.s.config   = index < 3 ? static_cast<int>(index) : 0;
    c.s.epoch    = snapshot.epoch + 1;
    c.s.slots[2] = 1;
    if(index == 6)
        c.s.command = 1;
    Publish(c.s);
}
void Callback(AudioHandle::InputBuffer, AudioHandle::OutputBuffer out, size_t n)
{
    const uint32_t start = DWT->CYCCNT;
    ++g_pod_test.callbacks;
    float      input[48]{}, left[48]{}, right[48]{};
    const auto state = stage;
    const auto b     = block;
    const auto c     = cell;
    if(state == 0 || state == 1)
    {
        if(state == 1 && b >= warm && b < warm + 1024)
            for(size_t i = 0; i < n; ++i)
            {
                input[i] = 0.05f * std::sin(phase);
                phase += 0.04f;
                if(phase > 6.2831853f)
                    phase -= 6.2831853f;
            }
        const auto s = snapshot;
        demo.Process(input, left, right, n, s);
        for(size_t i = 0; i < n; ++i)
        {
            if(!std::isfinite(left[i]) || !std::isfinite(right[i]))
                ++g_pod_test.nonfinite;
            if(left[i] != right[i])
                ++g_pod_test.monoErrors;
            if(state == 1 && b >= warm && std::abs(left[i]) > 1e-7f)
            {
                ++g_pod_test.results[c].nonzero;
                if(b >= warm + 1024)
                    ++g_pod_test.results[c].tail;
            }
        }
    }
    for(size_t i = 0; i < n; ++i)
        out[0][i] = out[1][i] = 0;
    const uint32_t elapsed = DWT->CYCCNT - start;
    if(state == 1)
    {
        if(b >= warm)
        {
            timings[c][b - warm] = elapsed;
            if(elapsed > g_pod_test.cpu / 1000)
            {
                ++g_pod_test.results[c].overruns;
                ++consecutive;
            }
            else
                consecutive = 0;
        }
        block            = b + 1;
        g_pod_test.block = b + 1;
        if(b + 1 >= warm + samples)
            stage = 2;
        if(consecutive >= 4 || g_pod_test.nonfinite || g_pod_test.monoErrors)
        {
            g_pod_test.abort = 1;
            stage            = 3;
            g_pod_test.state = 4;
        }
    }
}
void Summarize(unsigned c)
{
    auto &r  = g_pod_test.results[c];
    r.mode   = c < 3 ? 0 : c - 2;
    r.config = c < 3 ? c : 0;
    r.count  = std::min<uint32_t>(samples, block > warm ? block - warm : 0u);
    uint64_t total = 0;
    for(unsigned i = 0; i < r.count; ++i)
        total += timings[c][i];
    if(r.count)
    {
        std::sort(timings[c], timings[c] + r.count);
        r.average = static_cast<uint32_t>(total / r.count);
        r.maximum = timings[c][r.count - 1];
        r.p999    = timings[c][(r.count * 999 + 999) / 1000 - 1];
    }
    r.freeze = demo.freezeStatus.load();
}
void Oracle()
{
    poddemo::DigiMono d;
    d.Attach(history + 1, 4096);
    for(int cfg = 0; cfg < 3; ++cfg)
    {
        d.Reset();
        poddemo::DigiParams p;
        p.delay[0] = 101;
        p.delay[1] = 149;
        p.lowCutHz
            = 0; // zero-feedback integer marker oracle, without filtering
        p.config = static_cast<poddemo::Config>(cfg);
        d.Set(p);
        for(int n = 0; n < 4096; ++n)
        {
            float expected
                = cfg == 0 ? (n == 101 ? 1.f : 0)
                           : cfg == 1 ? ((n == 101 || n == 250) ? 1.f : 0)
                                      : ((n == 101 || n == 149) ? 0.5f : 0);
            float y = d.Process(n == 0 ? 1.f : 0);
            ++g_pod_test.oracleChecks;
            if(std::abs(y - expected) > 1e-6f)
                ++g_pod_test.oracleFailures;
        }
    }
}
int main()
{
    const uint32_t reset = RCC->RSR;
    hw.Init();
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    hw.SetAudioBlockSize(48);
    std::memset(const_cast<Report *>(&g_pod_test), 0, sizeof(Report));
    g_pod_test.magic  = 0x504F4432;
    g_pod_test.schema = 1;
    g_pod_test.state  = 1;
    g_pod_test.cpu    = System::GetSysClkFreq();
    g_pod_test.reset  = reset;
    for(int i = 0; i < 3; ++i)
        g_pod_test.uid[i]
            = reinterpret_cast<volatile uint32_t *>(0x1ff1e800)[i];
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    history[0] = history[2 * poddemo::kHistory + 1] = capture[0]
        = capture[2 * poddemo::kFreeze + 1]         = 12345;
    Oracle();
    Select(0);
    demo.Init(history + 1, capture + 1, snapshot);
    g_pod_test.state = 2;
    hw.StartAdc();
    hw.StartAudio(Callback);
    uint32_t last       = System::GetNow();
    bool     abortSaved = false;
    while(true)
    {
        const uint32_t now = System::GetNow();
        g_pod_test.uptime  = now;
        if(now - last < 1)
            continue;
        last = now;
        ++g_pod_test.heartbeat;
        hw.ProcessAllControls();
        ++g_pod_test.scans;
        g_pod_test.pot1
            = static_cast<uint32_t>(hw.GetKnobValue(DaisyPod::KNOB_1) * 10000);
        g_pod_test.pot2
            = static_cast<uint32_t>(hw.GetKnobValue(DaisyPod::KNOB_2) * 10000);
        if(hw.button1.RisingEdge())
            ++g_pod_test.b1;
        if(hw.button2.RisingEdge())
            ++g_pod_test.b2;
        if(hw.encoder.Increment())
            ++g_pod_test.turns;
        if(history[0] != 12345 || history[2 * poddemo::kHistory + 1] != 12345
           || capture[0] != 12345 || capture[2 * poddemo::kFreeze + 1] != 12345)
        {
            ++g_pod_test.guardErrors;
            stage            = 3;
            g_pod_test.state = 4;
        }
        if(stage == 0 && demo.Service(snapshot))
        {
            block       = 0;
            phase       = 0;
            consecutive = 0;
            stage       = 1;
        }
        if(stage == 2)
        {
            Summarize(cell);
            g_pod_test.doneMask |= 1u << cell;
            if(cell + 1 < cells)
            {
                ++cell;
                g_pod_test.cell = cell;
                Select(cell);
                stage = 0;
            }
            else
            {
                stage = 3;
                g_pod_test.state
                    = (g_pod_test.oracleFailures || g_pod_test.guardErrors) ? 4
                                                                            : 3;
            }
        }
        if(g_pod_test.abort && !abortSaved)
        {
            Summarize(cell);
            abortSaved = true;
        }
        hw.led1.Set(g_pod_test.state == 4 ? 0.2f : 0,
                    g_pod_test.state == 3 ? 0.2f : 0,
                    g_pod_test.state == 2 ? 0.2f : 0);
        hw.led2.Set(0, 0, ((now / 500) % 2) ? 0.1f : 0);
        hw.UpdateLeds();
    }
}
