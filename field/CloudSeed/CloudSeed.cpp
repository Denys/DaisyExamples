#include "daisy_field.h"
#include "daisysp.h"
#include "DSP/ReverbController.h"
#include "Parameters.h"
#include "Programs.h"
#include <cstdint>
#include <new>
#include <cstdio>

using namespace daisy;
using namespace daisysp;
using namespace Cloudseed;

namespace
{
constexpr size_t kNumKnobs       = 8;
constexpr size_t kNumPages       = 2;
constexpr size_t kAudioBlockSize = BUFFER_SIZE;
constexpr float  kLedDim         = 0.06f;

enum KeyIndex
{
    KEY_A1 = 0,
    KEY_A2,
    KEY_A3,
    KEY_A4,
    KEY_A5,
    KEY_A6,
    KEY_A7,
    KEY_A8,
    KEY_B1,
    KEY_B2,
    KEY_B3,
    KEY_B4,
    KEY_B5,
    KEY_B6,
    KEY_B7,
    KEY_B8,
};

constexpr size_t kKnobLeds[kNumKnobs] = {
    DaisyField::LED_KNOB_1,
    DaisyField::LED_KNOB_2,
    DaisyField::LED_KNOB_3,
    DaisyField::LED_KNOB_4,
    DaisyField::LED_KNOB_5,
    DaisyField::LED_KNOB_6,
    DaisyField::LED_KNOB_7,
    DaisyField::LED_KNOB_8,
};

constexpr size_t kTopRowLeds[8] = {
    DaisyField::LED_KEY_A1,
    DaisyField::LED_KEY_A2,
    DaisyField::LED_KEY_A3,
    DaisyField::LED_KEY_A4,
    DaisyField::LED_KEY_A5,
    DaisyField::LED_KEY_A6,
    DaisyField::LED_KEY_A7,
    DaisyField::LED_KEY_A8,
};

constexpr size_t kBottomRowLeds[8] = {
    DaisyField::LED_KEY_B1,
    DaisyField::LED_KEY_B2,
    DaisyField::LED_KEY_B3,
    DaisyField::LED_KEY_B4,
    DaisyField::LED_KEY_B5,
    DaisyField::LED_KEY_B6,
    DaisyField::LED_KEY_B7,
    DaisyField::LED_KEY_B8,
};

struct EngineState
{
    float    mix;
    float    size;
    float    decay;
    float    diffusion;
    float    pre_delay;
    float    damping;
    float    mod_amount;
    float    mod_rate;
    bool     bypass;
    bool     early_diffuse;
    bool     late_diffuse;
    bool     low_cut;
    bool     high_cut;
    bool     interpolation;
    uint16_t seed_tap;
    uint16_t seed_diffusion;
    uint16_t seed_delay;
    uint16_t seed_postdiffusion;
    uint32_t clear_generation;
};

struct UiState
{
    EngineState engine;
    uint8_t     page;
};

class CloudSeedFieldEngine
{
  public:
    explicit CloudSeedFieldEngine(int sample_rate) : controller_(sample_rate) {}

    void Init(int sample_rate)
    {
        controller_.SetSamplerate(sample_rate);
        controller_.ClearBuffers();
    }

    void ApplyState(const EngineState& state)
    {
        for(int i = 0; i < Parameter::COUNT; ++i)
        {
            params_[i] = ProgramDarkPlate[i];
        }

        params_[Parameter::Interpolation]       = state.interpolation ? 1.0 : 0.0;
        params_[Parameter::LowCutEnabled]       = state.low_cut ? 1.0 : 0.0;
        params_[Parameter::HighCutEnabled]      = state.high_cut ? 1.0 : 0.0;
        params_[Parameter::InputMix]            = 0.18;
        params_[Parameter::DryOut]              = 0.0;
        params_[Parameter::EarlyOut]            = 0.30 + 0.18 * state.diffusion;
        params_[Parameter::LateOut]             = 0.76;
        params_[Parameter::TapEnabled]          = 0.0;
        params_[Parameter::TapPredelay]         = state.pre_delay;
        params_[Parameter::EarlyDiffuseEnabled] = state.early_diffuse ? 1.0 : 0.0;
        params_[Parameter::EarlyDiffuseCount]   = 0.10 + 0.55 * state.diffusion;
        params_[Parameter::EarlyDiffuseDelay]   = 0.08 + 0.48 * state.diffusion;
        params_[Parameter::EarlyDiffuseModAmount] = 0.60 * state.mod_amount;
        params_[Parameter::EarlyDiffuseFeedback]  = 0.28 + 0.55 * state.diffusion;
        params_[Parameter::EarlyDiffuseModRate]   = state.mod_rate;
        params_[Parameter::LateMode]              = 1.0;
        params_[Parameter::LateLineCount]         = 0.55;
        params_[Parameter::LateDiffuseEnabled]    = state.late_diffuse ? 1.0 : 0.0;
        params_[Parameter::LateDiffuseCount]      = 0.20 + 0.65 * state.diffusion;
        params_[Parameter::LateLineSize]          = 0.10 + 0.88 * state.size;
        params_[Parameter::LateLineModAmount]     = state.mod_amount;
        params_[Parameter::LateDiffuseDelay]      = 0.06 + 0.54 * state.diffusion;
        params_[Parameter::LateDiffuseModAmount]  = 0.75 * state.mod_amount;
        params_[Parameter::LateLineDecay]         = state.decay;
        params_[Parameter::LateLineModRate]       = state.mod_rate;
        params_[Parameter::LateDiffuseFeedback]   = 0.32 + 0.56 * state.diffusion;
        params_[Parameter::LateDiffuseModRate]    = state.mod_rate;
        params_[Parameter::EqLowShelfEnabled]     = 0.0;
        params_[Parameter::EqHighShelfEnabled]    = 1.0;
        params_[Parameter::EqLowpassEnabled]      = state.high_cut ? 1.0 : 0.0;
        params_[Parameter::EqHighFreq]            = 0.35 + 0.65 * (1.0 - state.damping);
        params_[Parameter::EqCutoff]              = 0.18 + 0.80 * (1.0 - state.damping);
        params_[Parameter::EqHighGain]            = 0.45 + 0.20 * (1.0 - state.damping);
        params_[Parameter::EqCrossSeed]           = 0.60;
        params_[Parameter::LowCut]                = 0.22;
        params_[Parameter::HighCut]               = 0.18 + 0.80 * (1.0 - state.damping);
        params_[Parameter::SeedTap]               = SeedToParam(state.seed_tap);
        params_[Parameter::SeedDiffusion]         = SeedToParam(state.seed_diffusion);
        params_[Parameter::SeedDelay]             = SeedToParam(state.seed_delay);
        params_[Parameter::SeedPostDiffusion]     = SeedToParam(state.seed_postdiffusion);

        for(int i = 0; i < Parameter::COUNT; ++i)
        {
            controller_.SetParameter(i, params_[i]);
        }

        wet_mix_ = state.mix;
        bypass_  = state.bypass;

        if(state.clear_generation != last_clear_generation_)
        {
            controller_.ClearBuffers();
            last_clear_generation_ = state.clear_generation;
        }
    }

    void Process(const float* in_left,
                 const float* in_right,
                 float*       out_left,
                 float*       out_right,
                 size_t       size)
    {
        if(bypass_)
        {
            for(size_t i = 0; i < size; ++i)
            {
                out_left[i]  = in_left[i];
                out_right[i] = in_right[i];
            }
            return;
        }

        float wet_left[BUFFER_SIZE];
        float wet_right[BUFFER_SIZE];
        float dry_left[BUFFER_SIZE];
        float dry_right[BUFFER_SIZE];

        for(size_t i = 0; i < size; ++i)
        {
            dry_left[i]  = in_left[i];
            dry_right[i] = in_right[i];
        }

        controller_.Process(dry_left,
                            dry_right,
                            wet_left,
                            wet_right,
                            static_cast<int>(size));

        const float dry_mix = 1.0f - wet_mix_;
        for(size_t i = 0; i < size; ++i)
        {
            out_left[i]  = fclamp((dry_left[i] * dry_mix) + (wet_left[i] * wet_mix_),
                                  -1.0f,
                                  1.0f);
            out_right[i] = fclamp((dry_right[i] * dry_mix) + (wet_right[i] * wet_mix_),
                                  -1.0f,
                                  1.0f);
        }
    }

    void FormatCurrentParameter(int param_id, char* buffer, int buffer_len) const
    {
        FormatParameter(static_cast<float>(params_[param_id]), buffer_len, param_id, buffer);
    }

  private:
    static double SeedToParam(uint16_t seed)
    {
        return static_cast<double>(seed) / 999.0;
    }

    ReverbController controller_;
    double           params_[Parameter::COUNT] = {0.0};
    float            wet_mix_                  = 0.45f;
    bool             bypass_                   = false;
    uint32_t         last_clear_generation_    = 0;
};

DaisyField hw;

volatile uint32_t ui_sequence = 0;
UiState           ui_state    = {
    {
        0.45f,
        0.46f,
        0.63f,
        0.54f,
        0.04f,
        0.42f,
        0.28f,
        0.23f,
        false,
        false,
        true,
        true,
        false,
        true,
        334,
        185,
        218,
        365,
        0,
    },
    0,
};

alignas(CloudSeedFieldEngine) static uint8_t DSY_SDRAM_BSS engine_storage[sizeof(CloudSeedFieldEngine)];
CloudSeedFieldEngine* engine = nullptr;

uint32_t lcg_state        = 0xC10D5EEDu;
uint8_t  clear_led_pulse  = 0;
uint8_t  random_led_pulse = 0;

void WriteUiState(const UiState& next)
{
    ++ui_sequence;
    ui_state = next;
    ++ui_sequence;
}

UiState ReadUiState()
{
    UiState   snapshot;
    uint32_t  seq_a = 0;
    uint32_t  seq_b = 0;

    do
    {
        seq_a = ui_sequence;
        if(seq_a & 1u)
        {
            continue;
        }
        snapshot = ui_state;
        seq_b    = ui_sequence;
    } while(seq_a != seq_b || (seq_b & 1u));

    return snapshot;
}

uint16_t NextSeed()
{
    lcg_state = (lcg_state * 1664525u) + 1013904223u;
    return static_cast<uint16_t>(lcg_state % 1000u);
}

bool UpdateContinuous(float* target, float value)
{
    if(fabsf(*target - value) > 0.0025f)
    {
        *target = value;
        return true;
    }
    return false;
}

void RandomizeSeeds(EngineState* state)
{
    state->seed_tap           = NextSeed();
    state->seed_diffusion     = NextSeed();
    state->seed_delay         = NextSeed();
    state->seed_postdiffusion = NextSeed();
}

void UpdateControlState()
{
    UiState next  = ReadUiState();
    bool    dirty = false;

    hw.ProcessDigitalControls();
    hw.ProcessAnalogControls();

    dirty |= UpdateContinuous(&next.engine.mix, hw.GetKnobValue(0));
    dirty |= UpdateContinuous(&next.engine.size, hw.GetKnobValue(1));
    dirty |= UpdateContinuous(&next.engine.decay, hw.GetKnobValue(2));
    dirty |= UpdateContinuous(&next.engine.diffusion, hw.GetKnobValue(3));
    dirty |= UpdateContinuous(&next.engine.pre_delay, hw.GetKnobValue(4));
    dirty |= UpdateContinuous(&next.engine.damping, hw.GetKnobValue(5));
    dirty |= UpdateContinuous(&next.engine.mod_amount, hw.GetKnobValue(6));
    dirty |= UpdateContinuous(&next.engine.mod_rate, hw.GetKnobValue(7));

    if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge())
    {
        next.page = static_cast<uint8_t>((next.page + kNumPages - 1) % kNumPages);
        dirty     = true;
    }

    if(hw.GetSwitch(DaisyField::SW_2)->RisingEdge())
    {
        next.page = static_cast<uint8_t>((next.page + 1) % kNumPages);
        dirty     = true;
    }

    if(hw.KeyboardRisingEdge(KEY_A1))
    {
        next.engine.bypass = !next.engine.bypass;
        dirty              = true;
    }

    if(hw.KeyboardRisingEdge(KEY_A2))
    {
        ++next.engine.clear_generation;
        clear_led_pulse = 10;
        dirty           = true;
    }

    if(hw.KeyboardRisingEdge(KEY_A3))
    {
        next.engine.early_diffuse = !next.engine.early_diffuse;
        dirty                     = true;
    }

    if(hw.KeyboardRisingEdge(KEY_A4))
    {
        next.engine.late_diffuse = !next.engine.late_diffuse;
        dirty                    = true;
    }

    if(hw.KeyboardRisingEdge(KEY_A5))
    {
        next.engine.low_cut = !next.engine.low_cut;
        dirty               = true;
    }

    if(hw.KeyboardRisingEdge(KEY_A6))
    {
        next.engine.high_cut = !next.engine.high_cut;
        dirty                = true;
    }

    if(hw.KeyboardRisingEdge(KEY_A7))
    {
        RandomizeSeeds(&next.engine);
        ++next.engine.clear_generation;
        random_led_pulse = 12;
        dirty            = true;
    }

    if(hw.KeyboardRisingEdge(KEY_A8))
    {
        next.engine.interpolation = !next.engine.interpolation;
        dirty                     = true;
    }

    if(dirty)
    {
        WriteUiState(next);
    }

    if(clear_led_pulse > 0)
    {
        --clear_led_pulse;
    }

    if(random_led_pulse > 0)
    {
        --random_led_pulse;
    }
}

void UpdateDisplay()
{
    const UiState state = ReadUiState();
    char          line[32];
    char          value_a[16];
    char          value_b[16];

    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString((char*)"CloudSeed Field", Font_6x8, true);

    if(state.page == 0)
    {
        snprintf(line,
                 sizeof(line),
                 "Mix %3d  Size %3d",
                 static_cast<int>(state.engine.mix * 100.0f),
                 static_cast<int>(state.engine.size * 100.0f));
        hw.display.SetCursor(0, 10);
        hw.display.WriteString(line, Font_6x8, true);

        snprintf(line,
                 sizeof(line),
                 "Dec %3d  Diff %3d",
                 static_cast<int>(state.engine.decay * 100.0f),
                 static_cast<int>(state.engine.diffusion * 100.0f));
        hw.display.SetCursor(0, 20);
        hw.display.WriteString(line, Font_6x8, true);

        snprintf(line,
                 sizeof(line),
                 "Pre %3d  Damp %3d",
                 static_cast<int>(state.engine.pre_delay * 100.0f),
                 static_cast<int>(state.engine.damping * 100.0f));
        hw.display.SetCursor(0, 30);
        hw.display.WriteString(line, Font_6x8, true);

        snprintf(line,
                 sizeof(line),
                 "Mod %3d  Rate %3d",
                 static_cast<int>(state.engine.mod_amount * 100.0f),
                 static_cast<int>(state.engine.mod_rate * 100.0f));
        hw.display.SetCursor(0, 40);
        hw.display.WriteString(line, Font_6x8, true);

        snprintf(line, sizeof(line), "SW pages  A-row toggles");
        hw.display.SetCursor(0, 52);
        hw.display.WriteString(line, Font_6x8, true);
    }
    else
    {
        engine->FormatCurrentParameter(Parameter::TapPredelay, value_a, sizeof(value_a));
        engine->FormatCurrentParameter(Parameter::LateLineDecay, value_b, sizeof(value_b));
        snprintf(line, sizeof(line), "Pre %s", value_a);
        hw.display.SetCursor(0, 10);
        hw.display.WriteString(line, Font_6x8, true);
        snprintf(line, sizeof(line), "Decay %s", value_b);
        hw.display.SetCursor(0, 18);
        hw.display.WriteString(line, Font_6x8, true);

        engine->FormatCurrentParameter(Parameter::HighCut, value_a, sizeof(value_a));
        engine->FormatCurrentParameter(Parameter::LateLineModRate, value_b, sizeof(value_b));
        snprintf(line, sizeof(line), "HiCut %s", value_a);
        hw.display.SetCursor(0, 28);
        hw.display.WriteString(line, Font_6x8, true);
        snprintf(line, sizeof(line), "Rate %s", value_b);
        hw.display.SetCursor(0, 36);
        hw.display.WriteString(line, Font_6x8, true);

        snprintf(line,
                 sizeof(line),
                 "BP%d ED%d LD%d HC%d",
                 state.engine.bypass ? 1 : 0,
                 state.engine.early_diffuse ? 1 : 0,
                 state.engine.late_diffuse ? 1 : 0,
                 state.engine.high_cut ? 1 : 0);
        hw.display.SetCursor(0, 46);
        hw.display.WriteString(line, Font_6x8, true);

        snprintf(line, sizeof(line), "Seeds %03u %03u", state.engine.seed_delay, state.engine.seed_postdiffusion);
        hw.display.SetCursor(0, 54);
        hw.display.WriteString(line, Font_6x8, true);
    }

    hw.display.Update();
}

void UpdateLeds()
{
    const UiState state = ReadUiState();

    const float knob_values[kNumKnobs] = {
        state.engine.mix,
        state.engine.size,
        state.engine.decay,
        state.engine.diffusion,
        state.engine.pre_delay,
        state.engine.damping,
        state.engine.mod_amount,
        state.engine.mod_rate,
    };

    for(size_t i = 0; i < kNumKnobs; ++i)
    {
        hw.led_driver.SetLed(kKnobLeds[i], knob_values[i]);
    }

    hw.led_driver.SetLed(kTopRowLeds[0], state.engine.bypass ? 1.0f : kLedDim);
    hw.led_driver.SetLed(kTopRowLeds[1], clear_led_pulse > 0 ? 1.0f : 0.18f);
    hw.led_driver.SetLed(kTopRowLeds[2], state.engine.early_diffuse ? 1.0f : kLedDim);
    hw.led_driver.SetLed(kTopRowLeds[3], state.engine.late_diffuse ? 1.0f : kLedDim);
    hw.led_driver.SetLed(kTopRowLeds[4], state.engine.low_cut ? 1.0f : kLedDim);
    hw.led_driver.SetLed(kTopRowLeds[5], state.engine.high_cut ? 1.0f : kLedDim);
    hw.led_driver.SetLed(kTopRowLeds[6], random_led_pulse > 0 ? 1.0f : 0.18f);
    hw.led_driver.SetLed(kTopRowLeds[7], state.engine.interpolation ? 1.0f : kLedDim);

    for(size_t i = 0; i < 8; ++i)
    {
        hw.led_driver.SetLed(kBottomRowLeds[i], i == state.page ? 0.35f : 0.0f);
    }

    hw.led_driver.SetLed(DaisyField::LED_SW_1, 0.15f);
    hw.led_driver.SetLed(DaisyField::LED_SW_2, 0.15f);
    hw.led_driver.SwapBuffersAndTransmit();
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    static uint32_t applied_sequence = 0;
    const uint32_t  current_sequence = ui_sequence;
    if((current_sequence != applied_sequence) && ((current_sequence & 1u) == 0u))
    {
        const UiState snapshot = ReadUiState();
        engine->ApplyState(snapshot.engine);
        applied_sequence = ui_sequence;
    }

    float in_left[BUFFER_SIZE];
    float in_right[BUFFER_SIZE];
    float out_left[BUFFER_SIZE];
    float out_right[BUFFER_SIZE];

    for(size_t i = 0; i < size; ++i)
    {
        in_left[i]  = in[0][i];
        in_right[i] = in[1][i];
    }

    engine->Process(in_left, in_right, out_left, out_right, size);

    for(size_t i = 0; i < size; ++i)
    {
        out[0][i] = out_left[i];
        out[1][i] = out_right[i];
    }
}
} // namespace

int main(void)
{
    initPrograms();

    hw.Init();
    hw.SetAudioBlockSize(kAudioBlockSize);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    engine = new(engine_storage) CloudSeedFieldEngine(static_cast<int>(hw.AudioSampleRate()));
    engine->Init(static_cast<int>(hw.AudioSampleRate()));
    engine->ApplyState(ReadUiState().engine);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(true)
    {
        UpdateControlState();
        UpdateDisplay();
        UpdateLeds();
        System::Delay(10);
    }
}
