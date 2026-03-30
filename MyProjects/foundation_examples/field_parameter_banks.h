#pragma once

#include <cmath>
#include <cstdint>

namespace FieldParameterBanks
{
constexpr int kNumKnobs = 8;

enum class ParamBank : uint8_t
{
    Main = 0,
    Alt,
};

struct ParamBankState
{
    float values[kNumKnobs]   = {};
    bool  captured[kNumKnobs] = {};
};

class ParamBankSet
{
  public:
    void Init(float initial_value = 0.0f)
    {
        active_bank_ = ParamBank::Main;
        FillState(main_, initial_value);
        FillState(alt_, initial_value);
    }

    void SetActiveBank(ParamBank bank)
    {
        if(active_bank_ == bank)
            return;

        active_bank_ = bank;
        ClearCaptured(active_bank_);
    }
    ParamBank ActiveBank() const { return active_bank_; }

    float Read(ParamBank bank, int knob) const
    {
        return ReadState(StateFor(bank), knob);
    }

    float ReadActive(int knob) const { return Read(active_bank_, knob); }

    void Write(ParamBank bank, int knob, float value)
    {
        WriteState(StateFor(bank), knob, value);
    }

    void WriteActive(int knob, float value) { Write(active_bank_, knob, value); }

    bool IsCaptured(ParamBank bank, int knob) const
    {
        return Captured(StateFor(bank), knob);
    }

    bool IsCapturedActive(int knob) const { return IsCaptured(active_bank_, knob); }

    void SetCaptured(ParamBank bank, int knob, bool captured)
    {
        SetCapturedState(StateFor(bank), knob, captured);
    }

    void ClearCaptured(ParamBank bank) { ClearCapturedState(StateFor(bank)); }
    void ClearCapturedActive() { ClearCaptured(active_bank_); }

    bool CatchIfClose(ParamBank bank,
                      int       knob,
                      float     control_value,
                      float     threshold = 0.015f)
    {
        if(!ValidKnob(knob))
            return false;

        ParamBankState& state = StateFor(bank);
        if(fabsf(control_value - state.values[knob]) <= threshold)
        {
            state.captured[knob] = true;
            return true;
        }
        return false;
    }

    bool CatchIfCloseActive(int   knob,
                            float control_value,
                            float threshold = 0.015f)
    {
        return CatchIfClose(active_bank_, knob, control_value, threshold);
    }

  private:
    static bool ValidKnob(int knob)
    {
        return knob >= 0 && knob < kNumKnobs;
    }

    static void FillState(ParamBankState& state, float value)
    {
        for(int i = 0; i < kNumKnobs; ++i)
        {
            state.values[i]   = value;
            state.captured[i] = false;
        }
    }

    static float ReadState(const ParamBankState& state, int knob)
    {
        if(!ValidKnob(knob))
            return 0.0f;
        return state.values[knob];
    }

    static void WriteState(ParamBankState& state, int knob, float value)
    {
        if(!ValidKnob(knob))
            return;
        state.values[knob] = value;
    }

    static bool Captured(const ParamBankState& state, int knob)
    {
        return ValidKnob(knob) ? state.captured[knob] : false;
    }

    static void SetCapturedState(ParamBankState& state, int knob, bool captured)
    {
        if(!ValidKnob(knob))
            return;
        state.captured[knob] = captured;
    }

    static void ClearCapturedState(ParamBankState& state)
    {
        for(int i = 0; i < kNumKnobs; ++i)
            state.captured[i] = false;
    }

    ParamBankState& StateFor(ParamBank bank)
    {
        return bank == ParamBank::Alt ? alt_ : main_;
    }

    const ParamBankState& StateFor(ParamBank bank) const
    {
        return bank == ParamBank::Alt ? alt_ : main_;
    }

    ParamBankState main_       = {};
    ParamBankState alt_        = {};
    ParamBank     active_bank_ = ParamBank::Main;
};

} // namespace FieldParameterBanks
