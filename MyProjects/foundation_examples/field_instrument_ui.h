#pragma once

#include "field_defaults.h"
#include <cmath>
#include <cstdio>

namespace FieldInstrumentUI
{
using namespace daisy;
using namespace FieldDefaults;

constexpr int kInstrumentParams = 8;

class ParamZoomState
{
  public:
    void Init(float initial_value = 0.0f)
    {
        active_index_ = -1;
        active_value_ = initial_value;
        last_event_ms_ = 0;
        for(int i = 0; i < kInstrumentParams; ++i)
            last_values_[i] = initial_value;
    }

    void SetBaseline(const float values[kInstrumentParams])
    {
        for(int i = 0; i < kInstrumentParams; ++i)
            last_values_[i] = values[i];
    }

    void Capture(const float values[kInstrumentParams],
                 uint32_t    now_ms,
                 float       threshold = 0.015f)
    {
        for(int i = 0; i < kInstrumentParams; ++i)
        {
            if(fabsf(values[i] - last_values_[i]) > threshold)
            {
                active_index_ = i;
                active_value_ = values[i];
                last_event_ms_ = now_ms;
                last_values_[i] = values[i];
            }
        }
    }

    bool IsActive(uint32_t now_ms, uint32_t duration_ms = 1400) const
    {
        return active_index_ >= 0 && (now_ms - last_event_ms_) < duration_ms;
    }

    void Clear()
    {
        active_index_ = -1;
        last_event_ms_ = 0;
    }

    int ActiveIndex() const { return active_index_; }
    float ActiveValue() const { return active_value_; }

  private:
    float    last_values_[kInstrumentParams] = {};
    int      active_index_                   = -1;
    float    active_value_                   = 0.0f;
    uint32_t last_event_ms_                  = 0;
};

class OneHotKeyLedBank
{
  public:
    void Init(DaisyField* hw) { hw_ = hw; }

    void SetActiveA(int idx) { active_a_ = (idx >= 0 && idx < 8) ? idx : -1; }
    void SetActiveB(int idx) { active_b_ = (idx >= 0 && idx < 8) ? idx : -1; }
    void ClearA() { active_a_ = -1; }
    void ClearB() { active_b_ = -1; }

    void Update(float active_brightness = 1.0f, float inactive_brightness = 0.0f)
    {
        if(!hw_)
            return;

        for(int i = 0; i < 8; ++i)
        {
            hw_->led_driver.SetLed(kLedKeysA[i],
                                   i == active_a_ ? active_brightness
                                                  : inactive_brightness);
            hw_->led_driver.SetLed(kLedKeysB[i],
                                   i == active_b_ ? active_brightness
                                                  : inactive_brightness);
        }

        hw_->led_driver.SwapBuffersAndTransmit();
    }

  private:
    DaisyField* hw_      = nullptr;
    int         active_a_ = -1;
    int         active_b_ = -1;
};

enum class KeyLedState : uint8_t
{
    Off = 0,
    Blink,
    On,
};

class FieldTriStateKeyLEDs
{
  public:
    void Init(DaisyField* hw)
    {
        hw_ = hw;
        Clear();
    }

    void Clear()
    {
        for(int i = 0; i < 8; ++i)
        {
            state_a_[i] = KeyLedState::Off;
            state_b_[i] = KeyLedState::Off;
        }
    }

    void ClearA()
    {
        for(int i = 0; i < 8; ++i)
            state_a_[i] = KeyLedState::Off;
    }

    void ClearB()
    {
        for(int i = 0; i < 8; ++i)
            state_b_[i] = KeyLedState::Off;
    }

    void SetA(int idx, KeyLedState state)
    {
        if(idx >= 0 && idx < 8)
            state_a_[idx] = state;
    }

    void SetB(int idx, KeyLedState state)
    {
        if(idx >= 0 && idx < 8)
            state_b_[idx] = state;
    }

    KeyLedState GetA(int idx) const
    {
        return (idx >= 0 && idx < 8) ? state_a_[idx] : KeyLedState::Off;
    }

    KeyLedState GetB(int idx) const
    {
        return (idx >= 0 && idx < 8) ? state_b_[idx] : KeyLedState::Off;
    }

    void Update(uint32_t now_ms,
                float    on_brightness    = 1.0f,
                float    blink_brightness = 1.0f,
                uint32_t blink_period_ms   = 250)
    {
        if(!hw_)
            return;

        const bool blink_on = blink_period_ms == 0U
                                  ? true
                                  : ((now_ms / blink_period_ms) & 1U) == 0U;

        for(int i = 0; i < 8; ++i)
        {
            hw_->led_driver.SetLed(kLedKeysA[i],
                                   LedBrightness(state_a_[i],
                                                 on_brightness,
                                                 blink_brightness,
                                                 blink_on));
            hw_->led_driver.SetLed(kLedKeysB[i],
                                   LedBrightness(state_b_[i],
                                                 on_brightness,
                                                 blink_brightness,
                                                 blink_on));
        }

        hw_->led_driver.SwapBuffersAndTransmit();
    }

  private:
    static float LedBrightness(KeyLedState state,
                               float       on_brightness,
                               float       blink_brightness,
                               bool        blink_on)
    {
        switch(state)
        {
            case KeyLedState::Off: return 0.0f;
            case KeyLedState::Blink: return blink_on ? blink_brightness : 0.0f;
            case KeyLedState::On: return on_brightness;
            default: return 0.0f;
        }
    }

    DaisyField*  hw_      = nullptr;
    KeyLedState  state_a_[8] = {};
    KeyLedState  state_b_[8] = {};
};

inline const char* WaveformName(int waveform)
{
    switch(waveform)
    {
        case daisysp::Oscillator::WAVE_SIN: return "SINE";
        case daisysp::Oscillator::WAVE_TRI: return "TRI";
        case daisysp::Oscillator::WAVE_SAW: return "SAW";
        case daisysp::Oscillator::WAVE_SQUARE: return "SQUARE";
        default: return "CUSTOM";
    }
}

inline void FormatPercent(char* buffer, size_t size, float value)
{
    snprintf(buffer, size, "%d%%", static_cast<int>(value * 100.0f + 0.5f));
}

inline void FormatMilliseconds(char* buffer, size_t size, float seconds)
{
    snprintf(buffer, size, "%.0f ms", seconds * 1000.0f);
}

inline void FormatHertz(char* buffer, size_t size, float hertz)
{
    snprintf(buffer, size, "%.0f Hz", hertz);
}

inline void FormatMidiNoteName(char* buffer, size_t size, uint8_t note)
{
    static const char* kNames[12] = {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B",
    };
    const int octave = static_cast<int>(note) / 12 - 1;
    snprintf(buffer, size, "%s%d", kNames[note % 12], octave);
}

} // namespace FieldInstrumentUI
