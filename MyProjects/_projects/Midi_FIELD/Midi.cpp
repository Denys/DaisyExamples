

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

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    float sum = 0.f;
    hw.ProcessDigitalControls();
    hw.ProcessAnalogControls();
    if(hw.GetSwitch(hw.SW_1)->FallingEdge())
    {
        voice_handler.FreeAllVoices();
    }
    voice_handler.SetCutoff(250.f + hw.GetKnobValue(hw.KNOB_1) * 8000.f);

    for(size_t i = 0; i < size; i += 2)
    {
        sum        = 0.f;
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
        hw.midi.Listen();
        // Handle MIDI Events
        while(hw.midi.HasEvents())
        {
            HandleMidiMessage(hw.midi.PopEvent());
        }
    }
}


/*
#include "daisy_field.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

// Global variables
DaisyField hardware;
MidiUartHandler   midi;

// Sequencer variables
const int NUM_TRACKS = 4;
const int NUM_STEPS = 8;
bool step_active[NUM_TRACKS][NUM_STEPS];
uint8_t step_note[NUM_TRACKS][NUM_STEPS];
uint8_t step_velocity[NUM_TRACKS][NUM_STEPS];
float step_gate_length[NUM_TRACKS][NUM_STEPS];
uint8_t track_channel[NUM_TRACKS];
bool track_mute[NUM_TRACKS];

uint8_t current_step = 0;
float bpm = 120.0f;
float step_duration_ms = 0;
bool sequencer_running = false;
bool clock_tick = false;

// MIDI clock variables
bool sending_midi_clock = false;
uint32_t last_clock_time = 0;
float clock_interval_ms = 0;
uint8_t ppq_counter = 0;

// Scheduled notes
const int MAX_SCHEDULED_NOTES = 32;
struct ScheduledNote {
    uint8_t channel;
    uint8_t note;
    uint8_t velocity;
    float gate_length;
    uint32_t start_time;
    uint32_t duration;
    bool active;
};
ScheduledNote scheduled_notes[MAX_SCHEDULED_NOTES];

// Error handling
bool midi_error_occurred = false;
uint32_t last_error_time = 0;
const uint32_t ERROR_DISPLAY_DURATION = 3000;

// Function prototypes
void InitMidi();
void InitSequencer();
void InitScheduledNotes();
void StartSequencer();
void StopSequencer();
void ProcessSequencerStep();
void SendMidiNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
void SendMidiNoteOff(uint8_t channel, uint8_t note, uint8_t velocity = 0);
void SendMidiCc(uint8_t channel, uint8_t control_number, uint8_t value);
void ScheduleNote(uint8_t channel, uint8_t note, uint8_t velocity, float gate_length, float step_duration_ms);
void ProcessScheduledNotes();
void StartMidiClock(float bpm);
void StopMidiClock();
void ProcessMidiClock();
void CheckMidiErrors();
void DisplayMidiError(OledDisplay<SSD130x4WireSpi128x64Driver>& display);

// Audio callback
void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    // Audio processing code would go here
    // For a MIDI-only application, this could be minimal
    
    for(size_t i = 0; i < size; i++)
    {
        // Clear output
        out[0][i] = out[1][i] = 0.0f;
    }
}

int main(void)
{
    // Initialize hardware
    hardware.Init();
    hardware.StartAdc();
    
    // Initialize MIDI
    InitMidi();
    
    // Initialize sequencer
    InitSequencer();
    
    // Initialize scheduled notes
    InitScheduledNotes();
    
    // Display already initialized in hardware.Init()
    
    // Start audio
    hardware.StartAudio(AudioCallback);
    
    // Main loop
    while(1)
    {
        // Process controls
        hardware.ProcessAllControls();
        
        // Process MIDI input
        midi.Listen();
        
        // Process any incoming MIDI messages
        while(midi.HasEvents())
        {
            auto event = midi.PopEvent();
            // Process event...
        }
        
        // Check for button presses to start/stop sequencer
        if(hardware.GetSwitch(DaisyField::SW_1)->RisingEdge())
        {
            if(sequencer_running)
                StopSequencer();
            else
                StartSequencer();
        }
        
        // Update BPM based on knob
        float new_bpm = 60.0f + hardware.GetKnobValue(DaisyField::KNOB_1) * 180.0f;
        if(abs(new_bpm - bpm) > 0.5f)
        {
            bpm = new_bpm;
            step_duration_ms = (60000.0f / bpm) / 4.0f;
            
            if(sending_midi_clock)
            {
                clock_interval_ms = (60000.0f / bpm) / 24.0f;
            }
        }
        
        // Process MIDI clock
        ProcessMidiClock();
        
        // Check if it's time for a new step (based on MIDI clock)
        if(sending_midi_clock && ppq_counter % 6 == 0)
        {
            clock_tick = true;
        }
        else
        {
            clock_tick = false;
        }
        
        // Process sequencer step if clock ticked
        if(clock_tick)
        {
            ProcessSequencerStep();
        }
        
        // Process scheduled notes
        ProcessScheduledNotes();
        
        // Check for MIDI errors
        CheckMidiErrors();
        
        // Update display
        hardware.display.Fill(false);
        
        // Display sequencer status
        hardware.display.SetCursor(0, 0);
        hardware.display.WriteString(sequencer_running ? "Running" : "Stopped", Font_7x10, true);
        
        // Display BPM
        hardware.display.SetCursor(0, 12);
        char bpm_str[16];
        sprintf(bpm_str, "BPM: %.1f", bpm);
        hardware.display.WriteString(bpm_str, Font_7x10, true);
        
        // Display current step
        hardware.display.SetCursor(0, 24);
        char step_str[16];
        sprintf(step_str, "Step: %d", current_step + 1);
        hardware.display.WriteString(step_str, Font_7x10, true);
        
        // Display MIDI error if needed
        if(midi_error_occurred)
        {
            hardware.display.SetCursor(0, 36);
            hardware.display.WriteString("MIDI Error", Font_7x10, true);
        }
        
        hardware.display.Update();
        
        // Update LEDs
        for(int i = 0; i < NUM_STEPS; i++)
        {
            bool is_current = (i == current_step && sequencer_running);
            bool is_active = step_active[0][i]; // Show track 1 steps
            
            float brightness = 0.0f;
            if(is_current && is_active)
                brightness = 1.0f;
            else if(is_current)
                brightness = 0.5f;
            else if(is_active)
                brightness = 0.2f;
            
            hardware.led_driver.SetLed(DaisyField::LED_KEY_B1 + i, brightness);
        }
        hardware.led_driver.SwapBuffersAndTransmit();
        
        // Small delay to prevent CPU hogging
        System::Delay(1);
    }
}

// Initialize MIDI
void InitMidi()
{
    MidiUartHandler::Config midi_config;
    midi.Init(midi_config);
    midi.StartReceive();
}

// Initialize sequencer
void InitSequencer()
{
    for(int t = 0; t < NUM_TRACKS; t++)
    {
        track_channel[t] = t;
        track_mute[t] = false;
        
        for(int s = 0; s < NUM_STEPS; s++)
        {
            step_active[t][s] = false;
            step_note[t][s] = 60 + t;
            step_velocity[t][s] = 100;
            step_gate_length[t][s] = 0.5f;
        }
    }
    
    step_duration_ms = (60000.0f / bpm) / 4.0f;
}

// Initialize scheduled notes
void InitScheduledNotes()
{
    for(int i = 0; i < MAX_SCHEDULED_NOTES; i++)
    {
        scheduled_notes[i].active = false;
    }
}

// Start sequencer
void StartSequencer()
{
    sequencer_running = true;
    current_step = 0;
    StartMidiClock(bpm);
}

// Stop sequencer
void StopSequencer()
{
    sequencer_running = false;
    StopMidiClock();
    
    for(int c = 0; c < 16; c++)
    {
        SendMidiCc(c, 123, 0);
    }
}

// Process sequencer step
void ProcessSequencerStep()
{
    for(int t = 0; t < NUM_TRACKS; t++)
    {
        if(!track_mute[t] && step_active[t][current_step])
        {
            ScheduleNote(
                track_channel[t],
                step_note[t][current_step],
                step_velocity[t][current_step],
                step_gate_length[t][current_step],
                step_duration_ms
            );
        }
    }
    
    current_step = (current_step + 1) % NUM_STEPS;
}

// Send MIDI Note On
void SendMidiNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
    uint8_t msg[3] = {static_cast<uint8_t>(0x90 | (channel & 0x0F)), static_cast<uint8_t>(note & 0x7F), static_cast<uint8_t>(velocity & 0x7F)};
    midi.SendMessage(msg, 3);
}

// Send MIDI Note Off
void SendMidiNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
{
    uint8_t msg[3] = {static_cast<uint8_t>(0x80 | (channel & 0x0F)), static_cast<uint8_t>(note & 0x7F), static_cast<uint8_t>(velocity & 0x7F)};
    midi.SendMessage(msg, 3);
}

// Send MIDI CC
void SendMidiCc(uint8_t channel, uint8_t control_number, uint8_t value)
{
    uint8_t msg[3] = {static_cast<uint8_t>(0xB0 | (channel & 0x0F)), static_cast<uint8_t>(control_number & 0x7F), static_cast<uint8_t>(value & 0x7F)};
    midi.SendMessage(msg, 3);
}

// Schedule a note
void ScheduleNote(uint8_t channel, uint8_t note, uint8_t velocity, float gate_length, float step_duration_ms)
{
    for(int i = 0; i < MAX_SCHEDULED_NOTES; i++)
    {
        if(!scheduled_notes[i].active)
        {
            scheduled_notes[i].channel = channel;
            scheduled_notes[i].note = note;
            scheduled_notes[i].velocity = velocity;
            scheduled_notes[i].gate_length = gate_length;
            scheduled_notes[i].start_time = System::GetNow();
            scheduled_notes[i].duration = step_duration_ms * gate_length;
            scheduled_notes[i].active = true;
            
            SendMidiNoteOn(channel, note, velocity);
            return;
        }
    }
}

// Process scheduled notes
void ProcessScheduledNotes()
{
    uint32_t current_time = System::GetNow();
    
    for(int i = 0; i < MAX_SCHEDULED_NOTES; i++)
    {
        if(scheduled_notes[i].active)
        {
            if(current_time - scheduled_notes[i].start_time >= scheduled_notes[i].duration)
            {
                SendMidiNoteOff(scheduled_notes[i].channel, scheduled_notes[i].note);
                scheduled_notes[i].active = false;
            }
        }
    }
}

// Start MIDI clock
void StartMidiClock(float bpm)
{
    clock_interval_ms = (60000.0f / bpm) / 24.0f;
    sending_midi_clock = true;
    ppq_counter = 0;
    last_clock_time = System::GetNow();

    uint8_t msg[1] = {0xFA}; // Start
    midi.SendMessage(msg, 1);
}

// Stop MIDI clock
void StopMidiClock()
{
    sending_midi_clock = false;
    uint8_t msg[1] = {0xFC}; // Stop
    midi.SendMessage(msg, 1);
}

// Process MIDI clock
void ProcessMidiClock()
{
    if(!sending_midi_clock)
        return;

    uint32_t current_time = System::GetNow();

    if(current_time - last_clock_time >= clock_interval_ms)
    {
        uint8_t msg[1] = {0xF8}; // Clock
        midi.SendMessage(msg, 1);
        last_clock_time = current_time;
        ppq_counter = (ppq_counter + 1) % 24;
    }
}

// Check for MIDI errors (simplified - no error handling available in current API)
void CheckMidiErrors()
{
    // MIDI error handling not available in current libDaisy API
    // This function is kept for compatibility but does nothing
}

// Display MIDI error
void DisplayMidiError(OledDisplay<SSD130x4WireSpi128x64Driver>& display)
{
    if(midi_error_occurred)
    {
        display.SetCursor(0, 48);
        display.WriteString("MIDI Error", Font_7x10, true);
    }
}
*/

/*
#include "daisy_pod.h"
#include "daisysp.h"
#include <stdio.h>
#include <string.h>

using namespace daisy;
using namespace daisysp;

DaisyPod   hw;
Oscillator osc;
Svf        filt;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    float sig;
    for(size_t i = 0; i < size; i += 2)
    {
        sig = osc.Process();
        filt.Process(sig);
        out[i] = out[i + 1] = filt.Low();
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
            char        buff[512];
            sprintf(buff,
                    "Note Received:\t%d\t%d\t%d\r\n",
                    m.channel,
                    m.data[0],
                    m.data[1]);
            hw.seed.usb_handle.TransmitInternal((uint8_t *)buff, strlen(buff));
            // This is to avoid Max/MSP Note outs for now..
            if(m.data[1] != 0)
            {
                p = m.AsNoteOn();
                osc.SetFreq(mtof(p.note));
                osc.SetAmp((p.velocity / 127.0f));
            }
        }
        break;
        case ControlChange:
        {
            ControlChangeEvent p = m.AsControlChange();
            switch(p.control_number)
            {
                case 1:
                    // CC 1 for cutoff.
                    filt.SetFreq(mtof((float)p.value));
                    break;
                case 2:
                    // CC 2 for res.
                    filt.SetRes(((float)p.value / 127.0f));
                    break;
                default: break;
            }
            break;
        }
        default: break;
    }
}


// Main -- Init, and Midi Handling
int main(void)
{
    // Init
    float samplerate;
    hw.Init();
    hw.SetAudioBlockSize(4);
    hw.seed.usb_handle.Init(UsbHandle::FS_INTERNAL);
    System::Delay(250);

    // Synthesis
    samplerate = hw.AudioSampleRate();
    osc.Init(samplerate);
    osc.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);
    filt.Init(samplerate);

    // Start stuff.
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    hw.midi.StartReceive();
    for(;;)
    {
        hw.midi.Listen();
        // Handle MIDI Events
        while(hw.midi.HasEvents())
        {
            HandleMidiMessage(hw.midi.PopEvent());
        }
    }
}
*/