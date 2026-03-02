#include "daisy_field.h"
#include "daisysp.h"
#include <string>

using namespace daisy;
using namespace daisysp;

DaisyField hw;

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

// MIDI note display state (updated in main loop, not audio callback)
static int last_midi_note_ = -1;
static int last_midi_velocity_ = -1;
static bool new_midi_note_ = false;

// Volatile parameters for thread-safe communication between main loop and audio callback
volatile float knob1_value = 0.0f;
volatile bool sw1_pressed = false;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    // ONLY audio processing - read pre-updated parameters from main loop
    float cutoff = 250.f + knob1_value * 8000.f;
    voice_handler.SetCutoff(cutoff);

    for(size_t i = 0; i < size; i += 2)
    {
        float sum        = 0.f;
        sum        = voice_handler.Process() * 0.5f;
        out[i]     = sum;
        out[i + 1] = sum;
    }
}

// Typical Switch case for Message Type.
void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:
        {
            NoteOnEvent p = m.AsNoteOn();
            // Store for display in main loop (non-blocking)
            last_midi_note_ = p.note;
            last_midi_velocity_ = p.velocity;
            new_midi_note_ = true;
            
            // Note Off can come in as Note On w/ 0 Velocity
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

// Main -- Init, and Midi Handling
int main(void)
{
    // Init
    float samplerate;
    hw.Init();
    samplerate = hw.AudioSampleRate();
    voice_handler.Init(samplerate);

    //display
    const char str[] = "Midi";
    char *     cstr  = (char *)str;
    hw.display.WriteString(cstr, Font_7x10, true);
    hw.display.Update();

    // Start stuff.
    hw.midi.StartReceive();
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    for(;;)
    {
        // Control processing in main loop (NOT in audio callback)
        hw.ProcessDigitalControls();
        hw.ProcessAnalogControls();
        
        // Read knob values - update volatile variable for audio callback
        knob1_value = hw.GetKnobValue(hw.KNOB_1);
        
        // Handle switches
        if(hw.GetSwitch(hw.SW_1)->FallingEdge())
        {
            voice_handler.FreeAllVoices();
        }
        
        // Handle MIDI
        hw.midi.Listen();
        // Handle MIDI Events
        while(hw.midi.HasEvents())
        {
            HandleMidiMessage(hw.midi.PopEvent());
        }
        // Update display in main loop (non-blocking)
        if(new_midi_note_)
        {
            hw.display.Fill(false);
            hw.display.SetCursor(0, 0);
            char debugStr[32];
            sprintf(debugStr, "Note: %d Vel: %d", last_midi_note_, last_midi_velocity_);
            hw.display.WriteString(debugStr, Font_7x10, true);
            hw.display.Update();
            new_midi_note_ = false;
        }
    }
}
