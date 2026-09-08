#include "daisy_field.h"
#include "firmware/FieldAdapter.h"
#include "src/core/OutputTestArm.h"
#include "src/core/SpscQueue.h"
#include "src/dsp/Primitives.h"
#include <array>
#include <cstdio>

namespace
{
daisy::DaisyField hw;
hydrapulse::field::CallbackMeter meter;
hydrapulse::field::BootRecord boot;
hydrapulse::core::OutputTestArm output_arm;
struct TruthSnapshot
{
    std::array<float, 8> knobs{};
    std::array<float, 4> cv{};
    std::uint16_t keys{0}, seen{0}, dac1{0}, dac2{0};
    std::uint32_t cb{0}, load{0}, overruns{0}, late{0}, gate_rises{0}, drops{0};
    float peak_l{0}, peak_r{0};
    bool gate{false}, armed_output{false}, vegas{false};
};
struct KeyEvent
{
    std::uint8_t raw_index{0};
    bool down{false};
    std::uint32_t callback{0};
};
hydrapulse::core::SpscQueue<TruthSnapshot, 8> snapshots;
hydrapulse::core::SpscQueue<KeyEvent, 64> keys;
std::uint32_t blocks = 0, drops = 0, gate_rises = 0;
std::uint16_t previous_keys = 0, seen = 0;
bool previous_gate = false;
float peak_l = 0, peak_r = 0;

void AudioCallback(daisy::AudioHandle::InputBuffer input, daisy::AudioHandle::OutputBuffer output,
                   std::size_t size)
{
    meter.Begin(hw);
    const auto f = hydrapulse::field::ReadControls(hw);
    std::uint16_t raw = 0;
    for (std::uint8_t i = 0; i < 16; ++i)
    {
        if (hw.KeyboardState(i))
            raw |= std::uint16_t(1u << i);
        if (((raw ^ previous_keys) & (1u << i)) != 0)
        {
            const bool down = (raw & (1u << i)) != 0;
            if (down)
                seen |= std::uint16_t(1u << i);
            if (!keys.Push({i, down, meter.Count()}))
                ++drops;
        }
    }
    previous_keys = raw;
    const bool gate = hw.gate_in.State();
    if (gate && !previous_gate)
        ++gate_rises;
    previous_gate = gate;
    const bool test = output_arm.Update(f.play, f.shift);
    constexpr std::uint16_t codes[5] = {0, 1024, 2048, 3072, 4095};
    const auto c1 = codes[std::min(4u, unsigned(hydrapulse::dsp::Unit(f.knobs[0]) * 5.0f))];
    const auto c2 = codes[std::min(4u, unsigned(hydrapulse::dsp::Unit(f.knobs[1]) * 5.0f))];
    hw.SetCvOut1(test ? c1 : 0);
    hw.SetCvOut2(test ? c2 : 0);
    dsy_gpio_write(&hw.gate_out, test ? 1 : 0);
    for (std::size_t i = 0; i < size; ++i)
    {
        // Deliberately no gain/limiter: measure the actual passthrough path with low-level stimulus.
        output[0][i] = std::isfinite(input[0][i]) ? input[0][i] : 0;
        output[1][i] = std::isfinite(input[1][i]) ? input[1][i] : 0;
        peak_l = std::max(peak_l, std::fabs(output[0][i]));
        peak_r = std::max(peak_r, std::fabs(output[1][i]));
    }
    if (++blocks % 40u == 0)
    {
        TruthSnapshot s{};
        s.knobs = f.knobs;
        for (std::size_t i = 0; i < 4; ++i)
            s.cv[i] = hw.GetCvValue(i);
        s.keys = raw;
        s.seen = seen;
        s.dac1 = test ? c1 : 0;
        s.dac2 = test ? c2 : 0;
        s.gate = gate;
        s.armed_output = test;
        s.vegas = f.shift && !f.play;
        s.cb = meter.Count();
        s.load = meter.MaxPermille();
        s.overruns = meter.Overruns();
        s.late = meter.LateStarts();
        s.gate_rises = gate_rises;
        s.drops = drops;
        s.peak_l = peak_l;
        s.peak_r = peak_r;
        if (!snapshots.Push(s))
            ++drops;
        peak_l = peak_r = 0;
    }
    meter.End(hw);
}
void Draw(const TruthSnapshot &s)
{
    if (s.vegas)
    {
        hw.VegasMode();
        return;
    }
    char line[32];
    hw.display.Fill(false);
    hydrapulse::field::Text(hw, 0, "HYDRAPULSE FIELD TRUTH");
    std::snprintf(line, sizeof(line), "KEY:%04X SEEN:%04X", unsigned(s.keys), unsigned(s.seen));
    hydrapulse::field::Text(hw, 1, line);
    std::snprintf(line, sizeof(line), "GATE:%u EDGES:%lu", unsigned(s.gate),
                  static_cast<unsigned long>(s.gate_rises));
    hydrapulse::field::Text(hw, 2, line);
    std::snprintf(line, sizeof(line), "DAC:%04u %04u", unsigned(s.dac1), unsigned(s.dac2));
    hydrapulse::field::Text(hw, 3, line);
    hydrapulse::field::Text(hw, 4, s.armed_output ? "OUTPUT TEST ACTIVE" : "OUTPUTS OFF");
    std::snprintf(line, sizeof(line), "CPUmax %u.%u%%", unsigned(s.load / 10u), unsigned(s.load % 10u));
    hydrapulse::field::Text(hw, 5, line);
    hydrapulse::field::Text(hw, 6, "SW1 HOLD OUTPUT TEST");
    hydrapulse::field::Text(hw, 7, "SW2 LED/OLED TEST");
    hw.display.Update();
    for (std::size_t i = 0; i < daisy::DaisyField::LED_LAST; ++i)
        hw.led_driver.SetLed(i, 0.0f);
    for (std::size_t i = 0; i < 16; ++i)
        hw.led_driver.SetLed(hydrapulse::field::kLogicalToLed[i], (s.keys & (1u << i)) ? 0.6f : 0);
    hw.led_driver.SwapBuffersAndTransmit();
}
int Norm(float x)
{
    return std::isfinite(x) ? int(std::clamp(x, -4.0f, 4.0f) * 10000.0f) : 0;
}
} // namespace
int main()
{
    hydrapulse::field::InitHardware(hw);
    boot.Capture(hw);
    meter.Init();
    hw.StartAudio(AudioCallback);
    TruthSnapshot current{};
    std::uint32_t last_log = 0, last_boot = 0, midi_count = 0;
    unsigned last_type = 0, last_channel = 0, last_data0 = 0, last_data1 = 0;
    while (true)
    {
        hw.midi.Listen();
        for (unsigned i = 0; i < 32 && hw.midi.HasEvents(); ++i)
        {
            const auto event = hw.midi.PopEvent();
            ++midi_count;
            last_type = unsigned(event.type);
            last_channel = unsigned(event.channel);
            last_data0 = event.data[0];
            last_data1 = event.data[1];
        }
        KeyEvent key{};
        for (unsigned i = 0; i < 8 && keys.Pop(key); ++i)
            hw.seed.PrintLine("[HPF] key raw=%u down=%u cb=%lu", unsigned(key.raw_index), unsigned(key.down),
                              static_cast<unsigned long>(key.callback));
        bool updated = false;
        for (unsigned i = 0; i < 8 && snapshots.Pop(current); ++i)
            updated = true;
        if (updated)
            Draw(current);
        const auto now = daisy::System::GetNow();
        if (now - last_log >= 250u)
        {
            last_log = now;
            hw.seed.PrintLine("[HPF] truth cb=%lu seen=%u gate=%u edges=%lu",
                              static_cast<unsigned long>(current.cb), unsigned(current.seen),
                              unsigned(current.gate), static_cast<unsigned long>(current.gate_rises));
            hw.seed.PrintLine("[HPF] outputs dac1=%u dac2=%u enabled=%u", unsigned(current.dac1),
                              unsigned(current.dac2), unsigned(current.armed_output));
            hw.seed.PrintLine(
                "[HPF] timing cb=%lu load_pm=%lu over=%lu late=%lu drops=%lu",
                static_cast<unsigned long>(current.cb), static_cast<unsigned long>(current.load),
                static_cast<unsigned long>(current.overruns), static_cast<unsigned long>(current.late),
                static_cast<unsigned long>(current.drops));
            hw.seed.PrintLine("[HPF] knobs first=1 k1=%d k2=%d k3=%d k4=%d", Norm(current.knobs[0]),
                              Norm(current.knobs[1]), Norm(current.knobs[2]), Norm(current.knobs[3]));
            hw.seed.PrintLine("[HPF] knobs first=5 k5=%d k6=%d k7=%d k8=%d", Norm(current.knobs[4]),
                              Norm(current.knobs[5]), Norm(current.knobs[6]), Norm(current.knobs[7]));
            hw.seed.PrintLine("[HPF] cv c1=%d c2=%d c3=%d c4=%d peakL=%d peakR=%d", Norm(current.cv[0]),
                              Norm(current.cv[1]), Norm(current.cv[2]), Norm(current.cv[3]),
                              Norm(current.peak_l), Norm(current.peak_r));
            hw.seed.PrintLine("[HPF] midi count=%lu type=%u channel0=%u d0=%u d1=%u",
                              static_cast<unsigned long>(midi_count), last_type, last_channel, last_data0,
                              last_data1);
        }
        if (now - last_boot >= 2000u)
        {
            last_boot = now;
            boot.Print(hw, "truth");
        }
        daisy::System::Delay(1);
    }
}
