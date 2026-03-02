

#include "daisy_field.h"
#include "daisysp.h"

#define NUM_VOICES 16

using namespace daisy;

DaisyField hw;


struct voice
{
    void Init(float samplerate)
    {
        osc_.Init(samplerate);
        amp_ = 0.0f;
        osc_.SetAmp(0.7f);
        osc_.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);
        on_ = false;
    }
    float Process()
    {
        float sig;
        amp_ += 0.0025f * ((on_ ? 1.0f : 0.0f) - amp_);
        sig = osc_.Process() * amp_;
        return sig;
    }
    void set_note(float nn) { osc_.SetFreq(daisysp::mtof(nn)); }

    daisysp::Oscillator osc_;
    float               amp_, midibase_;
    bool                on_;
};

voice   v[NUM_VOICES];
uint8_t buttons[16];
// Use bottom row to set major scale
// Top row chromatic notes, and the inbetween notes are just the octave.
float scale[16]   = {0.f,
                   2.f,
                   4.f,
                   5.f,
                   7.f,
                   9.f,
                   11.f,
                   12.f,
                   0.f,
                   1.f,
                   3.f,
                   0.f,
                   6.f,
                   8.f,
                   10.f,
                   0.0f};
float active_note = scale[0];

int8_t octaves = 0;

static daisysp::ReverbSc verb;
// Use two side buttons to change octaves.
float kvals[8];
float cvvals[4];

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    bool trig, use_verb;
    trig = false;
    hw.ProcessAnalogControls();
    hw.ProcessDigitalControls();
    if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge())
    {
        octaves -= 1;
        trig = true;
    }
    if(hw.GetSwitch(DaisyField::SW_2)->RisingEdge())
    {
        octaves += 1;
        trig = true;
    }
    use_verb = true;

    for(int i = 0; i < 8; i++)
    {
        kvals[i] = hw.GetKnobValue(i);
        if(i < 4)
        {
            cvvals[i] = hw.GetCvValue(i);
        }
    }

    if(octaves < 0)
        octaves = 0;
    if(octaves > 4)
        octaves = 4;

    if(trig)
    {
        for(int i = 0; i < NUM_VOICES; i++)
        {
            v[i].set_note((12.0f * octaves) + 24.0f + scale[i]);
        }
    }
    for(size_t i = 0; i < 16; i++)
    {
        v[i].on_ = hw.KeyboardState(i);
    }
    float sig, send;
    float wetl, wetr;
    for(size_t i = 0; i < size; i += 2)
    {
        sig = 0.0f;
        for(int i = 0; i < NUM_VOICES; i++)
        {
            if(i != 8 && i != 11 && i != 15)
                sig += v[i].Process();
        }
        send = sig * 0.35f;
        verb.Process(send, send, &wetl, &wetr);
        //        wetl = wetr = sig;
        if(!use_verb)
            wetl = wetr = 0.0f;
        out[i]     = (sig + wetl) * 0.5f;
        out[i + 1] = (sig + wetr) * 0.5f;
    }
}

void UpdateLeds(float *knob_vals)
{
    // knob_vals is exactly 8 members
    size_t knob_leds[] = {
        DaisyField::LED_KNOB_1,
        DaisyField::LED_KNOB_2,
        DaisyField::LED_KNOB_3,
        DaisyField::LED_KNOB_4,
        DaisyField::LED_KNOB_5,
        DaisyField::LED_KNOB_6,
        DaisyField::LED_KNOB_7,
        DaisyField::LED_KNOB_8,
    };
    size_t keyboard_leds[] = {
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
    for(size_t i = 0; i < 8; i++)
    {
        hw.led_driver.SetLed(knob_leds[i], knob_vals[i]);
    }
    for(size_t i = 0; i < 13; i++)
    {
        hw.led_driver.SetLed(keyboard_leds[i], 1.f);
    }
    hw.led_driver.SwapBuffersAndTransmit();
}

int main(void)
{
    float sr;
    hw.Init();
    sr = hw.AudioSampleRate();
    // Initialize controls.
    octaves = 2;
    for(int i = 0; i < NUM_VOICES; i++)
    {
        v[i].Init(sr);
        v[i].set_note((12.0f * octaves) + 24.0f + scale[i]);
    }

    verb.Init(sr);
    verb.SetFeedback(0.94f);
    verb.SetLpFreq(8000.0f);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    for(;;)
    {
        UpdateLeds(kvals);
        System::Delay(1);
        hw.seed.dac.WriteValue(DacHandle::Channel::ONE,
                               hw.GetKnobValue(0) * 4095);
        hw.seed.dac.WriteValue(DacHandle::Channel::TWO,
                               hw.GetKnobValue(1) * 4095);
    }
}




// #include "daisy_field.h"
// #include "daisysp.h"

// using namespace daisy;
// using namespace daisysp;

// DaisyField  hw;
// Oscillator  osc1, osc2;
// AdEnv       env;

// // Current oscillator waveforms (0 = WAVE_SIN, 1 = WAVE_TRI, 2 = WAVE_SAW, 3 = WAVE_SQUARE, 4 = WAVE_POLYBLEP_TRI, 5 = WAVE_POLYBLEP_SAW, 6 = WAVE_POLYBLEP_SQUARE)
// uint8_t     osc1_waveform = 0;
// uint8_t     osc2_waveform = 0;

// // Button definitions in rows from top to bottom
// // and columns from left to right
// // i.e. button 0 is row 0, column 0, which is the top left button.
// enum Button
// {
//     BUTTON_C4,
//     BUTTON_D4,
//     BUTTON_E4,
//     BUTTON_F4,
//     BUTTON_G4,
//     BUTTON_A4,
//     BUTTON_B4,
//     BUTTON_C5,
//     BUTTON_C_SHARP_4,
//     BUTTON_D_SHARP_4,
//     BUTTON_NONE_1,
//     BUTTON_F_SHARP_4,
//     BUTTON_G_SHARP_4,
//     BUTTON_A_SHARP_4,
//     BUTTON_NONE_2,
//     BUTTON_NONE_3,
// };

// // Array of frequencies for chromatic scale
// // starting at C4 (middle C)
// float notes[16] = {
//     261.63f, // C4
//     293.66f, // D4
//     329.63f, // E4
//     349.23f, // F4
//     392.00f, // G4
//     440.00f, // A4
//     493.88f, // B4
//     523.25f, // C5
//     277.18f, // C#4
//     311.13f, // D#4
//     0.0f,    // None
//     369.99f, // F#4
//     415.30f, // G#4
//     466.16f, // A#4
//     0.0f,    // None
//     0.0f,    // None
// };

// // Array of active notes
// bool      active_notes[16];

// static void AudioCallback(AudioHandle::InputBuffer  in,
//                           AudioHandle::OutputBuffer out,
//                           size_t                    size)
// {
//     float osc1_out, osc2_out, sig_out;
//     // Process Audio
//     for(size_t i = 0; i < size; i++)
//     {
//         float env_out = env.Process();
        
//         osc1_out = osc1.Process() * hw.knob[3].Value();
//         osc2_out = osc2.Process() * hw.knob[4].Value();

//         // Mix both oscillators
//         sig_out = (osc1_out + osc2_out) * 0.5f * env_out;

//         // Output to both channels
//         out[0][i] = sig_out;
//         out[1][i] = sig_out;
//     }
// }

// // Switch the waveform for oscillator 1
// void CycleWaveform1()
// {
//     osc1_waveform = (osc1_waveform + 1) % 7;
    
//     // Set the new waveform
//     switch(osc1_waveform)
//     {
//         case 0: osc1.SetWaveform(Oscillator::WAVE_SIN); break;
//         case 1: osc1.SetWaveform(Oscillator::WAVE_TRI); break;
//         case 2: osc1.SetWaveform(Oscillator::WAVE_SAW); break;
//         case 3: osc1.SetWaveform(Oscillator::WAVE_SQUARE); break;
//         case 4: osc1.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI); break;
//         case 5: osc1.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW); break;
//         case 6: osc1.SetWaveform(Oscillator::WAVE_POLYBLEP_SQUARE); break;
//         default: osc1.SetWaveform(Oscillator::WAVE_SIN); break;
//     }
// }

// // Switch the waveform for oscillator 2
// void CycleWaveform2()
// {
//     osc2_waveform = (osc2_waveform + 1) % 7;
    
//     // Set the new waveform
//     switch(osc2_waveform)
//     {
//         case 0: osc2.SetWaveform(Oscillator::WAVE_SIN); break;
//         case 1: osc2.SetWaveform(Oscillator::WAVE_TRI); break;
//         case 2: osc2.SetWaveform(Oscillator::WAVE_SAW); break;
//         case 3: osc2.SetWaveform(Oscillator::WAVE_SQUARE); break;
//         case 4: osc2.SetWaveform(Oscillator::WAVE_POLYBLEP_TRI); break;
//         case 5: osc2.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW); break;
//         case 6: osc2.SetWaveform(Oscillator::WAVE_POLYBLEP_SQUARE); break;
//         default: osc2.SetWaveform(Oscillator::WAVE_SIN); break;
//     }
// }

// int main(void)
// {
//     // Initialize the Daisy Field hardware
//     hw.Init();
    
//     // Configure envelope generator
//     env.Init(hw.AudioSampleRate());
//     env.SetTime(ADENV_SEG_ATTACK, 0.01f);
//     env.SetTime(ADENV_SEG_DECAY, 0.2f);
//     env.SetMax(1.0f);
//     env.SetMin(0.0f);
//     env.SetCurve(0.0f); // Linear

//     // Configure oscillator 1
//     osc1.Init(hw.AudioSampleRate());
//     osc1.SetWaveform(Oscillator::WAVE_SIN);
//     osc1.SetAmp(1.0f);
//     osc1.SetFreq(440.0f);

//     // Configure oscillator 2
//     osc2.Init(hw.AudioSampleRate());
//     osc2.SetWaveform(Oscillator::WAVE_SIN);
//     osc2.SetAmp(1.0f);
//     osc2.SetFreq(440.0f);

//     // Init array of active notes
//     for(int i = 0; i < 16; i++)
//     {
//         active_notes[i] = false;
//     }

//     hw.StartAdc();
//     hw.StartAudio(AudioCallback);

//     while(1)
//     {
//         // Process buttons
//         hw.ProcessAllControls();

//         // Update osc1 frequency with knob1
//         float osc1_freq_mod = hw.knob[1].Value();
//         osc1.SetFreq(440.0f + (osc1_freq_mod * 1000.0f)); // Range from 440Hz to 1440Hz

//         // Update osc2 frequency with knob2
//         float osc2_freq_mod = hw.knob[2].Value();
//         osc2.SetFreq(440.0f + (osc2_freq_mod * 1000.0f)); // Range from 440Hz to 1440Hz

//         // Check for SW1 press to change osc1 waveform
//         if(hw.sw[0].RisingEdge())
//         {
//             CycleWaveform1();
//             // Debounce
//             System::Delay(200);
//         }

//         // Check for SW2 press to change osc2 waveform
//         if(hw.sw[1].RisingEdge())
//         {
//             CycleWaveform2();
//             // Debounce
//             System::Delay(200);
//         }

//         // Get the key that was pressed (if any)
//         int idx = hw.KeyboardRisingEdge();
//         if(idx != -1 && notes[idx] > 0.0f)
//         {
//             active_notes[idx] = true;
            
//             // Retrigger envelope
//             env.Trigger();
            
//             // Set both oscillators to the note frequency
//             osc1.SetFreq(notes[idx] * (1.0f + hw.knob[1].Value()));
//             osc2.SetFreq(notes[idx] * (1.0f + hw.knob[2].Value()));
//         }

//         // Get the key that was released (if any)
//         idx = hw.KeyboardFallingEdge();
//         if(idx != -1)
//         {
//             active_notes[idx] = false;
//         }

//         // Initialize variable to check if any notes are active
//         bool any_active = false;
//         for(int i = 0; i < 16; i++)
//         {
//             if(active_notes[i] && notes[i] > 0.0f)
//             {
//                 any_active = true;
//                 break;
//             }
//         }

//         // If no notes are active, decay the envelope
//         if(!any_active)
//         {
//             // No active buttons - keep oscillators at frequency set by knobs
//         }
//     }
// }