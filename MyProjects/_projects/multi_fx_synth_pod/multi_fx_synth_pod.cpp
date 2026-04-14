#include "daisy_pod.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyPod hw;

// --- DSP Modules ---

// Synth
Oscillator osc1, osc2;
Adsr       env;

// FX Chain
Overdrive               drive;
Compressor              comp;
DelayLine<float, 48000> del;
Chorus                  cho;
DcBlock                 dc;
Limiter                 lim;

// --- Signal Variables ---
float sig_osc1       = 0.0f;
float sig_osc2       = 0.0f;
float sig_synth      = 0.0f;
float sig_mixer_main = 0.0f;

// --- Control Variables ---
volatile float pitch     = 440.0f;
volatile bool  gate      = false;
volatile float osc_blend = 0.5f; // Knob 1
volatile float drv_val   = 0.5f; // Knob 2

volatile float del_time  = 0.4f; // Knob 1 (Shift)
volatile float cho_depth = 0.3f; // Knob 2 (Shift)

volatile bool shift_mode = false; // Button 1
volatile bool fx_bypass  = false; // Button 2

// --- Helper Functions ---

void ProcessMidi()
{
    hw.midi.Listen();
    while(hw.midi.HasEvents())
    {
        auto msg = hw.midi.PopEvent();
        if(msg.type == NoteOn)
        {
            auto note_msg = msg.AsNoteOn();
            if(note_msg.velocity != 0)
            {
                pitch = mtof(note_msg.note);
                gate  = true;
            }
            else
            {
                gate = false;
            }
        }
        else if(msg.type == NoteOff)
        {
            gate = false;
        }
    }
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // MUST process analog controls strictly at audio block rate for proper filtering (BUG-005)
    hw.ProcessAnalogControls();

    if(!shift_mode)
    {
        // Base Mode
        osc_blend = hw.knob1.Value();
        drv_val   = hw.knob2.Value();
        drive.SetDrive((float)drv_val);
    }
    else
    {
        // Shift Mode
        del_time  = hw.knob1.Value();
        cho_depth = hw.knob2.Value();

        del.SetDelay(hw.AudioSampleRate()
                     * (0.05f + (float)del_time * 0.75f)); // 50ms to 800ms
        cho.SetLfoDepth((float)cho_depth);
    }

    for(size_t i = 0; i < size; i++)
    {
        // 1. Synth Processing
        float env_out = env.Process(gate);

        // Detune Logic: Osc 2 is 5 cents higher (subtle detune)
        osc1.SetFreq((float)pitch);
        osc2.SetFreq((float)pitch * 1.0029f); // ~5 cents

        sig_osc1 = osc1.Process();
        sig_osc2 = osc2.Process();

        // Mixer 1: Crossfade (0% = Osc1, 100% = Osc2)
        // Linear crossfade as requested
        sig_synth = (sig_osc1 * (1.0f - (float)osc_blend)) + (sig_osc2 * (float)osc_blend);
        sig_synth *= env_out;

        // 2. Main Mixer (Synth + Audio In)
        // mixer* summing synth and external audio in
        float audio_in = (in[0][i] + in[1][i]) * 0.5f; // Mono sum of input
        sig_mixer_main = sig_synth + audio_in;

        // 3. FX Chain: overdrive -> compressor -> delay -> chorus -> dc block -> limiter
        float out_sig = sig_mixer_main;

        if(!fx_bypass)
        {
            // Overdrive
            out_sig = drive.Process(out_sig);

            // Compressor
            out_sig = comp.Process(out_sig);

            // Delay
            float del_out = del.Read();
            del.Write(out_sig + (del_out * 0.5f));         // 50% feedback
            out_sig = (out_sig * 0.7f) + (del_out * 0.3f); // 30% wet

            // Chorus
            cho.Process(out_sig);
            float cho_l = cho.GetLeft();
            float cho_r = cho.GetRight();

            // DC Block
            cho_l = dc.Process(cho_l);
            cho_r = dc.Process(cho_r);

            // Limiter
            lim.ProcessBlock(&cho_l, 1, 1.0f);
            lim.ProcessBlock(&cho_r, 1, 1.0f);

            // Final Output
            out[0][i] = cho_l;
            out[1][i] = cho_r;
        }
        else
        {
            // Bypassed: Just DC Block and Limiter for safety
            float final_l = dc.Process(out_sig);
            float final_r = dc.Process(out_sig);
            lim.ProcessBlock(&final_l, 1, 1.0f);
            lim.ProcessBlock(&final_r, 1, 1.0f);
            out[0][i] = final_l;
            out[1][i] = final_r;
        }
    }
}

int main(void)
{
    // Initialize Hardware
    hw.Init();
    hw.SetAudioBlockSize(4);
    float sample_rate = hw.AudioSampleRate();

    // Initialize Synth
    osc1.Init(sample_rate);
    osc1.SetWaveform(Oscillator::WAVE_SAW);
    osc2.Init(sample_rate);
    osc2.SetWaveform(Oscillator::WAVE_SAW);

    env.Init(sample_rate);
    env.SetTime(ADSR_SEG_ATTACK, 0.01f);
    env.SetTime(ADSR_SEG_DECAY, 0.1f);
    env.SetSustainLevel(0.8f);
    env.SetTime(ADSR_SEG_RELEASE, 0.2f);

    // Initialize FX
    drive.Init();

    comp.Init(sample_rate);
    comp.SetThreshold(-20.0f);
    comp.SetRatio(4.0f);

    del.Init();
    del.SetDelay(sample_rate * 0.4f); // 400ms delay

    cho.Init(sample_rate);
    cho.SetLfoFreq(0.5f);
    cho.SetLfoDepth(0.3f);

    dc.Init(sample_rate);

    lim.Init();

    // Start Processes
    hw.StartAdc();
    hw.midi.StartReceive();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        hw.ProcessDigitalControls();
        ProcessMidi();

        // Button 1: Toggle Shift Mode
        if(hw.button1.RisingEdge())
        {
            shift_mode = !shift_mode;
        }

        // Button 2: Toggle Global FX Bypass
        if(hw.button2.RisingEdge())
        {
            fx_bypass = !fx_bypass;
        }

        // Handle LEDs
        if(!shift_mode)
        {
            // Visual Feedback: Blue LED for Base Mode
            hw.led1.Set(0, 0, 1);
            hw.led2.Set(0, 0, fx_bypass ? 0 : 1); // LED 2 indicates Bypass status
        }
        else
        {
            // Visual Feedback: Green LED for Shift Mode
            hw.led1.Set(0, 1, 0);
            hw.led2.Set(0, fx_bypass ? 0 : 1, 0);
        }

        hw.UpdateLeds();
        System::Delay(1);
    }
}
