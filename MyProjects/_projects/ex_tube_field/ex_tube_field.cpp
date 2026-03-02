

#include "daisy_field.h"
#include "daisysp.h"
#include <string>
#include <stdio.h>

using namespace daisy;
using namespace daisysp;

DaisyField hw;

// Global Tube Effect
Tube tube;

// Global Knob State for Visualization
float prevKnob[8];
float currKnob[8];
int zoomParam = -1;
uint32_t zoomStartTime = 0;

class Voice
{
  public:
    Voice() {}
    ~Voice() {}
    void Init(float samplerate)
    {
        active_ = false;
        osc_.Init(samplerate);
        osc_.SetAmp(0.75f);
        osc_.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
        env_.Init(samplerate);
        env_.SetSustainLevel(0.5f);
        env_.SetTime(ADSR_SEG_ATTACK, 0.005f);
        env_.SetTime(ADSR_SEG_DECAY, 0.005f);
        env_.SetTime(ADSR_SEG_RELEASE, 0.2f);
        filt_.Init(samplerate);
        filt_.SetFreq(6000.f);
        filt_.SetRes(0.6f);
        filt_.SetDrive(0.8f);
    }

    float Process()
    {
        if(active_)
        {
            float sig, amp;
            amp = env_.Process(env_gate_);
            if(!env_.IsRunning())
                active_ = false;
            sig = osc_.Process();
            filt_.Process(sig);
            return filt_.Low() * (velocity_ / 127.f) * amp;
        }
        return 0.f;
    }

    void OnNoteOn(float note, float velocity)
    {
        note_     = note;
        velocity_ = velocity;
        osc_.SetFreq(mtof(note_));
        active_   = true;
        env_gate_ = true;
    }

    void OnNoteOff() { env_gate_ = false; }

    void SetCutoff(float val) { filt_.SetFreq(val); }

    inline bool  IsActive() const { return active_; }
    inline float GetNote() const { return note_; }

  private:
    Oscillator osc_;
    Svf        filt_;
    Adsr       env_;
    float      note_, velocity_;
    bool       active_;
    bool       env_gate_;
};

template <size_t max_voices>
class VoiceManager
{
  public:
    VoiceManager() {}
    ~VoiceManager() {}

    void Init(float samplerate)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices[i].Init(samplerate);
        }
    }

    float Process()
    {
        float sum;
        sum = 0.f;
        for(size_t i = 0; i < max_voices; i++)
        {
            sum += voices[i].Process();
        }
        return sum;
    }

    void OnNoteOn(float notenumber, float velocity)
    {
        Voice *v = FindFreeVoice();
        if(v == NULL)
            return;
        v->OnNoteOn(notenumber, velocity);
    }

    void OnNoteOff(float notenumber, float velocity)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            Voice *v = &voices[i];
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
            voices[i].OnNoteOff();
        }
    }

    void SetCutoff(float all_val)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices[i].SetCutoff(all_val);
        }
    }


  private:
    Voice  voices[max_voices];
    Voice *FindFreeVoice()
    {
        Voice *v = NULL;
        for(size_t i = 0; i < max_voices; i++)
        {
            if(!voices[i].IsActive())
            {
                v = &voices[i];
                break;
            }
        }
        return v;
    }
};

static VoiceManager<24> voice_handler;

// OLED Logic
void CheckParameterChanges()
{
    for(int i = 0; i < 8; i++)
    {
        if(fabsf(currKnob[i] - prevKnob[i]) > 0.02f)
        {
            zoomParam     = i;
            zoomStartTime = System::GetNow();
            prevKnob[i]   = currKnob[i];
        }
    }
    // Timeout
    if(System::GetNow() - zoomStartTime > 1200)
    {
        zoomParam = -1;
    }
}

void DrawZoomedParameter()
{
    hw.display.Fill(false);
    
    float val = currKnob[zoomParam];
    int percent = (int)(val * 100.f);
    char nameBuf[32];
    char valBuf[32];

    switch(zoomParam)
    {
        case 0: // Knob 1: Cutoff
            sprintf(nameBuf, "Filter Cutoff");
            sprintf(valBuf, "%d%% (%.0f Hz)", percent, 250.f + val * 8000.f); 
            break;
        case 4: // Knob 5: Gain
            sprintf(nameBuf, "Tube Drive");
            sprintf(valBuf, "%d%% (%.1f)", percent, 0.1f + (val * val * 49.9f));
            break;
        case 5: // Knob 6: Work Point
            {
                sprintf(nameBuf, "Work Point (Bias)");
                float q = 2.0f * val - 1.0f;
                const char* type = "Mix";
                if (q < -0.3f) type = "Triode";
                else if (q > 0.3f) type = "Asym";
                else type = "Pentode";
                sprintf(valBuf, "%d%% (%.2f %s)", percent, q, type);
            }
            break;
        case 6: // Knob 7: Distortion
            sprintf(nameBuf, "Hardness");
            sprintf(valBuf, "%d%% (%.1f)", percent, 0.1f + (val * val * 49.9f));
            break;
        case 7: // Knob 8: Mix
            sprintf(nameBuf, "Dry/Wet Mix");
            sprintf(valBuf, "%d%%", percent);
            break;
        default:
            sprintf(nameBuf, "Knob %d", zoomParam + 1);
            sprintf(valBuf, "%d%% (%.2f)", percent, val);
            break;
    }
    
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(nameBuf, Font_11x18, true);
    
    hw.display.SetCursor(0, 24);
    hw.display.WriteString(valBuf, Font_7x10, true);
    
    // Progress bar
    hw.display.DrawRect(0, 50, 127, 58, true, false);
    hw.display.DrawRect(0, 50, (int)(val * 127.f), 58, true, true);
    
    hw.display.Update();
}

void DrawNormalDisplay()
{
    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString("Tube Synth", Font_11x18, true);
    hw.display.SetCursor(0, 24);
    hw.display.WriteString("Turn knobs to edit", Font_6x8, true);
    hw.display.SetCursor(0, 40);
    hw.display.WriteString("K5:Gain K6:Bias", Font_6x8, true);
    hw.display.SetCursor(0, 50);
    hw.display.WriteString("K7:Dist K8:Mix", Font_6x8, true);
    hw.display.Update();
}

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    float sum = 0.f;
    hw.ProcessDigitalControls();
    hw.ProcessAnalogControls();
    
    // Update Knob Values
    for(int i = 0; i < 8; i++)
    {
        currKnob[i] = hw.GetKnobValue(i);
    }
    
    // Switch 1: Choke voices
    if(hw.GetSwitch(hw.SW_1)->FallingEdge())
    {
        voice_handler.FreeAllVoices();
    }
    
    // Knob 1: Filter Cutoff
    voice_handler.SetCutoff(250.f + currKnob[0] * 8000.f);

    // Knob 5: Tube Gain (0.1 - 50.0)
    // Exponential curve for drive feels better
    float gain_val = currKnob[4];
    tube.SetGain(0.1f + (gain_val * gain_val * 49.9f));

    // Knob 6: Work Point Q (-1.0 to 1.0)
    tube.SetWorkPoint(2.0f * currKnob[5] - 1.0f);

    // Knob 7: Distortion (0.1 - 50.0)
    float dist_val = currKnob[6];
    tube.SetDistortion(0.1f + (dist_val * dist_val * 49.9f));
    
    // Knob 8: Dry/Wet Mix (0.0 - 1.0)
    tube.SetMix(currKnob[7]);

    for(size_t i = 0; i < size; i += 2)
    {
        sum        = 0.f;
        // Process voices
        float voice_out = voice_handler.Process() * 0.5f;
        
        // Process Tube
        sum = tube.Process(voice_out);
        
        out[i]     = sum;
        out[i + 1] = sum;
    }
}

void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:
        {
            NoteOnEvent p = m.AsNoteOn();
            if(p.velocity == 0.f)
            {
                voice_handler.OnNoteOff(p.note, p.velocity);
            }
            else
            {
                voice_handler.OnNoteOn(p.note, p.velocity);
            }
        }
        break;
        case NoteOff:
        {
            NoteOnEvent p = m.AsNoteOn();
            voice_handler.OnNoteOff(p.note, p.velocity);
        }
        break;
        default: break;
    }
}

int main(void)
{
    float samplerate;
    hw.Init();
    samplerate = hw.AudioSampleRate();
    voice_handler.Init(samplerate);
    
    // Init Tube
    tube.Init(samplerate);

    // Start stuff
    hw.midi.StartReceive();
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    
    for(;;)
    {
        hw.midi.Listen();
        while(hw.midi.HasEvents())
        {
            HandleMidiMessage(hw.midi.PopEvent());
        }
        
        CheckParameterChanges();
        
        if(zoomParam >= 0)
        {
            DrawZoomedParameter();
        }
        else
        {
            DrawNormalDisplay();
        }
        
        // Update LEDs for tube knobs
        hw.led_driver.SetLed(DaisyField::LED_KNOB_5, currKnob[4]);
        hw.led_driver.SetLed(DaisyField::LED_KNOB_6, currKnob[5]); 
        hw.led_driver.SetLed(DaisyField::LED_KNOB_7, currKnob[6]); 
        hw.led_driver.SetLed(DaisyField::LED_KNOB_8, currKnob[7]); 
        hw.led_driver.SwapBuffersAndTransmit();

        System::Delay(16); // ~60fps
    }
}


/*
#include "daisy_field.h"
#include "daisysp.h"
#include <string>
#include <stdio.h>

using namespace daisy;
using namespace daisysp;

DaisyField hw;

// Global Tube Effect
Tube tube;

// Global Knob State for Visualization
float prevKnob[8];
float currKnob[8];
int zoomParam = -1;
uint32_t zoomStartTime = 0;

class Voice
{
  public:
    Voice() {}
    ~Voice() {}
    void Init(float samplerate)
    {
        active_ = false;
        osc_.Init(samplerate);
        osc_.SetAmp(0.75f);
        osc_.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
        env_.Init(samplerate);
        env_.SetSustainLevel(0.5f);
        env_.SetTime(ADSR_SEG_ATTACK, 0.005f);
        env_.SetTime(ADSR_SEG_DECAY, 0.005f);
        env_.SetTime(ADSR_SEG_RELEASE, 0.2f);
        filt_.Init(samplerate);
        filt_.SetFreq(6000.f);
        filt_.SetRes(0.6f);
        filt_.SetDrive(0.8f);
    }

    float Process()
    {
        if(active_)
        {
            float sig, amp;
            amp = env_.Process(env_gate_);
            if(!env_.IsRunning())
                active_ = false;
            sig = osc_.Process();
            filt_.Process(sig);
            return filt_.Low() * (velocity_ / 127.f) * amp;
        }
        return 0.f;
    }

    void OnNoteOn(float note, float velocity)
    {
        note_     = note;
        velocity_ = velocity;
        osc_.SetFreq(mtof(note_));
        active_   = true;
        env_gate_ = true;
    }

    void OnNoteOff() { env_gate_ = false; }

    void SetCutoff(float val) { filt_.SetFreq(val); }

    inline bool  IsActive() const { return active_; }
    inline float GetNote() const { return note_; }

  private:
    Oscillator osc_;
    Svf        filt_;
    Adsr       env_;
    float      note_, velocity_;
    bool       active_;
    bool       env_gate_;
};

template <size_t max_voices>
class VoiceManager
{
  public:
    VoiceManager() {}
    ~VoiceManager() {}

    void Init(float samplerate)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices[i].Init(samplerate);
        }
    }

    float Process()
    {
        float sum;
        sum = 0.f;
        for(size_t i = 0; i < max_voices; i++)
        {
            sum += voices[i].Process();
        }
        return sum;
    }

    void OnNoteOn(float notenumber, float velocity)
    {
        Voice *v = FindFreeVoice();
        if(v == NULL)
            return;
        v->OnNoteOn(notenumber, velocity);
    }

    void OnNoteOff(float notenumber, float velocity)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            Voice *v = &voices[i];
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
            voices[i].OnNoteOff();
        }
    }

    void SetCutoff(float all_val)
    {
        for(size_t i = 0; i < max_voices; i++)
        {
            voices[i].SetCutoff(all_val);
        }
    }


  private:
    Voice  voices[max_voices];
    Voice *FindFreeVoice()
    {
        Voice *v = NULL;
        for(size_t i = 0; i < max_voices; i++)
        {
            if(!voices[i].IsActive())
            {
                v = &voices[i];
                break;
            }
        }
        return v;
    }
};

static VoiceManager<24> voice_handler;

// OLED Logic
void CheckParameterChanges()
{
    for(int i = 0; i < 8; i++)
    {
        if(fabsf(currKnob[i] - prevKnob[i]) > 0.02f)
        {
            zoomParam     = i;
            zoomStartTime = System::GetNow();
            prevKnob[i]   = currKnob[i];
        }
    }
    // Timeout
    if(System::GetNow() - zoomStartTime > 1200)
    {
        zoomParam = -1;
    }
}

void DrawZoomedParameter()
{
    hw.display.Fill(false);
    
    float val = currKnob[zoomParam];
    int percent = (int)(val * 100.f);
    char nameBuf[32];
    char valBuf[32];

    switch(zoomParam)
    {
        case 0: // Knob 1: Cutoff
            sprintf(nameBuf, "Filter Cutoff");
            sprintf(valBuf, "%d%% (%.0f Hz)", percent, 250.f + val * 8000.f); 
            break;
        case 4: // Knob 5: Gain
            sprintf(nameBuf, "Tube Drive");
            sprintf(valBuf, "%d%% (%.1f)", percent, 0.1f + (val * val * 49.9f));
            break;
        case 5: // Knob 6: Work Point
            {
                sprintf(nameBuf, "Work Point (Bias)");
                float q = 2.0f * val - 1.0f;
                const char* type = "Mix";
                if (q < -0.3f) type = "Triode";
                else if (q > 0.3f) type = "Asym";
                else type = "Pentode";
                sprintf(valBuf, "%d%% (%.2f %s)", percent, q, type);
            }
            break;
        case 6: // Knob 7: Distortion
            sprintf(nameBuf, "Hardness");
            sprintf(valBuf, "%d%% (%.1f)", percent, 0.1f + (val * val * 49.9f));
            break;
        case 7: // Knob 8: Mix
            sprintf(nameBuf, "Dry/Wet Mix");
            sprintf(valBuf, "%d%%", percent);
            break;
        default:
            sprintf(nameBuf, "Knob %d", zoomParam + 1);
            sprintf(valBuf, "%d%% (%.2f)", percent, val);
            break;
    }
    
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(nameBuf, Font_11x18, true);
    
    hw.display.SetCursor(0, 24);
    hw.display.WriteString(valBuf, Font_7x10, true);
    
    // Progress bar
    hw.display.DrawRect(0, 50, 127, 58, true, false);
    hw.display.DrawRect(0, 50, (int)(val * 127.f), 58, true, true);
    
    hw.display.Update();
}

void DrawNormalDisplay()
{
    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString("Tube Synth", Font_11x18, true);
    hw.display.SetCursor(0, 24);
    hw.display.WriteString("Turn knobs to edit", Font_6x8, true);
    hw.display.SetCursor(0, 40);
    hw.display.WriteString("K5:Gain K6:Bias", Font_6x8, true);
    hw.display.SetCursor(0, 50);
    hw.display.WriteString("K7:Dist K8:Mix", Font_6x8, true);
    hw.display.Update();
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    float sum = 0.f;
    hw.ProcessDigitalControls();
    hw.ProcessAnalogControls();
    
    // Update Knob Values
    for(int i = 0; i < 8; i++)
    {
        currKnob[i] = hw.GetKnobValue(i);
    }
    
    // Switch 1: Choke voices
    if(hw.GetSwitch(hw.SW_1)->FallingEdge())
    {
        voice_handler.FreeAllVoices();
    }
    
    // Knob 1: Filter Cutoff
    voice_handler.SetCutoff(250.f + currKnob[0] * 8000.f);

    // Knob 5: Tube Gain (0.1 - 50.0)
    // Exponential curve for drive feels better
    float gain_val = currKnob[4];
    tube.SetGain(0.1f + (gain_val * gain_val * 49.9f));

    // Knob 6: Work Point Q (-1.0 to 1.0)
    tube.SetWorkPoint(2.0f * currKnob[5] - 1.0f);

    // Knob 7: Distortion (0.1 - 50.0)
    float dist_val = currKnob[6];
    tube.SetDistortion(0.1f + (dist_val * dist_val * 49.9f));
    
    // Knob 8: Dry/Wet Mix (0.0 - 1.0)
    tube.SetMix(currKnob[7]);

    for(size_t i = 0; i < size; i++)
    {
        sum = 0.f;
        // Process voices
        float voice_out = voice_handler.Process() * 0.5f;
        
        // Process Tube
        sum = tube.Process(voice_out);
        
        out[0][i] = sum;  // Left
        out[1][i] = sum;  // Right
    }
}

void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:
        {
            NoteOnEvent p = m.AsNoteOn();
            if(p.velocity == 0.f)
            {
                voice_handler.OnNoteOff(p.note, p.velocity);
            }
            else
            {
                voice_handler.OnNoteOn(p.note, p.velocity);
            }
        }
        break;
        case NoteOff:
        {
            NoteOnEvent p = m.AsNoteOn();
            voice_handler.OnNoteOff(p.note, p.velocity);
        }
        break;
        default: break;
    }
}

int main(void)
{
    float samplerate;
    hw.Init();
    samplerate = hw.AudioSampleRate();
    voice_handler.Init(samplerate);
    
    // Init Tube
    tube.Init(samplerate);

    // Start stuff
    hw.midi.StartReceive();
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    
    for(;;)
    {
        hw.midi.Listen();
        while(hw.midi.HasEvents())
        {
            HandleMidiMessage(hw.midi.PopEvent());
        }
        
        CheckParameterChanges();
        
        if(zoomParam >= 0)
        {
            DrawZoomedParameter();
        }
        else
        {
            DrawNormalDisplay();
        }
        
        // Update LEDs for tube knobs
        hw.led_driver.SetLed(DaisyField::LED_KNOB_5, currKnob[4]);
        hw.led_driver.SetLed(DaisyField::LED_KNOB_6, currKnob[5]); 
        hw.led_driver.SetLed(DaisyField::LED_KNOB_7, currKnob[6]); 
        hw.led_driver.SetLed(DaisyField::LED_KNOB_8, currKnob[7]); 
        hw.led_driver.SwapBuffersAndTransmit();

        System::Delay(16); // ~60fps
    }
}
*/