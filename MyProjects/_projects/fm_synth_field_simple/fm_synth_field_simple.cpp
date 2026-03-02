#include "daisy_field.h"
#include "daisysp.h"
#include <cmath>

using namespace daisy;
using namespace daisysp;

DaisyField hw;

// ========== FM VOICE CLASS ==========
class FMVoice
{
  public:
    FMVoice() {}
    ~FMVoice() {}

    void Init(float samplerate)
    {
        active_   = false;
        note_     = 0.0f;
        velocity_ = 0.0f;

        fm_synth_.Init(samplerate);
        fm_synth_.SetRatio(1.0f);
        fm_synth_.SetIndex(2.0f);

        envelope_.Init(samplerate);
        envelope_.SetTime(ADSR_SEG_ATTACK, 0.005f);   // 5ms attack
        envelope_.SetTime(ADSR_SEG_DECAY, 0.3f);      // 300ms decay
        envelope_.SetTime(ADSR_SEG_RELEASE, 0.1f);    // 100ms release
        envelope_.SetSustainLevel(0.0f);              // Percussive
    }

    float Process()
    {
        if(!active_)
            return 0.0f;

        float fm_out  = fm_synth_.Process();
        float env_val = envelope_.Process(gate_);
        float output  = fm_out * env_val * (velocity_ / 127.0f);

        // Check if envelope finished
        if(!envelope_.IsRunning() && !gate_)
            active_ = false;

        return output;
    }

    void OnNoteOn(float note, float velocity)
    {
        note_     = note;
        velocity_ = velocity;
        active_   = true;
        gate_     = true;

        float freq = mtof(note);
        fm_synth_.SetFrequency(freq);
    }

    void OnNoteOff() { gate_ = false; }

    void SetRatio(float ratio) { fm_synth_.SetRatio(ratio); }
    void SetIndex(float index) { fm_synth_.SetIndex(index); }
    void SetDecay(float decay) { envelope_.SetTime(ADSR_SEG_DECAY, decay); }

    inline bool  IsActive() const { return active_; }
    inline float GetNote() const { return note_; }

  private:
    Fm2  fm_synth_;
    Adsr envelope_;
    bool active_;
    bool gate_;
    float note_;
    float velocity_;
};

// ========== VOICE MANAGER (From Field Midi.cpp) ==========
template <size_t max_voices>
class FMVoiceManager
{
  public:
    FMVoiceManager() {}
    ~FMVoiceManager() {}

    void Init(float samplerate)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices_[i].Init(samplerate);
        }
    }

    float Process()
    {
        float sum = 0.0f;
        for(size_t i = 0; i < max_voices; i++)
        {
            sum += voices_[i].Process();
        }
        return sum;
    }

    void OnNoteOn(float notenumber, float velocity)
    {
        FMVoice* v = FindFreeVoice();
        if(v == nullptr)
            return;
        v->OnNoteOn(notenumber, velocity);
    }

    void OnNoteOff(float notenumber)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            FMVoice* v = &voices_[i];
            if(v->IsActive() && v->GetNote() == notenumber)
            {
                v->OnNoteOff();
            }
        }
    }

    void FreeAllVoices()
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices_[i].OnNoteOff();
        }
    }

    void SetRatio(float ratio)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices_[i].SetRatio(ratio);
        }
    }

    void SetIndex(float index)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices_[i].SetIndex(index);
        }
    }

    void SetDecay(float decay)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices_[i].SetDecay(decay);
        }
    }

    int CountActiveVoices() const
    {
        int count = 0;
        for(size_t i = 0; i < max_voices; i++)
        {
            if(voices_[i].IsActive())
                count++;
        }
        return count;
    }

  private:
    FMVoice  voices_[max_voices];
    FMVoice* FindFreeVoice()
    {
        FMVoice* v = nullptr;
        for(size_t i = 0; i < max_voices; i++)
        {
            if(!voices_[i].IsActive())
            {
                v = &voices_[i];
                break;
            }
        }
        return v;
    }
};

static FMVoiceManager<8> voice_manager;
static Chorus            chorus_fx;

// ========== MIDI HANDLER ==========
static MidiUartHandler midi;

void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:
        {
            NoteOnEvent p = m.AsNoteOn();
            if(p.velocity > 0)
            {
                voice_manager.OnNoteOn(p.note, p.velocity);
            }
            else
            {
                voice_manager.OnNoteOff(p.note);
            }
        }
        break;

        case NoteOff:
        {
            NoteOnEvent p = m.AsNoteOn();
            voice_manager.OnNoteOff(p.note);
        }
        break;

        case ControlChange:
        {
            ControlChangeEvent p = m.AsControlChange();
            switch(p.control_number)
            {
                case 1:  // CC1: Modulation wheel can control something
                    // Example: voice_manager.SetIndex(p.value / 12.7f);
                    break;
                default: break;
            }
        }
        break;

        default: break;
    }
}

// ========== OLED DISPLAY ==========
float    prevKnob[8], currKnob[8];
int      zoomParam     = -1;
uint32_t zoomStartTime = 0;

void CheckParameterChanges()
{
    for(int i = 0; i < 8; i++)
    {
        currKnob[i] = hw.GetKnobValue(i);

        if(fabsf(currKnob[i] - prevKnob[i]) > 0.02f)
        {
            zoomParam     = i;
            zoomStartTime = System::GetNow();
            prevKnob[i]   = currKnob[i];
        }
    }

    if(System::GetNow() - zoomStartTime > 1200)
        zoomParam = -1;
}

void DrawZoomedParameter(int param)
{
    char buf[32];
    int  percent = (int)(currKnob[param] * 100.0f);

    hw.display.SetCursor(0, 10);

    switch(param)
    {
        case 0: // K1: Ratio
        {
            float ratio_raw = currKnob[param] * 7.5f + 0.5f;
            int   ratio     = (int)floor(ratio_raw);
            sprintf(buf, "Ratio (floor)");
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            sprintf(buf, "%d%% (%d:1)", percent, ratio);
            hw.display.WriteString(buf, Font_11x18, true);
            break;
        }

        case 1: // K2: FM Index
        {
            float index = currKnob[param] * 10.0f;
            sprintf(buf, "FM Index");
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            sprintf(buf, "%d%% (%.1f)", percent, index);
            hw.display.WriteString(buf, Font_11x18, true);
            break;
        }

        case 2: // K3: Decay
        {
            float decay = currKnob[param] * 1.99f + 0.01f;
            sprintf(buf, "Decay Time");
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            sprintf(buf, "%d%% (%.2fs)", percent, decay);
            hw.display.WriteString(buf, Font_11x18, true);
            break;
        }

        case 3: // K4: Chorus LFO Depth
        {
            sprintf(buf, "Chorus Depth");
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            sprintf(buf, "%d%% (%.2f)", percent, currKnob[param]);
            hw.display.WriteString(buf, Font_11x18, true);
            break;
        }

        case 4: // K5: Chorus LFO Freq
        {
            float lfo_freq = currKnob[param] * 4.9f + 0.1f;
            sprintf(buf, "Chorus LFO Freq");
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            sprintf(buf, "%d%% (%.1fHz)", percent, lfo_freq);
            hw.display.WriteString(buf, Font_11x18, true);
            break;
        }

        case 5: // K6: Chorus Delay
        {
            float delay_ms = currKnob[param] * 45.0f + 5.0f;
            sprintf(buf, "Chorus Delay");
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            sprintf(buf, "%d%% (%.0fms)", percent, delay_ms);
            hw.display.WriteString(buf, Font_11x18, true);
            break;
        }

        case 6: // K7: Chorus Feedback
        {
            float feedback = currKnob[param] * 0.9f;
            sprintf(buf, "Chorus Feedback");
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            sprintf(buf, "%d%% (%.2f)", percent, feedback);
            hw.display.WriteString(buf, Font_11x18, true);
            break;
        }

        default:
            sprintf(buf, "Knob %d", param + 1);
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            sprintf(buf, "%d%%", percent);
            hw.display.WriteString(buf, Font_11x18, true);
            break;
    }

    int barWidth = (int)(currKnob[param] * 127.0f);
    hw.display.DrawRect(0, 50, barWidth, 58, true, true);
}

void DrawDefaultView()
{
    char buf[32];

    sprintf(buf, "FM Poly MIDI");
    hw.display.WriteString(buf, Font_7x10, true);

    hw.display.SetCursor(0, 12);
    float ratio_raw = currKnob[0] * 7.5f + 0.5f;
    int   ratio     = (int)floor(ratio_raw);
    sprintf(buf, "Ratio: %d:1", ratio);
    hw.display.WriteString(buf, Font_7x10, true);

    hw.display.SetCursor(0, 24);
    float index = currKnob[1] * 10.0f;
    sprintf(buf, "Index: %.1f", index);
    hw.display.WriteString(buf, Font_7x10, true);

    hw.display.SetCursor(0, 36);
    int voices = voice_manager.CountActiveVoices();
    sprintf(buf, "Voices: %d/8", voices);
    hw.display.WriteString(buf, Font_7x10, true);

    hw.display.SetCursor(0, 48);
    sprintf(buf, "MIDI Ready");
    hw.display.WriteString(buf, Font_7x10, true);
}

void UpdateOLED()
{
    hw.display.Fill(false);

    if(zoomParam != -1)
    {
        DrawZoomedParameter(zoomParam);
    }
    else
    {
        DrawDefaultView();
    }

    hw.display.Update();
}

// ========== AUDIO CALLBACK ==========
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    hw.ProcessAnalogControls();

    // K1: Ratio (0.5-8.0, with floor quantization)
    float ratio_raw = hw.GetKnobValue(0) * 7.5f + 0.5f;
    float ratio     = floor(ratio_raw);

    // K2: FM Index (0-10)
    float index = hw.GetKnobValue(1) * 10.0f;

    // K3: Decay time (0.01-2.0s)
    float decay = hw.GetKnobValue(2) * 1.99f + 0.01f;

    // K4-K7: Chorus parameters
    float chorus_depth    = hw.GetKnobValue(3);
    float chorus_lfo_freq = hw.GetKnobValue(4) * 4.9f + 0.1f;
    float chorus_delay    = hw.GetKnobValue(5) * 45.0f + 5.0f;
    float chorus_feedback = hw.GetKnobValue(6) * 0.9f;

    // Update voice manager
    voice_manager.SetRatio(ratio);
    voice_manager.SetIndex(index);
    voice_manager.SetDecay(decay);

    // Update chorus
    chorus_fx.SetLfoDepth(chorus_depth);
    chorus_fx.SetLfoFreq(chorus_lfo_freq);
    chorus_fx.SetDelay(chorus_delay / 1000.0f);
    chorus_fx.SetFeedback(chorus_feedback);

    // Process audio
    for(size_t i = 0; i < size; i++)
    {
        float sum        = voice_manager.Process() * 0.125f;  // Scale for 8 voices
        float chorus_out = chorus_fx.Process(sum);
        out[0][i] = out[1][i] = chorus_out;
    }
}

// ========== MAIN ==========
int main(void)
{
    float samplerate;
    hw.Init();
    samplerate = hw.AudioSampleRate();

    // Initialize voice manager
    voice_manager.Init(samplerate);

    // Initialize chorus
    chorus_fx.Init(samplerate);
    chorus_fx.SetLfoDepth(0.5f);
    chorus_fx.SetLfoFreq(0.5f);
    chorus_fx.SetDelay(0.015f);
    chorus_fx.SetFeedback(0.3f);

    // Initialize MIDI
    MidiUartHandler::Config midi_config;
    midi.Init(midi_config);
    midi.StartReceive();

    // Initialize display
    hw.display.Fill(false);
    const char str[] = "FM Poly MIDI...";
    char*      cstr  = (char*)str;
    hw.display.WriteString(cstr, Font_7x10, true);
    hw.display.Update();

    // Start audio
    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    // Initialize prev knob values
    for(int i = 0; i < 8; i++)
    {
        prevKnob[i] = hw.GetKnobValue(i);
        currKnob[i] = prevKnob[i];
    }

    // Main loop
    for(;;)
    {
        // Process MIDI messages
        midi.Listen();
        while(midi.HasEvents())
        {
            HandleMidiMessage(midi.PopEvent());
        }

        // Update OLED
        CheckParameterChanges();
        UpdateOLED();
        System::Delay(1);  // Fast loop for MIDI responsiveness
    }
}
