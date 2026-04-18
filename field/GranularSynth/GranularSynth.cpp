#include "daisy_field.h"
#include "daisysp.h"
#include <cmath>
#include <cstdio>

using namespace daisy;
using namespace daisysp;

namespace
{
constexpr size_t kNumVoices       = 8;
constexpr size_t kNumKnobs        = 8;
constexpr size_t kNumPlayableKeys = 13;

constexpr int   kMinOctave     = 0;
constexpr int   kMaxOctave     = 4;
constexpr int   kStartingOctave = 2;
constexpr float kOutputGain    = 0.18f;
constexpr float kSustainLevel  = 0.85f;
constexpr float kDecaySeconds  = 0.08f;

constexpr float kScale[16] = {
    0.0f,
    2.0f,
    4.0f,
    5.0f,
    7.0f,
    9.0f,
    11.0f,
    12.0f,
    0.0f,
    1.0f,
    3.0f,
    0.0f,
    6.0f,
    8.0f,
    10.0f,
    0.0f,
};

constexpr size_t kPlayableKeys[kNumPlayableKeys]
    = {0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 12, 13, 14};

constexpr size_t kPlayableKeyLeds[kNumPlayableKeys] = {
    DaisyField::LED_KEY_A1,
    DaisyField::LED_KEY_A2,
    DaisyField::LED_KEY_A3,
    DaisyField::LED_KEY_A4,
    DaisyField::LED_KEY_A5,
    DaisyField::LED_KEY_A6,
    DaisyField::LED_KEY_A7,
    DaisyField::LED_KEY_A8,
    DaisyField::LED_KEY_B2,
    DaisyField::LED_KEY_B3,
    DaisyField::LED_KEY_B5,
    DaisyField::LED_KEY_B6,
    DaisyField::LED_KEY_B7,
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

struct ControlState
{
    volatile float shape;
    volatile float formant_hz;
    volatile float bleed;
    volatile float attack_s;
    volatile float release_s;
    volatile float cutoff_hz;
    volatile float spread;
    volatile float reverb;
    volatile int   octave;
};

class GrainVoice
{
  public:
    void Init(float sample_rate)
    {
        osc_.Init(sample_rate);
        env_.Init(sample_rate);
        env_.SetSustainLevel(kSustainLevel);
        env_.SetDecayTime(kDecaySeconds);
        active_    = false;
        gate_      = false;
        key_index_ = -1;
        pan_       = 0.0f;
        age_       = 0;
    }

    void SetTimbre(float shape,
                   float formant_hz,
                   float bleed,
                   float attack_s,
                   float release_s,
                   float spread)
    {
        osc_.SetShape(shape);
        osc_.SetFormantFreq(formant_hz);
        osc_.SetBleed(bleed);
        env_.SetAttackTime(attack_s);
        env_.SetReleaseTime(release_s);

        if(key_index_ >= 0)
        {
            pan_ = KeyPan(static_cast<size_t>(key_index_), spread);
        }
    }

    void NoteOn(size_t key_index, float freq_hz, float spread, uint32_t age)
    {
        key_index_ = static_cast<int>(key_index);
        pan_       = KeyPan(key_index, spread);
        gate_      = true;
        active_    = true;
        age_       = age;
        osc_.SetFreq(freq_hz);
        env_.Retrigger(false);
    }

    void NoteOff() { gate_ = false; }

    void Process(float* left, float* right)
    {
        if(!active_)
        {
            return;
        }

        const float amp = env_.Process(gate_);
        if(!gate_ && !env_.IsRunning())
        {
            active_    = false;
            key_index_ = -1;
            return;
        }

        const float sig        = osc_.Process() * amp;
        const float left_gain  = sqrtf(0.5f * (1.0f - pan_));
        const float right_gain = sqrtf(0.5f * (1.0f + pan_));
        *left += sig * left_gain;
        *right += sig * right_gain;
    }

    bool     IsActive() const { return active_; }
    bool     MatchesKey(size_t key_index) const
    {
        return active_ && key_index_ == static_cast<int>(key_index);
    }
    uint32_t Age() const { return age_; }

  private:
    static float KeyPan(size_t key_index, float spread)
    {
        const float key_position = (static_cast<float>(key_index) / 14.0f) * 2.0f
                                   - 1.0f;
        return fclamp(key_position * spread, -1.0f, 1.0f);
    }

    GrainletOscillator osc_;
    Adsr               env_;
    bool               active_;
    bool               gate_;
    int                key_index_;
    float              pan_;
    uint32_t           age_;
};

class VoiceManager
{
  public:
    void Init(float sample_rate)
    {
        note_counter_ = 0;
        for(size_t i = 0; i < kNumVoices; ++i)
        {
            voices_[i].Init(sample_rate);
        }
    }

    void SetTimbre(float shape,
                   float formant_hz,
                   float bleed,
                   float attack_s,
                   float release_s,
                   float spread)
    {
        for(size_t i = 0; i < kNumVoices; ++i)
        {
            voices_[i].SetTimbre(
                shape, formant_hz, bleed, attack_s, release_s, spread);
        }
    }

    void NoteOn(size_t key_index, float freq_hz, float spread)
    {
        GrainVoice* voice = FindVoiceForKey(key_index);
        if(voice == nullptr)
        {
            voice = FindFreeVoice();
        }
        if(voice == nullptr)
        {
            voice = FindOldestVoice();
        }
        if(voice != nullptr)
        {
            voice->NoteOn(key_index, freq_hz, spread, ++note_counter_);
        }
    }

    void NoteOff(size_t key_index)
    {
        GrainVoice* voice = FindVoiceForKey(key_index);
        if(voice != nullptr)
        {
            voice->NoteOff();
        }
    }

    void Process(float* left, float* right)
    {
        for(size_t i = 0; i < kNumVoices; ++i)
        {
            voices_[i].Process(left, right);
        }
    }

    size_t ActiveVoiceCount() const
    {
        size_t count = 0;
        for(size_t i = 0; i < kNumVoices; ++i)
        {
            count += voices_[i].IsActive() ? 1 : 0;
        }
        return count;
    }

    bool IsKeyActive(size_t key_index) const
    {
        for(size_t i = 0; i < kNumVoices; ++i)
        {
            if(voices_[i].MatchesKey(key_index))
            {
                return true;
            }
        }
        return false;
    }

  private:
    GrainVoice* FindVoiceForKey(size_t key_index)
    {
        for(size_t i = 0; i < kNumVoices; ++i)
        {
            if(voices_[i].MatchesKey(key_index))
            {
                return &voices_[i];
            }
        }
        return nullptr;
    }

    GrainVoice* FindFreeVoice()
    {
        for(size_t i = 0; i < kNumVoices; ++i)
        {
            if(!voices_[i].IsActive())
            {
                return &voices_[i];
            }
        }
        return nullptr;
    }

    GrainVoice* FindOldestVoice()
    {
        GrainVoice* oldest = &voices_[0];
        for(size_t i = 1; i < kNumVoices; ++i)
        {
            if(voices_[i].Age() < oldest->Age())
            {
                oldest = &voices_[i];
            }
        }
        return oldest;
    }

    GrainVoice voices_[kNumVoices];
    uint32_t   note_counter_;
};

DaisyField   hw;
VoiceManager voice_manager;
ReverbSc     reverb;
Svf          lowpass_left;
Svf          lowpass_right;

ControlState controls = {
    0.35f,
    1200.0f,
    0.25f,
    0.01f,
    0.25f,
    6000.0f,
    0.4f,
    0.15f,
    kStartingOctave,
};

float knob_values[kNumKnobs] = {
    controls.shape,
    0.38f,
    controls.bleed,
    0.15f,
    0.25f,
    0.65f,
    controls.spread,
    controls.reverb,
};

float KnobToFormantHz(float value)
{
    const float normalized = value * value;
    return 120.0f + normalized * 5880.0f;
}

float KnobToAttackSeconds(float value)
{
    return 0.001f + value * value * 1.25f;
}

float KnobToReleaseSeconds(float value)
{
    return 0.02f + value * value * 2.5f;
}

float KnobToCutoffHz(float value)
{
    const float normalized = value * value;
    return 180.0f + normalized * 11820.0f;
}

float KeyToMidi(size_t key_index, int octave)
{
    return 24.0f + (12.0f * static_cast<float>(octave)) + kScale[key_index];
}

void UpdateControls()
{
    hw.ProcessDigitalControls();
    hw.ProcessAnalogControls();

    if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge())
    {
        controls.octave--;
        if(controls.octave < kMinOctave)
        {
            controls.octave = kMinOctave;
        }
    }

    if(hw.GetSwitch(DaisyField::SW_2)->RisingEdge())
    {
        controls.octave++;
        if(controls.octave > kMaxOctave)
        {
            controls.octave = kMaxOctave;
        }
    }

    for(size_t i = 0; i < kNumKnobs; ++i)
    {
        knob_values[i] = hw.GetKnobValue(i);
    }

    controls.shape      = knob_values[0];
    controls.formant_hz = KnobToFormantHz(knob_values[1]);
    controls.bleed      = knob_values[2];
    controls.attack_s   = KnobToAttackSeconds(knob_values[3]);
    controls.release_s  = KnobToReleaseSeconds(knob_values[4]);
    controls.cutoff_hz  = KnobToCutoffHz(knob_values[5]);
    controls.spread     = knob_values[6];
    controls.reverb     = knob_values[7];

    for(size_t i = 0; i < kNumPlayableKeys; ++i)
    {
        const size_t key_index = kPlayableKeys[i];
        if(hw.KeyboardRisingEdge(key_index))
        {
            const float midi_note = KeyToMidi(key_index, controls.octave);
            voice_manager.NoteOn(
                key_index, mtof(midi_note), controls.spread);
        }
        if(hw.KeyboardFallingEdge(key_index))
        {
            voice_manager.NoteOff(key_index);
        }
    }
}

void UpdateDisplay()
{
    char line[32];
    char title[] = "GranularSynth";

    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(title, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Oct %d  Voices %u",
             controls.octave,
             static_cast<unsigned>(voice_manager.ActiveVoiceCount()));
    hw.display.SetCursor(0, 8);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Shp %3d  Frm %4d",
             static_cast<int>(controls.shape * 100.0f),
             static_cast<int>(controls.formant_hz));
    hw.display.SetCursor(0, 16);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Bld %3d  Atk %3d",
             static_cast<int>(controls.bleed * 100.0f),
             static_cast<int>(controls.attack_s * 1000.0f));
    hw.display.SetCursor(0, 24);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Rel %4d  Cut %4d",
             static_cast<int>(controls.release_s * 1000.0f),
             static_cast<int>(controls.cutoff_hz));
    hw.display.SetCursor(0, 32);
    hw.display.WriteString(line, Font_6x8, true);

    snprintf(line,
             sizeof(line),
             "Spr %3d  Rev %3d",
             static_cast<int>(controls.spread * 100.0f),
             static_cast<int>(controls.reverb * 100.0f));
    hw.display.SetCursor(0, 40);
    hw.display.WriteString(line, Font_6x8, true);

    char hint[] = "SW: Octave  Keys: Notes";
    hw.display.SetCursor(0, 52);
    hw.display.WriteString(hint, Font_6x8, true);
    hw.display.Update();
}

void UpdateLeds()
{
    for(size_t i = 0; i < kNumKnobs; ++i)
    {
        hw.led_driver.SetLed(kKnobLeds[i], knob_values[i]);
    }

    for(size_t i = 0; i < kNumPlayableKeys; ++i)
    {
        const size_t key_index = kPlayableKeys[i];
        const float  level
            = voice_manager.IsKeyActive(key_index) ? 1.0f : 0.08f;
        hw.led_driver.SetLed(kPlayableKeyLeds[i], level);
    }

    hw.led_driver.SetLed(DaisyField::LED_SW_1,
                         controls.octave > kMinOctave ? 0.25f : 0.05f);
    hw.led_driver.SetLed(DaisyField::LED_SW_2,
                         controls.octave < kMaxOctave ? 0.25f : 0.05f);
    hw.led_driver.SwapBuffersAndTransmit();
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    (void)in;

    const float shape      = controls.shape;
    const float formant_hz = controls.formant_hz;
    const float bleed      = controls.bleed;
    const float attack_s   = controls.attack_s;
    const float release_s  = controls.release_s;
    const float cutoff_hz  = controls.cutoff_hz;
    const float spread     = controls.spread;
    const float reverb_amt = controls.reverb;

    voice_manager.SetTimbre(
        shape, formant_hz, bleed, attack_s, release_s, spread);
    lowpass_left.SetFreq(cutoff_hz);
    lowpass_right.SetFreq(cutoff_hz);
    reverb.SetFeedback(0.78f + (0.18f * reverb_amt));

    for(size_t i = 0; i < size; ++i)
    {
        float dry_left  = 0.0f;
        float dry_right = 0.0f;
        float wet_left  = 0.0f;
        float wet_right = 0.0f;

        voice_manager.Process(&dry_left, &dry_right);

        lowpass_left.Process(dry_left);
        lowpass_right.Process(dry_right);
        dry_left  = lowpass_left.Low();
        dry_right = lowpass_right.Low();

        reverb.Process(dry_left * reverb_amt * 0.35f,
                       dry_right * reverb_amt * 0.35f,
                       &wet_left,
                       &wet_right);

        out[0][i] = fclamp((dry_left + wet_left * reverb_amt) * kOutputGain,
                           -1.0f,
                           1.0f);
        out[1][i] = fclamp((dry_right + wet_right * reverb_amt) * kOutputGain,
                           -1.0f,
                           1.0f);
    }
}
} // namespace

int main(void)
{
    hw.Init();
    const float sample_rate = hw.AudioSampleRate();

    voice_manager.Init(sample_rate);

    lowpass_left.Init(sample_rate);
    lowpass_right.Init(sample_rate);
    lowpass_left.SetRes(0.35f);
    lowpass_right.SetRes(0.35f);
    lowpass_left.SetDrive(0.6f);
    lowpass_right.SetDrive(0.6f);

    reverb.Init(sample_rate);
    reverb.SetLpFreq(8000.0f);
    reverb.SetFeedback(0.8f);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(true)
    {
        UpdateControls();
        UpdateDisplay();
        UpdateLeds();
        System::Delay(10);
    }
}
