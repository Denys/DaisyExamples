#include "daisy_field.h"
#include "daisysp.h"
#include <string>

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
        env_gate_ = false;
        note_     = 0.0f;
        velocity_ = 0.0f;
        
        // Initialize FM operators (using 2x Fm2 for 4 operators)
        fm_pair_1_.Init(samplerate);
        fm_pair_2_.Init(samplerate);
        
        // Initialize envelopes (4 total, one per operator)
        for(int i = 0; i < 4; i++)
        {
            env_[i].Init(samplerate);
            env_[i].SetTime(ADSR_SEG_ATTACK, 0.005f);
            env_[i].SetTime(ADSR_SEG_DECAY, 0.1f);
            env_[i].SetTime(ADSR_SEG_RELEASE, 0.3f);
            env_[i].SetSustainLevel(0.7f);
        }
        
        // Initialize filter
        filt_.Init(samplerate);
        filt_.SetFreq(4000.0f);
        filt_.SetRes(0.3f);
        
        // Default FM parameters
        algorithm_   = 0;
        mod_index_   = 2.0f;
        op_levels_[0] = 1.0f;
        op_levels_[1] = 0.8f;
        op_levels_[2] = 0.6f;
        op_levels_[3] = 0.4f;
    }

    float Process()
    {
        if(!active_)
            return 0.0f;

        float out = ProcessAlgorithm();
        
        // Check if voice should be deactivated
        bool any_running = false;
        for(int i = 0; i < 4; i++)
        {
            if(env_[i].IsRunning())
                any_running = true;
        }
        if(!any_running && !env_gate_)
            active_ = false;

        return out;
    }

    void OnNoteOn(float note, float velocity)
    {
        note_     = note;
        velocity_ = velocity;
        active_   = true;
        env_gate_ = true;
    }

    void OnNoteOff() { env_gate_ = false; }

    void SetAlgorithm(int algo) { algorithm_ = algo; }
    void SetModIndex(float idx) { mod_index_ = idx; }
    void SetOperatorLevel(int op, float level)
    {
        if(op >= 0 && op < 4)
            op_levels_[op] = level;
    }
    void SetCutoff(float freq) { filt_.SetFreq(freq); }
    void SetResonance(float res) { filt_.SetRes(res); }

    inline bool  IsActive() const { return active_; }
    inline float GetNote() const { return note_; }

  private:
    // FM operators
    Fm2  fm_pair_1_; // Operators 1+2
    Fm2  fm_pair_2_; // Operators 3+4
    Adsr env_[4];
    Svf  filt_;

    // Voice state
    bool  active_;
    bool  env_gate_;
    float note_;
    float velocity_;
    
    // FM parameters
    int   algorithm_;
    float mod_index_;
    float op_levels_[4];

    float ProcessAlgorithm()
    {
        float base_freq = mtof(note_);
        float out       = 0.0f;

        // Set base frequencies
        fm_pair_1_.SetFrequency(base_freq);
        fm_pair_2_.SetFrequency(base_freq);

        switch(algorithm_)
        {
            case 0: // Stack: OP1→OP2→OP3→OP4
            {
                fm_pair_1_.SetRatio(1.0f);
                fm_pair_1_.SetIndex(mod_index_);
                float op1 = fm_pair_1_.Process() * env_[0].Process(env_gate_)
                            * op_levels_[0];
                out = op1;
                break;
            }

            case 1: // Parallel: All ops to output
            {
                fm_pair_1_.SetRatio(1.0f);
                fm_pair_1_.SetIndex(0.5f);
                float op1 = fm_pair_1_.Process() * env_[0].Process(env_gate_)
                            * op_levels_[0];

                fm_pair_2_.SetRatio(2.0f);
                fm_pair_2_.SetIndex(0.5f);
                float op3 = fm_pair_2_.Process() * env_[2].Process(env_gate_)
                            * op_levels_[2];

                out = (op1 + op3) * 0.5f;
                break;
            }

            case 2: // 2+2 Split
            {
                fm_pair_1_.SetRatio(1.0f);
                fm_pair_1_.SetIndex(mod_index_ * 0.7f);
                float sig1 = fm_pair_1_.Process() * env_[0].Process(env_gate_)
                             * op_levels_[0];

                fm_pair_2_.SetRatio(3.0f);
                fm_pair_2_.SetIndex(mod_index_ * 0.7f);
                float sig2 = fm_pair_2_.Process() * env_[2].Process(env_gate_)
                             * op_levels_[2];

                out = (sig1 + sig2) * 0.5f;
                break;
            }

            case 3: // Harmonic (Organ-like)
            {
                fm_pair_1_.SetRatio(1.0f);
                fm_pair_1_.SetIndex(0.2f);
                float h1 = fm_pair_1_.Process() * env_[0].Process(env_gate_)
                           * op_levels_[0];

                fm_pair_2_.SetRatio(2.0f);
                fm_pair_2_.SetIndex(0.2f);
                float h2 = fm_pair_2_.Process() * env_[1].Process(env_gate_)
                           * op_levels_[1];

                out = (h1 * 0.6f + h2 * 0.4f);
                break;
            }

            case 4: // Bell
            {
                fm_pair_1_.SetRatio(1.4f);
                fm_pair_1_.SetIndex(mod_index_);
                float car = fm_pair_1_.Process() * env_[0].Process(env_gate_);

                fm_pair_2_.SetRatio(3.5f);
                fm_pair_2_.SetIndex(mod_index_ * 0.5f);
                float mod = fm_pair_2_.Process() * env_[2].Process(env_gate_);

                out = (car + mod * 0.3f) * op_levels_[0];
                break;
            }

            case 5: // Brass
            {
                fm_pair_1_.SetRatio(1.0f);
                fm_pair_1_.SetIndex(mod_index_ * 1.5f);
                out = fm_pair_1_.Process() * env_[0].Process(env_gate_)
                      * op_levels_[0];
                break;
            }

            case 6: // E-Piano
            {
                fm_pair_1_.SetRatio(1.0f);
                fm_pair_1_.SetIndex(mod_index_ * 2.0f);
                float car = fm_pair_1_.Process() * env_[0].Process(env_gate_);

                fm_pair_2_.SetRatio(14.0f);
                fm_pair_2_.SetIndex(0.5f);
                float high = fm_pair_2_.Process() * env_[2].Process(env_gate_);

                out = (car + high * 0.2f) * op_levels_[0];
                break;
            }

            case 7: // Bass
            {
                fm_pair_1_.SetRatio(1.0f);
                fm_pair_1_.SetIndex(mod_index_ * 3.0f);
                out = fm_pair_1_.Process() * env_[0].Process(env_gate_)
                      * op_levels_[0];
                break;
            }

            default:
                out = fm_pair_1_.Process() * env_[0].Process(env_gate_)
                      * op_levels_[0];
        }

        // Apply filter
        filt_.Process(out);
        out = filt_.Low();

        return out * (velocity_ / 127.0f);
    }
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

    void SetAlgorithm(int algo)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices_[i].SetAlgorithm(algo);
        }
    }

    void SetModIndex(float idx)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices_[i].SetModIndex(idx);
        }
    }

    void SetOperatorLevel(int op, float level)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices_[i].SetOperatorLevel(op, level);
        }
    }

    void SetCutoff(float freq)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices_[i].SetCutoff(freq);
        }
    }

    void SetResonance(float res)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices_[i].SetResonance(res);
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


// ========== OLED VISUALIZATION (sequencer_pod pattern) ==========
float    prevKnob[8], currKnob[8];
int      zoomParam     = -1;
uint32_t zoomStartTime = 0;
int      current_algorithm = 0;
int      octave_offset = 0;

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

    // Clear zoom after 1.2 seconds
    if(System::GetNow() - zoomStartTime > 1200)
        zoomParam = -1;
}

const char* GetAlgoName(int algo)
{
    switch(algo)
    {
        case 0: return "Stack";
        case 1: return "Parallel";
        case 2: return "2+2 Split";
        case 3: return "Harmonic";
        case 4: return "Bell";
        case 5: return "Brass";
        case 6: return "E-Piano";
        case 7: return "Bass";
        default: return "Custom";
    }
}

void DrawZoomedParameter(int param)
{
    char buf[32];
    int  percent = (int)(currKnob[param] * 100.0f);

    hw.display.SetCursor(0, 10);

    switch(param)
    {
        case 0: // OP1 Level
            sprintf(buf, "OP1 Level");
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            sprintf(buf, "%d%% (%.2f)", percent, currKnob[param]);
            hw.display.WriteString(buf, Font_11x18, true);
            break;

        case 1: // OP2 Level
            sprintf(buf, "OP2 Level");
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            sprintf(buf, "%d%% (%.2f)", percent, currKnob[param]);
            hw.display.WriteString(buf, Font_11x18, true);
            break;

        case 2: // OP3 Level
            sprintf(buf, "OP3 Level");
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            sprintf(buf, "%d%% (%.2f)", percent, currKnob[param]);
            hw.display.WriteString(buf, Font_11x18, true);
            break;

        case 3: // OP4 Level
            sprintf(buf, "OP4 Level");
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            sprintf(buf, "%d%% (%.2f)", percent, currKnob[param]);
            hw.display.WriteString(buf, Font_11x18, true);
            break;

        case 4: // Mod Index
        {
            float mod_val = currKnob[param] * 10.0f;
            sprintf(buf, "Mod Index");
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            sprintf(buf, "%d%% (%.1f)", percent, mod_val);
            hw.display.WriteString(buf, Font_11x18, true);
            break;
        }

        case 5: // Algorithm
        {
            int algo = (int)(currKnob[param] * 7.99f);
            sprintf(buf, "Algorithm");
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            sprintf(buf, "%d: %s", algo, GetAlgoName(algo));
            hw.display.WriteString(buf, Font_11x18, true);
            break;
        }

        case 6: // Filter Cutoff
        {
            float freq = 20.0f + currKnob[param] * 19980.0f;
            sprintf(buf, "Filter Cutoff");
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            if(freq < 1000.0f)
                sprintf(buf, "%d%% (%.0fHz)", percent, freq);
            else
                sprintf(buf, "%d%% (%.1fkHz)", percent, freq / 1000.0f);
            hw.display.WriteString(buf, Font_11x18, true);
            break;
        }

        case 7: // Resonance
            sprintf(buf, "Resonance");
            hw.display.WriteString(buf, Font_7x10, true);
            hw.display.SetCursor(0, 25);
            sprintf(buf, "%d%% (%.2f)", percent, currKnob[param]);
            hw.display.WriteString(buf, Font_11x18, true);
            break;
    }

    // Progress bar
    int barWidth = (int)(currKnob[param] * 127.0f);
    hw.display.DrawRect(0, 50, barWidth, 58, true, true);
}

void DrawDefaultView()
{
    char buf[32];

    // Title
    sprintf(buf, "FM Synth");
    hw.display.WriteString(buf, Font_7x10, true);

    // Algorithm
    hw.display.SetCursor(0, 12);
    sprintf(buf, "Algo %d: %s", current_algorithm, GetAlgoName(current_algorithm));
    hw.display.WriteString(buf, Font_7x10, true);

    // Active voices
    hw.display.SetCursor(0, 24);
    int active_count = voice_manager.CountActiveVoices();
    sprintf(buf, "Voices: %d/8", active_count);
    hw.display.WriteString(buf, Font_7x10, true);

    // Octave
    hw.display.SetCursor(0, 36);
    sprintf(buf, "Octave: %+d", octave_offset);
    hw.display.WriteString(buf, Font_7x10, true);

    // Operator levels (mini bars)
    hw.display.SetCursor(0, 48);
    sprintf(buf, "OP: %.0f %.0f %.0f %.0f",
            currKnob[0] * 10.0f,
            currKnob[1] * 10.0f,
            currKnob[2] * 10.0f,
            currKnob[3] * 10.0f);
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
    hw.ProcessDigitalControls();

    // Octave switches
    if(hw.sw[0].RisingEdge())
        octave_offset = fmax(-2, octave_offset - 1);
    if(hw.sw[1].RisingEdge())
        octave_offset = fmin(2, octave_offset + 1);

    // Update FM parameters
    voice_manager.SetOperatorLevel(0, hw.GetKnobValue(0));
    voice_manager.SetOperatorLevel(1, hw.GetKnobValue(1));
    voice_manager.SetOperatorLevel(2, hw.GetKnobValue(2));
    voice_manager.SetOperatorLevel(3, hw.GetKnobValue(3));
    voice_manager.SetModIndex(hw.GetKnobValue(4) * 10.0f);

    current_algorithm = (int)(hw.GetKnobValue(5) * 7.99f);
    voice_manager.SetAlgorithm(current_algorithm);

    voice_manager.SetCutoff(20.0f + hw.GetKnobValue(6) * 19980.0f);
    voice_manager.SetResonance(hw.GetKnobValue(7));

    // Keyboard handling
    for(int i = 0; i < 16; i++)
    {
        // Note layout: chromatic keyboard
        float base_note = 48.0f + (octave_offset * 12.0f); // C3 + octave offset
        float note      = base_note + i;

        if(hw.KeyboardRisingEdge(i))
        {
            voice_manager.OnNoteOn(note, 100);
        }

        if(hw.KeyboardFallingEdge(i))
        {
            voice_manager.OnNoteOff(note);
        }
    }

    // Process voices
    for(size_t i = 0; i < size; i++)
    {
        float sum      = voice_manager.Process() * 0.125f; // Scale for 8 voices
        out[0][i] = out[1][i] = sum;
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

    // Initialize display
    hw.display.Fill(false);
    const char str[] = "FM Synth Init...";
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
        CheckParameterChanges();
        UpdateOLED();
        System::Delay(16); // ~60 FPS
    }
}
