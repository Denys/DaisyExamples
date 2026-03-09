#pragma once

namespace noisetoaster
{
enum class VcoWave
{
    Saw = 0,
    Square,
    Triangle,
};

enum class LfoWave
{
    Sine = 0,
    Square,
    Triangle,
};

enum class VcfModSource
{
    Lfo = 0,
    Areg,
    Off,
};

enum class ThreeStateLed
{
    On = 0,
    Blink,
    Off,
};

constexpr int KeyALedId(int key_index)
{
    switch(key_index)
    {
        case 0: return 15;
        case 1: return 14;
        case 2: return 13;
        case 3: return 12;
        case 4: return 11;
        case 5: return 10;
        case 6: return 9;
        case 7: return 8;
        default: return 15;
    }
}

constexpr VcoWave AdvanceVcoWave(VcoWave mode)
{
    switch(mode)
    {
        case VcoWave::Saw: return VcoWave::Square;
        case VcoWave::Square: return VcoWave::Triangle;
        case VcoWave::Triangle:
        default: return VcoWave::Saw;
    }
}

constexpr LfoWave AdvanceLfoWave(LfoWave mode)
{
    switch(mode)
    {
        case LfoWave::Sine: return LfoWave::Square;
        case LfoWave::Square: return LfoWave::Triangle;
        case LfoWave::Triangle:
        default: return LfoWave::Sine;
    }
}

constexpr VcfModSource AdvanceVcfModSource(VcfModSource source)
{
    switch(source)
    {
        case VcfModSource::Lfo: return VcfModSource::Areg;
        case VcfModSource::Areg: return VcfModSource::Off;
        case VcfModSource::Off:
        default: return VcfModSource::Lfo;
    }
}

constexpr ThreeStateLed ThreeStateForVcoWave(VcoWave mode)
{
    switch(mode)
    {
        case VcoWave::Saw: return ThreeStateLed::On;
        case VcoWave::Square: return ThreeStateLed::Blink;
        case VcoWave::Triangle:
        default: return ThreeStateLed::Off;
    }
}

constexpr ThreeStateLed ThreeStateForLfoWave(LfoWave mode)
{
    switch(mode)
    {
        case LfoWave::Sine: return ThreeStateLed::On;
        case LfoWave::Square: return ThreeStateLed::Blink;
        case LfoWave::Triangle:
        default: return ThreeStateLed::Off;
    }
}

constexpr ThreeStateLed ThreeStateForVcfModSource(VcfModSource source)
{
    switch(source)
    {
        case VcfModSource::Lfo: return ThreeStateLed::On;
        case VcfModSource::Areg: return ThreeStateLed::Blink;
        case VcfModSource::Off:
        default: return ThreeStateLed::Off;
    }
}

constexpr float LedBrightnessForThreeState(ThreeStateLed state, bool blink_phase)
{
    switch(state)
    {
        case ThreeStateLed::On: return 1.0f;
        case ThreeStateLed::Blink: return blink_phase ? 1.0f : 0.0f;
        case ThreeStateLed::Off:
        default: return 0.0f;
    }
}

constexpr bool ShouldQueueRepeatTrigger(bool note_armed,
                                        bool repeat_mode,
                                        bool areg_running,
                                        bool trigger_pending)
{
    return note_armed && repeat_mode && !areg_running && !trigger_pending;
}

inline const char* ShortName(VcoWave mode)
{
    switch(mode)
    {
        case VcoWave::Saw: return "SAW";
        case VcoWave::Square: return "SQR";
        case VcoWave::Triangle:
        default: return "TRI";
    }
}

inline const char* ShortName(LfoWave mode)
{
    switch(mode)
    {
        case LfoWave::Sine: return "SIN";
        case LfoWave::Square: return "SQR";
        case LfoWave::Triangle:
        default: return "TRI";
    }
}

inline const char* ShortName(VcfModSource source)
{
    switch(source)
    {
        case VcfModSource::Lfo: return "LFO";
        case VcfModSource::Areg: return "AR";
        case VcfModSource::Off:
        default: return "OFF";
    }
}

inline const char* KnobLabel(int index)
{
    static constexpr const char* kLabels[8] = {
        "FREQ", "V-LFO", "V-AR", "CUT", "RES", "F-MOD", "ATT", "REL"};
    return (index >= 0 && index < 8) ? kLabels[index] : "---";
}
} // namespace noisetoaster
