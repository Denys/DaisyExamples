#pragma once
#include "daisy_field.h"
#include "src/app/Types.h"
#include "util/CpuLoadMeter.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>

#ifndef HPF_BUILD_ID
#define HPF_BUILD_ID "unrecorded"
#endif
#ifndef HPF_TIMING_TESTPOINT
#define HPF_TIMING_TESTPOINT 1
#endif

namespace hydrapulse::field
{
static_assert(sizeof(daisy::MidiMessageType) == sizeof(int),
              "Use default-width BSP enums consistently for app and libDaisy");
constexpr std::uint32_t kSampleRate = 48000, kBlockSize = 48;
// Provisional source-order mapping. P0 must qualify the physical mapping on the actual Field.
constexpr std::array<std::uint8_t, 16> kLogicalToBspKey{
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};
constexpr std::array<std::uint8_t, 16> kLogicalToLed{
    {daisy::DaisyField::LED_KEY_A1, daisy::DaisyField::LED_KEY_A2, daisy::DaisyField::LED_KEY_A3,
     daisy::DaisyField::LED_KEY_A4, daisy::DaisyField::LED_KEY_A5, daisy::DaisyField::LED_KEY_A6,
     daisy::DaisyField::LED_KEY_A7, daisy::DaisyField::LED_KEY_A8, daisy::DaisyField::LED_KEY_B1,
     daisy::DaisyField::LED_KEY_B2, daisy::DaisyField::LED_KEY_B3, daisy::DaisyField::LED_KEY_B4,
     daisy::DaisyField::LED_KEY_B5, daisy::DaisyField::LED_KEY_B6, daisy::DaisyField::LED_KEY_B7,
     daisy::DaisyField::LED_KEY_B8}};
inline void OutputsOff(daisy::DaisyField &hw)
{
    hw.SetCvOut1(0);
    hw.SetCvOut2(0);
    dsy_gpio_write(&hw.gate_out, 0);
}

inline InputFrame ReadControls(daisy::DaisyField &hw)
{
    hw.ProcessAnalogControls();
    hw.ProcessDigitalControls();
    InputFrame f{};
    for (std::size_t i = 0; i < 8; ++i)
        f.knobs[i] = hw.GetKnobValue(i);
    for (std::size_t i = 0; i < 16; ++i)
        if (hw.KeyboardState(kLogicalToBspKey[i]))
            f.keys |= std::uint16_t(1u << i);
    f.play = hw.sw[daisy::DaisyField::SW_1].Pressed();
    f.shift = hw.sw[daisy::DaisyField::SW_2].Pressed();
    return f;
}
inline void Text(daisy::DaisyField &hw, std::uint8_t line, const char *text)
{
    hw.display.SetCursor(0, static_cast<std::uint8_t>(line * 8u));
    char bounded[22];
    std::snprintf(bounded, sizeof(bounded), "%.21s", text);
    hw.display.WriteString(bounded, Font_6x8, true);
}

struct BootRecord
{
    unsigned board{0}, memory{0}, bootloader{0};
    unsigned cpu_hz{0}, tick_hz{0};
    void Capture(daisy::DaisyField &hw)
    {
        board = static_cast<unsigned>(hw.seed.CheckBoardVersion());
        memory = static_cast<unsigned>(daisy::System::GetProgramMemoryRegion());
        bootloader = static_cast<unsigned>(daisy::System::GetBootloaderVersion());
        cpu_hz = daisy::System::GetSysClkFreq();
        tick_hz = daisy::System::GetTickFreq();
    }
    void Print(daisy::DaisyField &hw, const char *app) const
    {
        // This is replayable boot metadata, not an electrical reset-cause measurement.
        // Keep each record within libDaisy's documented 128-byte logger buffer.
        hw.seed.PrintLine("[HPF] boot app=%.5s build=%.12s sr=48000 block=48 reset=unknown", app,
                          HPF_BUILD_ID);
        hw.seed.PrintLine("[HPF] system board=%u memory=%u loader=%u", board, memory, bootloader);
        hw.seed.PrintLine("[HPF] timing cpu=%u tick=%u", cpu_hz, tick_hz);
    }
};

class CallbackMeter
{
  public:
    void Init()
    {
        meter_.Init(float(kSampleRate), int(kBlockSize));
        ticks_per_block_ = daisy::System::GetTickFreq() / 1000u;
    }
    void Begin(daisy::DaisyField &hw)
    {
#if HPF_TIMING_TESTPOINT
        hw.seed.SetTestPoint(true);
#endif
        const auto now = daisy::System::GetTick();
        if (count_ && now - previous_start_ > ticks_per_block_ + ticks_per_block_ / 2u)
            ++late_;
        previous_start_ = now;
        meter_.OnBlockStart();
    }
    void End(daisy::DaisyField &hw)
    {
        meter_.OnBlockEnd();
        if (daisy::System::GetTick() - previous_start_ > ticks_per_block_)
            ++overruns_;
        ++count_;
#if HPF_TIMING_TESTPOINT
        hw.seed.SetTestPoint(false);
#else
        (void)hw;
#endif
    }
    std::uint32_t Count() const
    {
        return count_;
    }
    std::uint32_t MaxPermille() const
    {
        const float load = meter_.GetMaxCpuLoad();
        return std::isfinite(load) ? static_cast<std::uint32_t>(std::clamp(load * 1000.0f, 0.0f, 1000000.0f))
                                   : 0;
    }
    std::uint32_t Overruns() const
    {
        return overruns_;
    }
    std::uint32_t LateStarts() const
    {
        return late_;
    }

  private:
    daisy::CpuLoadMeter meter_;
    std::uint32_t count_{0}, overruns_{0}, late_{0}, previous_start_{0}, ticks_per_block_{0};
};
inline void InitHardware(daisy::DaisyField &hw)
{
    hw.Init(false);
    OutputsOff(hw);
    hw.SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate::SAI_48KHZ);
    hw.SetAudioBlockSize(kBlockSize);
    hw.seed.SetTestPoint(false);
    hw.seed.StartLog(false);
    hw.StartAdc();
    // Flush the keyboard history and settle ADC filtering before constructing UI state.
    for (unsigned i = 0; i < 32; ++i)
    {
        (void)ReadControls(hw);
        daisy::System::Delay(1);
    }
    hw.midi.StartReceive();
}
} // namespace hydrapulse::field
