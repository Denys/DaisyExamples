#pragma once
// This stub checks only our source syntax and selected API shapes. Never flash this build.
// It does not emulate libDaisy, DMA, analog hardware, or ISR execution.
#include <cstddef>
#include <cstdint>
struct dsy_gpio
{
};
inline void dsy_gpio_write(dsy_gpio *, std::uint8_t) {}
struct FontDef
{
};
inline FontDef Font_6x8;
namespace daisy
{
struct SaiHandle
{
    struct Config
    {
        enum class SampleRate
        {
            SAI_48KHZ
        };
    };
};
struct AudioHandle
{
    using InputBuffer = const float *const *;
    using OutputBuffer = float **;
};
struct System
{
    static std::uint32_t GetNow()
    {
        return 0;
    }
    static std::uint32_t GetTick()
    {
        return 0;
    }
    static std::uint32_t GetTickFreq()
    {
        return 200000000;
    }
    static std::uint32_t GetSysClkFreq()
    {
        return 400000000;
    }
    static unsigned GetProgramMemoryRegion()
    {
        return 0;
    }
    static unsigned GetBootloaderVersion()
    {
        return 0;
    }
    static void Delay(std::uint32_t) {}
};
enum MidiMessageType
{
    NoteOn,
    NoteOff,
    SystemRealTime,
    ChannelMode,
    ControlChange
};
enum SystemRealTimeType
{
    Start,
    Stop,
    Reset,
    TimingClock
};
struct MidiEvent
{
    MidiMessageType type{};
    SystemRealTimeType srt_type{};
    int channel{0};
    std::uint8_t data[2]{};
};
struct MidiHandler
{
    void StartReceive() {}
    void Listen() {}
    bool HasEvents() const
    {
        return false;
    }
    MidiEvent PopEvent()
    {
        return {};
    }
};
struct Switch
{
    bool Pressed() const
    {
        return false;
    }
};
struct GPIO
{
    void Write(bool) {}
};
struct GateIn
{
    bool State()
    {
        return false;
    }
};
struct Display
{
    void SetCursor(std::uint8_t, std::uint8_t) {}
    void WriteString(const char *, FontDef, bool) {}
    void Fill(bool) {}
    void Update() {}
};
struct LedDriver
{
    void SetLed(std::size_t, std::uint8_t) {}
    void SetLed(std::size_t, float) {}
    void SwapBuffersAndTransmit() {}
};
struct DaisySeed
{
    void SetTestPoint(bool) {}
    void StartLog(bool) {}
    unsigned CheckBoardVersion()
    {
        return 0;
    }
    template <class... T> void PrintLine(const char *, T...) {}
};
struct DaisyField
{
    enum
    {
        SW_1,
        SW_2,
        SW_LAST
    };
    enum
    {
        LED_KEY_B1,
        LED_KEY_B2,
        LED_KEY_B3,
        LED_KEY_B4,
        LED_KEY_B5,
        LED_KEY_B6,
        LED_KEY_B7,
        LED_KEY_B8,
        LED_KEY_A8,
        LED_KEY_A7,
        LED_KEY_A6,
        LED_KEY_A5,
        LED_KEY_A4,
        LED_KEY_A3,
        LED_KEY_A2,
        LED_KEY_A1,
        LED_KNOB_1,
        LED_KNOB_2,
        LED_KNOB_3,
        LED_KNOB_4,
        LED_KNOB_5,
        LED_KNOB_6,
        LED_KNOB_7,
        LED_KNOB_8,
        LED_SW_1,
        LED_SW_2,
        LED_LAST
    };
    DaisySeed seed;
    Switch sw[2];
    dsy_gpio gate_out;
    GateIn gate_in;
    Display display;
    LedDriver led_driver;
    MidiHandler midi;
    void Init(bool) {}
    void SetCvOut1(std::uint16_t) {}
    void SetCvOut2(std::uint16_t) {}
    void SetAudioSampleRate(SaiHandle::Config::SampleRate) {}
    void SetAudioBlockSize(std::size_t) {}
    void StartAdc() {}
    void ProcessAnalogControls() {}
    void ProcessDigitalControls() {}
    float GetKnobValue(std::size_t)
    {
        return 0;
    }
    float GetCvValue(std::size_t)
    {
        return 0;
    }
    bool KeyboardState(std::size_t) const
    {
        return false;
    }
    void VegasMode() {}
    void StartAudio(void (*)(AudioHandle::InputBuffer, AudioHandle::OutputBuffer, std::size_t)) {}
};
} // namespace daisy
