

/*#include "daisy_field.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

// Declare a DaisyField object
DaisyField hw;

// 3 oscillators
Oscillator vco1, vco2, vco3;
// 2 state-variable filters (one for low-pass, one for high-pass)
Svf lp_filt, hp_filt;
// ADSR envelope
Adsr env;
// Overdrive
Overdrive drive;

// Global parameters
float vco1_level, vco2_level, vco3_level;
float vco1_detune, vco2_detune, vco3_detune;
int   vco1_waveform, vco2_waveform, vco3_waveform;
float lp_cutoff, lp_res;
float hp_cutoff, hp_res;
float attack, decay, sustain, release;
float overdrive_amount;
float master_volume;

// Keyboard state
bool key_states[16];
int  last_key = -1;
bool gate = false;

// UI state
int control_mode = 0; // 0: Default, 1: Mixer/VCOs, 2: ADSR

// Note frequencies for the 16 keys (chromatic scale starting from C4)
float note_freqs[16] = {
    261.63f, 277.18f, 293.66f, 311.13f, 329.63f, 349.23f, 369.99f, 392.00f,
    415.30f, 440.00f, 466.16f, 493.88f, 523.25f, 554.37f, 587.33f, 622.25f};

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        // Get envelope value
        float env_val = env.Process(gate);

        // Process oscillators
        vco1.SetFreq(mtof(48.0f + vco1_detune));
        vco2.SetFreq(mtof(48.0f + vco2_detune));
        vco3.SetFreq(mtof(48.0f + vco3_detune));

        float vco1_out = vco1.Process() * vco1_level;
        float vco2_out = vco2.Process() * vco2_level;
        float vco3_out = vco3.Process() * vco3_level;

        float mixed_signal = (vco1_out + vco2_out + vco3_out) / 3.0f;

        // Apply envelope
        float enveloped_signal = mixed_signal * env_val;

        // Apply filters
        lp_filt.Process(enveloped_signal);
        float lp_out = lp_filt.Low();
        hp_filt.Process(lp_out);
        float hp_out = hp_filt.High();

        // Apply overdrive
        float final_out = drive.Process(hp_out) * master_volume;

        // Output to both channels
        out[0][i] = final_out;
        out[1][i] = final_out;
    }
}

void UpdateControls()
{
    hw.ProcessAllControls();

    // Switches determine the control mode
    if(hw.sw[0].RisingEdge())
    {
        control_mode = (control_mode == 1) ? 0 : 1;
    }
    if(hw.sw[1].RisingEdge())
    {
        control_mode = (control_mode == 2) ? 0 : 2;
    }


    switch(control_mode)
    {
        case 0: // Default mode
            lp_cutoff         = hw.knob[0].Process();
            lp_res            = hw.knob[1].Process();
            hp_cutoff         = hw.knob[2].Process();
            hp_res            = hw.knob[3].Process();
            overdrive_amount  = hw.knob[4].Process();
            master_volume     = hw.knob[5].Process();
            vco1_detune       = hw.knob[6].Process() * 12.0f; // up to one octave detune
            vco2_detune       = hw.knob[6].Process() * 12.0f;
            vco3_detune       = hw.knob[7].Process() * 12.0f;
            break;
        case 1: // Mixer/VCOs mode
            vco1_level        = hw.knob[0].Process();
            vco2_level        = hw.knob[1].Process();
            vco3_level        = hw.knob[2].Process();
            vco1_waveform     = hw.knob[3].Process() * 3;
            vco2_waveform     = hw.knob[4].Process() * 3;
            vco3_waveform     = hw.knob[5].Process() * 3;
            break;
        case 2: // ADSR mode
            attack            = hw.knob[0].Process();
            decay             = hw.knob[1].Process();
            sustain           = hw.knob[2].Process();
            release           = hw.knob[3].Process();
            break;
    }

    // Update DSP parameters
    lp_filt.SetFreq(fmap(lp_cutoff, 20.0f, 20000.0f, Mapping::LOG));
    lp_filt.SetRes(fmap(lp_res, 0.0f, 0.95f));
    hp_filt.SetFreq(fmap(hp_cutoff, 20.0f, 20000.0f, Mapping::LOG));
    hp_filt.SetRes(fmap(hp_res, 0.0f, 0.95f));
    drive.SetDrive(fmap(overdrive_amount, 0.1f, 1.0f));

    vco1.SetWaveform(vco1_waveform);
    vco2.SetWaveform(vco2_waveform);
    vco3.SetWaveform(vco3_waveform);

    env.SetAttackTime(fmap(attack, 0.01f, 4.0f));
    env.SetDecayTime(fmap(decay, 0.01f, 4.0f));
    env.SetSustainLevel(sustain);
    env.SetReleaseTime(fmap(release, 0.01f, 4.0f));
}

void UpdateKeyboard()
{
    for(int i = 0; i < 16; i++)
    {
        if(hw.KeyboardState(i))
        {
            if(!key_states[i]) // Rising edge
            {
                key_states[i] = true;
                last_key      = i;
            }
        }
        else
        {
            if(key_states[i]) // Falling edge
            {
                key_states[i] = false;
                if(last_key == i)
                {
                    last_key = -1;
                    // Find the next held key
                    for(int j = 15; j >= 0; j--)
                    {
                        if(key_states[j])
                        {
                            last_key = j;
                            break;
                        }
                    }
                }
            }
        }
    }

    if(last_key != -1)
    {
        float key_freq = note_freqs[last_key];
        vco1.SetFreq(key_freq + vco1_detune);
        vco2.SetFreq(key_freq + vco2_detune);
        vco3.SetFreq(key_freq + vco3_detune);
        gate = true;
    }
    else
    {
        gate = false;
    }

    // Update LEDs
    for(int i = 0; i < 16; i++)
    {
        hw.led_driver.SetLed(i, static_cast<float>(key_states[i]));
    }
}

int main(void)
{
    // Initialize the Daisy Field
    hw.Init();
    hw.SetAudioBlockSize(4);
    float sample_rate = hw.AudioSampleRate();

    // Initialize the DSP components
    vco1.Init(sample_rate);
    vco2.Init(sample_rate);
    vco3.Init(sample_rate);
    lp_filt.Init(sample_rate);
    hp_filt.Init(sample_rate);
    env.Init(sample_rate);
    drive.Init();

    // Set initial parameter values
    vco1_level = 1.0f;
    vco2_level = 1.0f;
    vco3_level = 1.0f;
    master_volume = 0.7f;
    vco1_waveform = Oscillator::WAVE_SAW;
    vco2_waveform = Oscillator::WAVE_SAW;
    vco3_waveform = Oscillator::WAVE_SAW;


    // Start the audio callback
    hw.StartAudio(AudioCallback);

    while(1)
    {
        UpdateControls();
        UpdateKeyboard();
        hw.display.Update();
        hw.led_driver.SwapBuffersAndTransmit();
    }
}*/


/*      claude  step seq    */
             
#include "daisy_field.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyField hw;
Oscillator osc;
AdEnv env;
Metro clock;

// Sequencer parameters
const int NUM_STEPS = 16;
bool sequence[NUM_STEPS] = {false}; // Step activation states
int current_step = 0;
bool playing = false;
float tempo = 120.0f; // BPM

// Audio parameters
float note_freq = 440.0f;
float env_time = 0.1f;

// Gate output parameters
bool gate_out_state = false;
float gate_length = 0.1f; // Gate length in seconds
uint32_t gate_timer = 0;
uint32_t gate_length_samples = 0;

// LED key mapping for A1-A8 and B1-B8
const int led_keys[NUM_STEPS] = {
    DaisyField::LED_KEY_A1, DaisyField::LED_KEY_A2, DaisyField::LED_KEY_A3, DaisyField::LED_KEY_A4,
    DaisyField::LED_KEY_A5, DaisyField::LED_KEY_A6, DaisyField::LED_KEY_A7, DaisyField::LED_KEY_A8,
    DaisyField::LED_KEY_B1, DaisyField::LED_KEY_B2, DaisyField::LED_KEY_B3, DaisyField::LED_KEY_B4,
    DaisyField::LED_KEY_B5, DaisyField::LED_KEY_B6, DaisyField::LED_KEY_B7, DaisyField::LED_KEY_B8
};

void UpdateControls();
void UpdateSequencer();
void UpdateLEDs();
void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size);

int main(void)
{
    // Initialize hardware
    hw.Init();
    
    // Initialize oscillator
    osc.Init(hw.AudioSampleRate());
    osc.SetWaveform(Oscillator::WAVE_SQUARE);
    osc.SetFreq(note_freq);
    osc.SetAmp(0.5f);
    
    // Initialize envelope
    env.Init(hw.AudioSampleRate());
    env.SetTime(ADENV_SEG_ATTACK, 0.01f);
    env.SetTime(ADENV_SEG_DECAY, env_time);
    env.SetMax(1.0f);
    env.SetMin(0.0f);
    env.SetCurve(0.0f);
    
    // Initialize clock/metro
    clock.Init(tempo / 60.0f * 4.0f, hw.AudioSampleRate()); // 16th notes
    
    // Initialize gate output parameters
    gate_length_samples = (uint32_t)(gate_length * hw.AudioSampleRate());
    
    // Start audio callback
    hw.StartAudio(AudioCallback);
    
    while(1)
    {
        UpdateControls();
        UpdateSequencer();
        UpdateLEDs();
        System::Delay(1); // Small delay to prevent excessive CPU usage
    }
}

void UpdateControls()
{
    hw.ProcessAnalogControls();
    hw.ProcessDigitalControls();
    
    // Check for step key presses (toggle step on/off)
    for(int i = 0; i < NUM_STEPS; i++)
    {
        if(hw.KeyboardRisingEdge(led_keys[i]))
        {
            sequence[i] = !sequence[i]; // Toggle step
        }
    }
    
    // Play/Stop control using GATE_IN
    if(hw.gate_in.Trig())
    {
        playing = !playing;
        if(playing)
        {
            current_step = 0; // Reset to first step when starting
        }
    }
    
    // Tempo control using CV_1 (knob)
    float tempo_cv = hw.GetKnobValue(DaisyField::KNOB_1);
    tempo = 60.0f + (tempo_cv * 180.0f); // 60-240 BPM range
    clock.SetFreq(tempo / 60.0f * 4.0f); // Update clock frequency
    
    // Note frequency control using CV_2 (knob)
    float freq_cv = hw.GetKnobValue(DaisyField::KNOB_2);
    note_freq = 110.0f + (freq_cv * 770.0f); // ~110Hz to 880Hz range
    osc.SetFreq(note_freq);
    
    // Envelope time control using CV_3 (knob)
    float env_cv = hw.GetKnobValue(DaisyField::KNOB_3);
    env_time = 0.05f + (env_cv * 0.45f); // 0.05s to 0.5s range
    env.SetTime(ADENV_SEG_DECAY, env_time);
    
    // Gate length control using CV_4 (knob)
    float gate_cv = hw.GetKnobValue(DaisyField::KNOB_4);
    gate_length = 0.01f + (gate_cv * 0.49f); // 0.01s to 0.5s range
    gate_length_samples = (uint32_t)(gate_length * hw.AudioSampleRate());
}


void UpdateSequencer()
{
    if(playing && clock.Process())
    {
        // Trigger note if current step is active
        if(sequence[current_step])
        {
            env.Trigger();
            
            // Trigger gate output
            gate_out_state = true;
            gate_timer = 0;
        }
        
        // Advance to next step
        current_step = (current_step + 1) % NUM_STEPS;
    }
}

void UpdateLEDs()
{
    // Update step LEDs
    for(int i = 0; i < NUM_STEPS; i++)
    {
        if(i == current_step && playing)
        {
            // Current step: bright LED regardless of sequence state
            hw.led_driver.SetLed(led_keys[i], 1.0f);
        }
        else if(sequence[i])
        {
            // Active step: medium brightness
            hw.led_driver.SetLed(led_keys[i], 0.5f);
        }
        else
        {
            // Inactive step: dim/off
            hw.led_driver.SetLed(led_keys[i], 0.0f);
        }
    }
    
    // Update status LEDs
    hw.led_driver.SetLed(DaisyField::LED_KNOB_1, playing ? 1.0f : 0.2f); // Play/stop indicator
    
    hw.led_driver.SwapBuffersAndTransmit();
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        // Process envelope
        float env_out = env.Process();
        
        // Generate oscillator output
        float osc_out = osc.Process();
        
        // Apply envelope to oscillator
        float signal = osc_out * env_out;
        
        // Output to both channels
        out[0][i] = signal;
        out[1][i] = signal;
        
        // Update gate output timer (sample-accurate timing)
        if(gate_out_state)
        {
            gate_timer++;
            if(gate_timer >= gate_length_samples)
            {
                gate_out_state = false;
                gate_timer = 0;
            }
        }
        
        // Set gate output using GPIO write
        dsy_gpio_write(&hw.gate_out, gate_out_state ? 1 : 0);
    }
}

/*#include "daisy_field.h"    // Field BSP
#include "hid/midi.h"       // libDaisy MIDI‑over‑UART
#include "hid/logger.h"     // libDaisy Logger
#include "daisysp.h"        // DaisySP

using namespace daisy;
using namespace daisysp;

//— MIDI & Scale Configuration —
static constexpr uint8_t MIDI_CHANNEL   = 0;   // 0 = MIDI channel 1
static constexpr uint8_t MIDI_BASE_NOTE = 60;  // C4

uint8_t buttons[16] = {0};
float   scale[16] = {
  0.f,  2.f,  4.f,  5.f,  7.f,  9.f, 11.f, 12.f,
  0.f,  1.f,  3.f,  0.f,  6.f,  8.f, 10.f,  0.f
};
int8_t  octaves = 0;

// Sawtooth oscillator state
static Oscillator    saw;
static bool          saw_on      = false;
static uint8_t       active_note = 0;

//— Hardware Globals —
DaisyField       hw;
MidiUartHandler  midi;
using Log = Logger<LOGGER_INTERNAL>;  // USB-CDC logger citeturn7file9

// Audio callback: generate saw for non-interleaved audio
static void AudioCallback(const float* const* in, float** out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        float s = saw_on ? saw.Process() : 0.0f;
        out[0][i] = s;
        out[1][i] = s;
    }
}

int main(void)
{
    // Init hardware & logging
    hw.Init();                                              // citeturn6file16
    Log::StartLog(true);                                    // wait for USB host citeturn7file9
    Log::PrintLine("Field MIDI + Saw Synth Ready!");

    // MIDI‑UART config (RX = D14, TX = D13)
    MidiUartTransport::Config tuart_cfg;                    // defaults RX/D14, TX/D13 citeturn2file2
    MidiUartHandler::Config    midi_cfg;
    midi_cfg.transport_config = tuart_cfg;
    midi.Init(midi_cfg);                                    // citeturn2file0
    midi.StartReceive();                                    // enable MIDI in citeturn8file0

    // Audio: 48kHz sample rate
    saw.Init(hw.AudioSampleRate());                         // citeturn9file4
    saw.SetWaveform(Oscillator::WAVE_SAW);
    hw.StartAudio(AudioCallback);                          // citeturn9file2

    // Main loop
    while(1)
    {
        hw.ProcessAllControls();                            // scan buttons/switches citeturn9file3

        // Octave switches: SW_1 (down), SW_2 (up)
        if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge())
        {
            octaves = octaves > 0 ? octaves - 1 : 0;
            Log::PrintLine("Octave → %d", octaves);
        }
        if(hw.GetSwitch(DaisyField::SW_2)->RisingEdge())
        {
            octaves = octaves < 4 ? octaves + 1 : 4;
            Log::PrintLine("Octave → %d", octaves);
        }

        // Handle MIDI‑in events
        midi.Listen();
        while(midi.HasEvents())
        {
            auto ev = midi.PopEvent();                       // citeturn8file0
            // Log raw MIDI data
            Log::PrintLine("MIDI In: type=%d ch=%d d0=%d d1=%d", ev.type, ev.channel, ev.data[0], ev.data[1]);

            switch(ev.type)
            {
                case MidiMessageType::NoteOn:
                    if(ev.channel == MIDI_CHANNEL)
                    {
                        active_note = ev.data[0];
                        saw.SetFreq(mtof((float)active_note));
                        saw_on = ev.data[1] > 0;
                    }
                    break;
                case MidiMessageType::NoteOff:
                    if(ev.channel == MIDI_CHANNEL && ev.data[0] == active_note)
                    {
                        saw_on = false;
                    }
                    break;
                default: break;
            }
        }

        // 16-button keyboard → MIDI‑out
        for(size_t i = 0; i < 16; i++)
        {
            if(hw.KeyboardRisingEdge(i))
            {
                uint8_t note = MIDI_BASE_NOTE + (uint8_t)scale[i] + 12*octaves;
                uint8_t msg_on[3]  = { static_cast<uint8_t>(0x90|MIDI_CHANNEL), note, 100 };
                midi.SendMessage(msg_on, 3);
                Log::PrintLine("Key %2d ON → %3d", i, note);
                buttons[i] = 1;
            }
            else if(hw.KeyboardFallingEdge(i) && buttons[i])
            {
                uint8_t note = MIDI_BASE_NOTE + (uint8_t)scale[i] + 12*octaves;
                uint8_t msg_off[3] = { static_cast<uint8_t>(0x80|MIDI_CHANNEL), note, 0 };
                midi.SendMessage(msg_off, 3);
                Log::PrintLine("Key %2d OFF→ %3d", i, note);
                buttons[i] = 0;
            }
        }

        hw.DelayMs(5);
    }
}*/



/*#include "daisy_field.h"    // Field BSP
#include "hid/midi.h"       // libDaisy MIDI‐over‐UART
#include "hid/logger.h"     // libDaisy Logger
#include "daisysp.h"        // DaisySP

using namespace daisy;
using namespace daisysp;

//— MIDI & Scale Configuration —
static constexpr uint8_t MIDI_CHANNEL   = 0;   // 0 = MIDI channel 1
static constexpr uint8_t MIDI_BASE_NOTE = 60;  // C4

uint8_t buttons[16] = {0};
float   scale[16] = {
  0.f, 2.f, 4.f, 5.f, 7.f, 9.f,11.f,12.f,
  0.f, 1.f, 3.f, 0.f, 6.f, 8.f,10.f, 0.f
};
int8_t  octaves = 0;

//— Hardware Globals —
DaisyField       hw;
MidiUartHandler  midi;
using Log = Logger<LOGGER_INTERNAL>;  // shorthand for the USB logger :contentReference[oaicite:2]{index=2}:contentReference[oaicite:3]{index=3}

int main(void)
{
  // 1) Init Field & wait for USB-Serial connection
  hw.Init();                            // :contentReference[oaicite:4]{index=4}:contentReference[oaicite:5]{index=5}
  Log::StartLog(true);                  // block until host opens port :contentReference[oaicite:6]{index=6}:contentReference[oaicite:7]{index=7}
  Log::PrintLine("Field MIDI Keyboard Ready!");

  // 2) MIDI-UART: default is USART1 TX=D13, RX=D14, 256-byte DMA buffer
  MidiUartTransport::Config tuart_cfg;  // defaults: TX=D13 for TRS-MIDI out :contentReference[oaicite:8]{index=8}:contentReference[oaicite:9]{index=9}
  MidiUartHandler::Config midi_cfg;
  midi_cfg.transport_config = tuart_cfg;
  midi.Init(midi_cfg);                  // :contentReference[oaicite:10]{index=10}:contentReference[oaicite:11]{index=11}

  // 3) Main loop: scan controls, handle octave switches, buttons → MIDI
  while(1)
  {
    hw.ProcessAllControls();            // scan SW_1, SW_2, and 16-key shift-reg :contentReference[oaicite:12]{index=12}:contentReference[oaicite:13]{index=13}

    // Octave down (SW_1), up (SW_2)
    if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge())
    {
      octaves = octaves > 0 ? octaves - 1 : 0;
      Log::PrintLine("Octave → %d", octaves);
    }
    if(hw.GetSwitch(DaisyField::SW_2)->RisingEdge())
    {
      octaves = octaves < 4 ? octaves + 1 : 4;
      Log::PrintLine("Octave → %d", octaves);
    }

    // 16-button keyboard (0..7 = chromatic, 8..15 = scale)
    for(size_t i = 0; i < 16; i++)
    {
      if(hw.KeyboardRisingEdge(i))
      {
        uint8_t note = MIDI_BASE_NOTE
                     + static_cast<uint8_t>(scale[i])
                     + 12 * octaves;

        uint8_t msg_on[3]  = { static_cast<uint8_t>(0x90 | MIDI_CHANNEL),
                               note,
                               100 };
        midi.SendMessage(msg_on, 3);     // :contentReference[oaicite:14]{index=14}

        Log::PrintLine("Key %2d ON → %3d", int(i), note);
        buttons[i] = 1;
      }
      else if(hw.KeyboardFallingEdge(i) && buttons[i])
      {
        uint8_t note = MIDI_BASE_NOTE
                     + static_cast<uint8_t>(scale[i])
                     + 12 * octaves;

        uint8_t msg_off[3] = { static_cast<uint8_t>(0x80 | MIDI_CHANNEL),
                               note,
                               0 };
        midi.SendMessage(msg_off, 3);

        Log::PrintLine("Key %2d OFF→ %3d", int(i), note);
        buttons[i] = 0;
      }
    }

    hw.DelayMs(5);                     // simple debounce / yield :contentReference[oaicite:16]{index=16}:contentReference[oaicite:17]{index=17}
  }
}
*/