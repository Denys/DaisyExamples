

/*          GPT 01 generated code           */

/*             VERSION 4    REVERB          */

// #include "daisy_field.h"
// #include "daisysp.h"

// using namespace daisy;
// using namespace daisysp;

// // Number of keys on Daisy Field
// #define NUM_VOICES 16

// // Number of waveforms to cycle through
// #define NUM_WAVEFORMS 4

// /**
//     Class: Voice
//     ------------
//     Represents a single voice with:
//       - Two oscillators (osc1, osc2)
//       - A smoothing envelope (amp_env_) for gating (on/off)
//       - user-set amplitudes for each oscillator (amp1_, amp2_)
//       - a gate flag on_
// */
// class Voice
// {
//   public:
//     void Init(float samplerate)
//     {
//         // Init both oscillators
//         osc1.Init(samplerate);
//         osc2.Init(samplerate);

//         // Default waveforms => sine
//         osc1.SetWaveform(Oscillator::WAVE_SIN);
//         osc2.SetWaveform(Oscillator::WAVE_SIN);

//         // Internal amplitude envelope for gating
//         amp_env_ = 0.f;

//         // Oscillator internal amps are set to 1;
//         // We'll scale by amp1_, amp2_ in Process()
//         osc1.SetAmp(1.f);
//         osc2.SetAmp(1.f);

//         // By default, voice is off
//         on_ = false;
//     }

//     // Returns one sample, mixing osc1 and osc2, scaled by gate envelope
//     float Process()
//     {
//         // Gate envelope transitions from 0 -> 1 or 1 -> 0 smoothly
//         float target = on_ ? 1.f : 0.f;
//         amp_env_ += 0.0025f * (target - amp_env_);

//         float s1 = osc1.Process() * amp1_;
//         float s2 = osc2.Process() * amp2_;
//         float mix = (s1 + s2) * amp_env_;
//         return mix;
//     }

//     // Frequencies
//     void SetFreq1(float freq) { osc1.SetFreq(freq); }
//     void SetFreq2(float freq) { osc2.SetFreq(freq); }

//     // Amplitudes
//     void SetAmp1(float amp) { amp1_ = amp; }
//     void SetAmp2(float amp) { amp2_ = amp; }  // Waveforms
//     void SetWaveform1(uint8_t waveform) { osc1.SetWaveform(waveform); }
//     void SetWaveform2(uint8_t waveform) { osc2.SetWaveform(waveform); }

//     // Gate
//     bool on_;

//   private:
//     Oscillator osc1, osc2;
//     float amp1_, amp2_;
//     float amp_env_;
// };

// // Global hardware object
// DaisyField hw;

// // We'll have 16 voices, one per key
// static Voice voices[NUM_VOICES];

// // A semitone offset array for each of the 16 keys
// static float scale[NUM_VOICES] = {
//     0.f,  2.f,  4.f,  5.f,  7.f,  9.f,  11.f, 12.f,
//     0.f,  1.f,  3.f,  0.f,  6.f,  8.f, 10.f,  0.f
// };

// // For cycling waveforms with SW_1 and SW_2
// static int osc1_waveform_idx = 0;
// static int osc2_waveform_idx = 0;

// // Our set of waveforms (sine, triangle, polyblep saw, polyblep square)
// static uint8_t WAVEFORMS[NUM_WAVEFORMS] = {
//     Oscillator::WAVE_SIN,
//     Oscillator::WAVE_TRI,
//     Oscillator::WAVE_POLYBLEP_SAW,
//     Oscillator::WAVE_POLYBLEP_SQUARE
// };

// // Reverb object
// //static daisysp::ReverbSc verb;
// static ReverbSc verb;

// /**
//     Audio Callback:
//     - Reads knobs, switches
//     - Sets oscillator freq, amp, wave
//     - Processes reverb after mixing
// */
// void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
//                    AudioHandle::InterleavingOutputBuffer out,
//                    size_t                                size)
// {
//     hw.ProcessAnalogControls();
//     hw.ProcessDigitalControls();

//     // Cycle waveform for osc1 on SW_1 press
//     if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge())
//     {
//         osc1_waveform_idx = (osc1_waveform_idx + 1) % NUM_WAVEFORMS;
//     }
//     // Cycle waveform for osc2 on SW_2 press
//     if(hw.GetSwitch(DaisyField::SW_2)->RisingEdge())
//     {
//         osc2_waveform_idx = (osc2_waveform_idx + 1) % NUM_WAVEFORMS;
//     }

//     // Read knobs:
//     // Knob1 => freq offset for osc1,
//     // Knob2 => freq offset for osc2,
//     // Knob3 => amplitude for osc1,
//     // Knob4 => amplitude for osc2,
//     float freq_mod_osc1 = hw.GetKnobValue(0) * 5.f;
//     float freq_mod_osc2 = hw.GetKnobValue(1) * 5.f;
//     float amp_osc1      = hw.GetKnobValue(2);
//     float amp_osc2      = hw.GetKnobValue(3);

//     // Update each Voice
//     for(int i = 0; i < NUM_VOICES; i++)
//     {
//         voices[i].on_ = hw.KeyboardState(i);

//         // Base note = MIDI 48 => C3
//         float base_midi = 48.f + scale[i];

//         // Convert to frequency (and add freq_mod to create pitch variation)
//         float f1 = daisysp::mtof(base_midi + freq_mod_osc1 * 12.f);
//         float f2 = daisysp::mtof(base_midi + freq_mod_osc2 * 12.f);

//         voices[i].SetFreq1(f1);
//         voices[i].SetFreq2(f2);

//         voices[i].SetAmp1(amp_osc1);
//         voices[i].SetAmp2(amp_osc2);

//         voices[i].SetWaveform1(WAVEFORMS[osc1_waveform_idx]);
//         voices[i].SetWaveform2(WAVEFORMS[osc2_waveform_idx]);
//     }

//     // Audio generation
//     for(size_t i = 0; i < size; i += 2)
//     {
//         float mix = 0.f;
//         for(int v = 0; v < NUM_VOICES; v++)
//         {
//             mix += voices[v].Process();
//         }
//         // scale down
//         mix *= 0.3f;

//         // Send signal to reverb
//         float wetL, wetR;
//         verb.Process(mix, mix, &wetL, &wetR);

//         // Example: mix 50/50 dry/wet
//         float outL = (mix + wetL) * 0.5f;
//         float outR = (mix + wetR) * 0.5f;

//         out[i]     = outL;
//         out[i + 1] = outR;
//     }
// }

// /**
//     UpdateLeds:
//     - Lights knob LEDs according to the knob positions
//     - Lights key LEDs if key is pressed
// */
// void UpdateLeds()
// {
//     static const size_t knob_leds[8] = {
//         DaisyField::LED_KNOB_1,
//         DaisyField::LED_KNOB_2,
//         DaisyField::LED_KNOB_3,
//         DaisyField::LED_KNOB_4,
//         DaisyField::LED_KNOB_5,
//         DaisyField::LED_KNOB_6,
//         DaisyField::LED_KNOB_7,
//         DaisyField::LED_KNOB_8,
//     };

//     // For each knob (0..7), set LED brightness
//     for(int k = 0; k < 8; k++)
//     {
//         float val = hw.GetKnobValue(k);
//         hw.led_driver.SetLed(knob_leds[k], val);
//     }

//     // Light each key LED if pressed
//     for(int i = 0; i < NUM_VOICES; i++)
//     {
//         float b = hw.KeyboardState(i) ? 1.0f : 0.0f;
//         hw.led_driver.SetLed(DaisyField::LED_KEY_A1 + i, b);
//     }

//     hw.led_driver.SwapBuffersAndTransmit();
// }

// /**
//     main:
//     - Initialize hardware
//     - Initialize reverb + voices
//     - Start audio
//     - Loop with LED updates
// */
// int main(void)
// {
//     // Init Daisy Field
//     hw.Init();
//     float sr = hw.AudioSampleRate();

//     // Init Reverb
//     verb.Init(sr);
//     // Example settings: moderate feedback, high LPF
//     verb.SetFeedback(0.88f);
//     verb.SetLpFreq(10000.0f);

//     // Init voices
//     for(int i = 0; i < NUM_VOICES; i++)
//     {
//         voices[i].Init(sr);
//     }

//     // Start reading knobs and audio
//     hw.StartAdc();
//     hw.StartAudio(AudioCallback);

//     while(true)
//     {
//         UpdateLeds();
//         System::Delay(1);
//     }
//     return 0;
// }


// /*          VERSION 3    WORKING         */
// #include "daisy_field.h"
// #include "daisysp.h"

// // Use these namespaces for brevity
// using namespace daisy;
// using namespace daisysp;

// // Number of keys on Daisy Field
// #define NUM_VOICES 16

// // Number of waveforms we want to cycle
// #define NUM_WAVEFORMS 4

// /**
//     Voice structure:
//     - Two oscillators (osc1, osc2)
//     - Smooth gate amplitude (amp_env) to avoid clicks
//     - A gate flag on_ that is set if the key is pressed
// */
// class Voice
// {
//   public:
//     // Initializes both oscillators and sets them silent
//     void Init(float samplerate)
//     {
//         osc1.Init(samplerate);
//         osc2.Init(samplerate);

//         // Default waveforms => Sine
//         osc1.SetWaveform(Oscillator::WAVE_SIN);
//         osc2.SetWaveform(Oscillator::WAVE_SIN);

//         // Start with zero amplitude
//         amp_env_ = 0.0f;

//         // By default, both internal amplitudes are 1.0
//         // We'll scale them in Process() for "amp1_" and "amp2_".
//         osc1.SetAmp(1.0f);
//         osc2.SetAmp(1.0f);

//         on_ = false;
//     }

//     // Process one sample. If on_ is true, we fade in; otherwise fade out
//     float Process()
//     {
//         // Smooth gating to prevent clicks
//         float target = on_ ? 1.0f : 0.0f;
//         amp_env_ += 0.0025f * (target - amp_env_);

//         // Generate signals from each oscillator
//         float s1 = osc1.Process() * amp1_;
//         float s2 = osc2.Process() * amp2_;

//         // Sum the two oscillator signals, then multiply by gate envelope
//         float mix = (s1 + s2) * amp_env_;

//         return mix;
//     }

//     // Set the frequency of osc1 (in Hz)
//     void SetFreq1(float freq) { osc1.SetFreq(freq); }
//     // Set the frequency of osc2 (in Hz)
//     void SetFreq2(float freq) { osc2.SetFreq(freq); }

//     // Set amplitude scaling for each oscillator
//     void SetAmp1(float amp) { amp1_ = amp; }
//     void SetAmp2(float amp) { amp2_ = amp; }

//     // Set the waveform for osc1 or osc2
//     void SetWaveform1(uint8_t waveform) { osc1.SetWaveform(waveform); }
//     void SetWaveform2(uint8_t waveform) { osc2.SetWaveform(waveform); }

//     // Gate state (true if key is pressed)
//     bool on_;

//   private:
//     Oscillator osc1, osc2;
//     float amp1_, amp2_;  // user-set amplitudes for each oscillator
//     float amp_env_;      // internal gate envelope
// };

// // Global Daisy Field handle
// DaisyField hw;

// // We have 16 voices, each is a sum of (osc1 + osc2) for that key
// static Voice voices[NUM_VOICES];

// // static daisysp::ReverbSc verb;

// // A scale array to offset each key. (Feel free to change or remove.)
// static float scale[NUM_VOICES] = {
//     0.f, 2.f, 4.f, 5.f, 7.f, 9.f,  11.f, 12.f,  // A1–A8
//     0.f, 1.f, 3.f, 0.f, 6.f, 8.f, 10.f,  0.f
// };

// // We'll keep track of which waveform each oscillator is using
// int osc1_waveform_idx = 0;
// int osc2_waveform_idx = 0;

// // Array of waveforms to cycle with SW_1, SW_2
// uint8_t WAVEFORMS[NUM_WAVEFORMS] = {
//     Oscillator::WAVE_SIN,
//     Oscillator::WAVE_TRI,
//     Oscillator::WAVE_POLYBLEP_SAW,
//     Oscillator::WAVE_POLYBLEP_SQUARE,
// };

// /**
//     Audio Callback
//     - Reads knobs/switches
//     - Sets each voice's oscillator parameters
//     - Sums all voices into the final output
// */
// void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
//                    AudioHandle::InterleavingOutputBuffer out,
//                    size_t                                size)
// {
//     // 1) Read the Field's analog/digital controls
//     hw.ProcessAnalogControls();
//     hw.ProcessDigitalControls();

//     // 2) Cycle waveforms with SW_1, SW_2
//     if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge())
//     {
//         osc1_waveform_idx = (osc1_waveform_idx + 1) % NUM_WAVEFORMS;
//     }
//     if(hw.GetSwitch(DaisyField::SW_2)->RisingEdge())
//     {
//         osc2_waveform_idx = (osc2_waveform_idx + 1) % NUM_WAVEFORMS;
//     }

//     // 3) Read knobs for freq and amp
//     //    - Knob1 => freq offset for Osc1
//     //    - Knob2 => freq offset for Osc2
//     //    - Knob3 => amplitude for Osc1
//     //    - Knob4 => amplitude for Osc2
//     float freq_mod_osc1 = hw.GetKnobValue(0) * 5.0f; // scale to taste
//     float freq_mod_osc2 = hw.GetKnobValue(1) * 5.0f;
//     float amp_osc1      = hw.GetKnobValue(2);
//     float amp_osc2      = hw.GetKnobValue(3);

//     // 4) Configure each voice
//     for(int i = 0; i < NUM_VOICES; i++)
//     {
//         // Check if key i is pressed
//         voices[i].on_ = hw.KeyboardState(i);

//         // Base note: MIDI 48 (C3). Add scale offset => scale[i].
//         // Then add freq offsets (freq_mod_osc1 or freq_mod_osc2).
//         // For demonstration, let's have bottom row (voices 0..7) use freq_mod_osc1,
//         // top row (voices 8..15) use freq_mod_osc2.
//         float base_midi = 48.0f + scale[i];
//         float freq1     = daisysp::mtof(base_midi + freq_mod_osc1 * 12.f); // *12 if you want a bigger range
//         float freq2     = daisysp::mtof(base_midi + freq_mod_osc2 * 12.f);

//         // Set each oscillator frequency
//         voices[i].SetFreq1(freq1);
//         voices[i].SetFreq2(freq2);

//         // Set each oscillator amplitude from knobs
//         voices[i].SetAmp1(amp_osc1);
//         voices[i].SetAmp2(amp_osc2);

//         // Set waveforms
//         voices[i].SetWaveform1(WAVEFORMS[osc1_waveform_idx]);
//         voices[i].SetWaveform2(WAVEFORMS[osc2_waveform_idx]);
//     }

//     // 5) Sum all voices into final output
//     for(size_t i = 0; i < size; i += 2)
//     {
//         float mix = 0.0f;
//         for(int v = 0; v < NUM_VOICES; v++)
//         {
//             mix += voices[v].Process();
//         }
//         // reduce overall volume to avoid clipping
//         mix *= 0.3f;

//         out[i]     = mix; // left
//         out[i + 1] = mix; // right
//     }
// }

// /**
//     UpdateLeds
//     - Light each knob LED based on knob position
//     - Light each key LED if pressed
// */
// void UpdateLeds()
// {
//     static const size_t knob_leds[8] = {
//         DaisyField::LED_KNOB_1,
//         DaisyField::LED_KNOB_2,
//         DaisyField::LED_KNOB_3,
//         DaisyField::LED_KNOB_4,
//         DaisyField::LED_KNOB_5,
//         DaisyField::LED_KNOB_6,
//         DaisyField::LED_KNOB_7,
//         DaisyField::LED_KNOB_8,
//     };

//     // For each knob, set LED brightness
//     for(int k = 0; k < 8; k++)
//     {
//         float val = hw.GetKnobValue(k);
//         hw.led_driver.SetLed(knob_leds[k], val);
//     }

//     // Light each key LED if pressed
//     for(int i = 0; i < NUM_VOICES; i++)
//     {
//         float b = hw.KeyboardState(i) ? 1.0f : 0.0f;
//         hw.led_driver.SetLed(DaisyField::LED_KEY_A1 + i, b);
//     }

//     hw.led_driver.SwapBuffersAndTransmit();
// }

// /**
//     main()
//     - Initialize hardware
//     - Init voices
//     - Start audio
//     - Loop forever, updating LEDs
// */
// int main(void)
// {
//     hw.Init();
//     float sr = hw.AudioSampleRate();

//     // Initialize all voices
//     for(int i = 0; i < NUM_VOICES; i++)
//     {
//         voices[i].Init(sr);
//     }

//     // Start ADC (knobs) and audio
//     hw.StartAdc();
//     hw.StartAudio(AudioCallback);

//     while(true)
//     {
//         UpdateLeds();
//         System::Delay(1);
//     }
//     return 0;
// }


// /*          VERSION 1                       */
// /***************************************************/
// /*  File: DualOscKeyboard.cpp                      */
// /*  Author: Your Name                              */
// /*  Description: Example code for Daisy Field      */
// /*               with 2 oscillators per voice,     */
// /*               waveform switching, and knob      */
// /*               frequency/amplitude control.      */
// /***************************************************/

// #include "daisy_field.h"
// #include "daisysp.h"

// using namespace daisy;     // Daisy library (hardware)
// using namespace daisysp;   // Daisy library (DSP)

// // Number of voices matches number of keys on Daisy Field.
// #define NUM_VOICES 16

// #define NUM_WAVEFORMS 4

// /**
//  *  Class: Voice
//  *  ------------
//  *  Represents a single "voice" with two oscillators (osc1, osc2),
//  *  amplitude ramp, and an "on/off" gate indicating if the key is pressed.
//  */
// class Voice
// {
//   public:
//     /**
//      *  Initializes the oscillators with default amplitude,
//      *  waveforms, and sets them off (gate = false).
//      *
//      *  @param samplerate: The audio sample rate (e.g., 48014 Hz).
//      */
//     void Init(float samplerate)
//     {
//         // Initialize oscillators
//         osc1.Init(samplerate);
//         osc2.Init(samplerate);

//         // Set default amplitude
//         osc1.SetAmp(0.3f);
//         osc2.SetAmp(0.3f);

//         // Set default waveforms to Sine
//         osc1.SetWaveform(Oscillator::WAVE_SIN);
//         osc2.SetWaveform(Oscillator::WAVE_SIN);

//         // Amplitude envelope multiplier for gating
//         amp_ = 0.0f;

//         // Key gate initially off
//         on_  = false;
//     }

//     /**
//      *  Processes one sample frame, returning the mixed signal
//      *  of osc1 + osc2, shaped by the amplitude envelope.
//      *
//      *  @return float: One sample of audio.
//      */
//     float Process()
//     {
//         // Simple linear ramp for turning amplitude on/off:
//         // moves towards 1.0 if on_, towards 0.0 if not on_.
//         amp_ += 0.0025f * ((on_ ? 1.0f : 0.0f) - amp_);

//         float sig = (osc1.Process() + osc2.Process()) * amp_ * 0.5f;
//         return sig;
//     }

//     /**
//      *  Sets the frequency (in Hz) of both oscillators,
//      *  converting from MIDI note if needed.
//      *
//      *  @param nn: MIDI note number (e.g., 60 = middle C),
//      *             or direct frequency offset logic can be used.
//      */
//     void SetNote(float nn)
//     {
//         // Convert MIDI note to frequency
//         float freq = mtof(nn);
//         osc1.SetFreq(freq);
//         osc2.SetFreq(freq);
//     }

//     /**
//      *  Sets the amplitude for osc1 and osc2.
//      *
//      *  @param amp1: Amplitude [0.0f..1.0f] for osc1
//      *  @param amp2: Amplitude [0.0f..1.0f] for osc2
//      */
//     void SetAmps(float amp1, float amp2)
//     {
//         osc1.SetAmp(amp1);
//         osc2.SetAmp(amp2);
//     }

//     /**
//      *  Sets the waveform for a specific oscillator.
//      *
//      *  @param is_osc1: True => modifies osc1, False => modifies osc2
//      *  @param waveform: A value from the Oscillator::Waveform enum
//      *                   (WAVE_SIN, WAVE_SAW, WAVE_SQUARE, WAVE_TRI, etc.)
//      */
//     void SetWaveform(bool is_osc1, uint8_t waveform)
//     {
//         if(is_osc1)
//             osc1.SetWaveform(waveform);
//         else
//             osc2.SetWaveform(waveform);
//     }

//     /**
//      *  Gate state for this voice (set by reading keyboard).
//      */
//     bool on_;

//   private:
//     Oscillator osc1, osc2;  // Two independent oscillators
//     float      amp_;        // Smooth gating value (0 => off, 1 => on)
// };

// /*******************************************************/
// /* Global objects and variables                        */
// /*******************************************************/
// DaisyField hw;                  ///< Daisy Field hardware handle
// Voice      voices[NUM_VOICES];  ///< Array of voices, one per key

// // Array to map each of the 16 keys to a note offset (in semitones).
// // This set creates a major scale for A1–A8, plus a partial chromatic scale
// // for B1–B8. Feel free to edit these if you want different scales.
// static float scale[NUM_VOICES] = {
//     0.f, 2.f, 4.f, 5.f, 7.f, 9.f,  11.f, 12.f,  // A1–A8
//     0.f, 1.f, 3.f, 0.f, 6.f, 8.f, 10.f,  0.f
// };

// // Two integers to track which waveform each oscillator is using
// // 0 => Sine, 1 => Saw, 2 => Square, 3 => Tri
// int osc1_waveform_idx = 0;
// int osc2_waveform_idx = 0;

// // Possible waveforms. We cycle these with SW_1, SW_2.
// uint8_t WAVEFORMS[NUM_WAVEFORMS] = {
//     Oscillator::WAVE_SIN,
//     Oscillator::WAVE_TRI,
//     Oscillator::WAVE_POLYBLEP_SAW,
//     Oscillator::WAVE_POLYBLEP_SQUARE,
// };
// // static Oscillator::Waveform WAVEFORMS[4] = {
// //     Oscillator::WAVE_SIN,
// //     Oscillator::WAVE_SAW,
// //     Oscillator::WAVE_SQUARE,
// //     Oscillator::WAVE_TRI
// // };

// // Store knob values for convenience
// float kvals[8];

// /*******************************************************/
// /* Function: AudioCallback                             */
// /* -------------------------                           */
// /* Primary audio engine callback, called automatically */
// /* at the hardware sample rate. Processes hardware     */
// /* controls (knobs, switches), sets oscillator params, */
// /* and writes out audio.                               */
// /*******************************************************/
// void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
//                    AudioHandle::InterleavingOutputBuffer out,
//                    size_t                                size)
// {
//     // 1) Process all analog/digital controls for the Daisy Field
//     hw.ProcessAnalogControls();
//     hw.ProcessDigitalControls();

//     // 2) Handle the wave switching using Switch 1 & Switch 2
//     //    Pressing SW_1 cycles oscillator 1's waveform, SW_2 cycles oscillator 2's
//     if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge())
//     {
//         osc1_waveform_idx = (osc1_waveform_idx + 1) % 4;
//     }
//     if(hw.GetSwitch(DaisyField::SW_2)->RisingEdge())
//     {
//         osc2_waveform_idx = (osc2_waveform_idx + 1) % 4;
//     }

//     // 3) Read the first four knobs to determine freq/amplitude
//     //    - Knob 1 => frequency mod for osc1
//     //    - Knob 2 => frequency mod for osc2
//     //    - Knob 3 => amplitude for osc1
//     //    - Knob 4 => amplitude for osc2
//     for(int i = 0; i < 4; i++)
//     {
//         kvals[i] = hw.GetKnobValue(i);
//     }
//     float freq_mod_osc1 = kvals[0] * 5.0f;  // scale to taste
//     float freq_mod_osc2 = kvals[1] * 5.0f;
//     float amp_osc1      = kvals[2];
//     float amp_osc2      = kvals[3];

//     // 4) Update each of the 16 voices
//     for(int i = 0; i < NUM_VOICES; i++)
//     {
//         // Check if keyboard key i is pressed => sets on_ = true
//         voices[i].on_ = hw.KeyboardState(i);

//         // Base note is 48 => C3. Add scale offset => scale[i].
//         // Then add freq_mod (like a mild transposition).
//         float midi_note = 48.0f + scale[i];

//         // Modify pitch by freq_mod (you can adjust scaling or logic to suit)
//         voices[i].SetNote(midi_note);   //(midi_note + ((i < 8) ? freq_mod_osc1 : freq_mod_osc2));

//         // Set amplitude for each oscillator
//         // For simplicity, if voice < 8 => belongs to osc1 group, otherwise => osc2 group
//         // but you can design your own logic for amplitude distribution
//         // float local_amp1 = (i < 8) ? amp_osc1 : 0.0f;
//         // float local_amp2 = (i < 8) ? 0.0f      : amp_osc2;
//         voices[i].SetAmps(amp_osc1, amp_osc2);   //(local_amp1, local_amp2);

//         // Set waveforms
//         voices[i].SetWaveform(true, WAVEFORMS[osc1_waveform_idx]);  // osc1
//         voices[i].SetWaveform(false, WAVEFORMS[osc2_waveform_idx]); // osc2
//     }

//     // 5) Generate audio
//     for(size_t i = 0; i < size; i += 2)
//     {
//         float mix = 0.0f;
//         for(int v = 0; v < NUM_VOICES; v++)
//         {
//             mix += voices[v].Process();
//         }
//         // Adjust overall volume if needed
//         mix *= 0.3f;

//         // Output to left and right channels
//         out[i]     = mix;  // Left
//         out[i + 1] = mix;  // Right
//     }
// }

// /*******************************************************/
// /* Function: UpdateLeds                                */
// /* --------------------                                */
// /* Simple function to set LED brightness for knobs and */
// /* show key states. Called from main() in a loop.      */
// /*******************************************************/
// void UpdateLeds()
// {
//     // LED arrays from daisy_field.h:
//     static const size_t knob_leds[8] = {
//         DaisyField::LED_KNOB_1,
//         DaisyField::LED_KNOB_2,
//         DaisyField::LED_KNOB_3,
//         DaisyField::LED_KNOB_4,
//         DaisyField::LED_KNOB_5,
//         DaisyField::LED_KNOB_6,
//         DaisyField::LED_KNOB_7,
//         DaisyField::LED_KNOB_8
//     };

//     // For demonstration, set each knob LED to the knob's value
//     for(size_t i = 0; i < 8; i++)
//     {
//         hw.led_driver.SetLed(knob_leds[i], hw.GetKnobValue(i));
//     }

//     // Set the LED for each of the 16 keys to on/off based on whether it's pressed
//     // The LED definitions for A1..B8 are sequential in daisy_field.h
//     for(size_t i = 0; i < NUM_VOICES; i++)
//     {
//         float brightness = hw.KeyboardState(i) ? 1.0f : 0.0f;
//         hw.led_driver.SetLed(DaisyField::LED_KEY_A1 + i, brightness);
//     }

//     hw.led_driver.SwapBuffersAndTransmit();
// }

// /*******************************************************/
// /* Function: main                                      */
// /* -------------
// /* Entry point: initializes hardware, sets up voices,  */
// /* and enters an infinite loop.                        */
// /*******************************************************/
// int main(void)
// {
//     // 1) Initialize the Daisy Field
//     hw.Init();
//     float samplerate = hw.AudioSampleRate();

//     // 2) Initialize all voices
//     for(int i = 0; i < NUM_VOICES; i++)
//     {
//         voices[i].Init(samplerate);
//     }

//     // 3) Start reading knobs (ADC) and start audio
//     hw.StartAdc();
//     hw.StartAudio(AudioCallback);

//     // 4) Main loop: read & update LED states, or other control
//     while(true)
//     {
//         UpdateLeds();
//         System::Delay(1);
//     }
//     // Execution never really reaches here
//     return 0;
// }


/*          VERSION 2                       */
// #include "daisy_field.h"
// #include "daisysp.h"

// // Use these namespaces for convenience
// using namespace daisy;
// using namespace daisysp;

// // Number of total voices/keys
// #define NUM_VOICES 16

// // Create global DaisyField hardware object
// DaisyField hw;

// /**
//     Voice struct:
//     - Two oscillators (osc1, osc2)
//     - Smooth amplitude gating (amp_)
//     - on_ indicates whether the key is pressed.
// */
// struct Voice
// {
//     void Init(float samplerate)
//     {
//         osc1.Init(samplerate);
//         osc2.Init(samplerate);

//         // Default amplitude
//         osc1.SetAmp(0.3f);
//         osc2.SetAmp(0.3f);

//         // Default waveforms
//         osc1.SetWaveform(Oscillator::WAVE_SIN);
//         osc2.SetWaveform(Oscillator::WAVE_SIN);

//         // Start silent
//         amp_ = 0.0f;
//         on_  = false;
//     }

//     // Processes one sample of audio
//     float Process()
//     {
//         // Smooth ramp from 0->1 if on_, or 1->0 if !on_.
//         amp_ += 0.0025f * ((on_ ? 1.0f : 0.0f) - amp_);
//         // Combine both oscillators, scaled by amp_
//         return (osc1.Process() + osc2.Process()) * 0.5f * amp_;
//     }

//     // Sets same frequency on both oscillators
//     void SetNote(float midi_note)
//     {
//         float freq = mtof(midi_note); // DaisySP function: MIDI note -> Hz
//         osc1.SetFreq(freq);
//         osc2.SetFreq(freq);
//     }

//     // Select waveforms for each oscillator if desired
//     void SetWaveform(bool for_osc1, Oscillator::Waveform wf)
//     {
//         if(for_osc1)
//             osc1.SetWaveform(wf);
//         else
//             osc2.SetWaveform(wf);
//     }

//     // Optionally set different amplitudes for each oscillator
//     void SetAmps(float a1, float a2)
//     {
//         osc1.SetAmp(a1);
//         osc2.SetAmp(a2);
//     }

//     bool  on_;  // key gating
//     float amp_; // smoothed amplitude
//     Oscillator osc1, osc2;
// };

// // Create the array of voices
// Voice voices[NUM_VOICES];

// // Semitone offsets for each key (A1..B8).
// // A1..A8 is a major scale, B row includes some chromatic offsets.
// static float scale[NUM_VOICES] = {
//     0.f,  2.f,  4.f,  5.f,  7.f,  9.f,  11.f, 12.f,
//     0.f,  1.f,  3.f,  0.f,  6.f,  8.f,  10.f, 0.f
// };

// // We have 4 waveforms: Sine, Tri, Polyblep Saw, Polyblep Square
// // static const Oscillator::Waveform WAVEFORMS[] = {
// //     Oscillator::WAVE_SIN,
// //     Oscillator::WAVE_TRI,
// //     Oscillator::WAVE_POLYBLEP_SAW,
// //     Oscillator::WAVE_POLYBLEP_SQUARE
// // };

// uint8_t waveforms[NUM_WAVEFORMS] = {
//     Oscillator::WAVE_SIN,
//     Oscillator::WAVE_TRI,
//     Oscillator::WAVE_POLYBLEP_SAW,
//     Oscillator::WAVE_POLYBLEP_SQUARE,
// };

// // We'll cycle waveforms with the encoder
// static int waveform_idx;

// // We'll shift octaves with SW_1 (down) / SW_2 (up)
// static int octave;

// // We’ll read knob1 as a "MIDI note" range from 0..127
// // Then we do freq = mtof( base_note + scale[i] ),
// // with an added "octave * 12" shift.
// static float base_midi; // Our "knob-based" note


// /*************************************************************
//     AudioCallback
//     -------------
//     - Runs at audio rate (~48kHz).
//     - We read the hardware (knobs, switches, encoder).
//     - We update waveforms and octaves.
//     - We compute note from knob1 => base_midi and offset with (octave*12).
//     - For each voice, set frequency, turn on/off, process audio.
// *************************************************************/
// void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
//                    AudioHandle::InterleavingOutputBuffer out,
//                    size_t                                size)
// {
//     // 1) Process hardware for the Daisy Field:
//     hw.ProcessAnalogControls();
//     hw.ProcessDigitalControls();

//     // 2) Update waveform from encoder movement
//     //    Each encoder "tick" increments or decrements waveform_idx
//     int enc = hw.encoder.Increment();
//     if(enc != 0)
//     {
//         waveform_idx += enc;
//         // clamp between 0 and 3 (4 waveforms)
//         if(waveform_idx < 0)
//             waveform_idx = 0;
//         if(waveform_idx > 3)
//             waveform_idx = 3;
//     }

//     // 3) Update octave from side push buttons
//     //    SW_1 => down, SW_2 => up
//     if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge())
//         octave--;
//     if(hw.GetSwitch(DaisyField::SW_2)->RisingEdge())
//         octave++;
//     // clamp between 0 and 4 (5 possible octaves)
//     if(octave < 0)
//         octave = 0;
//     if(octave > 4)
//         octave = 4;

//     // 4) Read knob 1 as "MIDI note" range from 0..127
//     //    hw.GetKnobValue(0) returns 0..1
//     //    scale it up to 0..127
//     float knob_val = hw.GetKnobValue(0);
//     base_midi      = knob_val * 127.f;

//     // 5) For each of the 16 voices:
//     //    - Check if key is pressed => on_
//     //    - Calculate the actual MIDI note
//     //      (base_midi + octave*12 + per-key scale offset)
//     //    - Set the waveforms, etc.
//     for(int i = 0; i < NUM_VOICES; i++)
//     {
//         voices[i].on_ = hw.KeyboardState(i);

//         float note = base_midi + (octave * 12) + scale[i];
//         voices[i].SetNote(note);

//         // Both oscillators share the same waveform for simplicity.
//         voices[i].SetWaveform(true,  WAVEFORMS[waveform_idx]);
//         voices[i].SetWaveform(false, WAVEFORMS[waveform_idx]);
//     }

//     // 6) Audio loop: Summation of all voices
//     for(size_t i = 0; i < size; i += 2)
//     {
//         float mix = 0.0f;
//         for(int v = 0; v < NUM_VOICES; v++)
//         {
//             mix += voices[v].Process();
//         }
//         // avoid clipping
//         mix *= 0.3f;

//         out[i]     = mix; // Left
//         out[i + 1] = mix; // Right
//     }
// }

// /*************************************************************
//     UpdateLeds
//     ----------
//     - Called at slower rates (e.g. in main loop)
//     - Lights up each key's LED if pressed,
//       sets knob LEDs to reflect their positions.
// *************************************************************/
// void UpdateLeds()
// {
//     // Simple: set each knob LED to the knob's value
//     static const size_t knob_leds[8] = {
//         DaisyField::LED_KNOB_1,
//         DaisyField::LED_KNOB_2,
//         DaisyField::LED_KNOB_3,
//         DaisyField::LED_KNOB_4,
//         DaisyField::LED_KNOB_5,
//         DaisyField::LED_KNOB_6,
//         DaisyField::LED_KNOB_7,
//         DaisyField::LED_KNOB_8
//     };

//     // Update knob LEDs
//     for(int k = 0; k < 8; k++)
//     {
//         float val = hw.GetKnobValue(k);
//         hw.led_driver.SetLed(knob_leds[k], val);
//     }

//     // Light key LED if pressed
//     for(int i = 0; i < NUM_VOICES; i++)
//     {
//         float brightness = hw.KeyboardState(i) ? 1.0f : 0.0f;
//         hw.led_driver.SetLed(DaisyField::LED_KEY_A1 + i, brightness);
//     }

//     // Flush LED data to hardware
//     hw.led_driver.SwapBuffersAndTransmit();
// }

// /*************************************************************
//     main()
//     ------
//     - Initialize hardware
//     - Initialize voices
//     - Start audio + ADC
//     - Loop, calling UpdateLeds
// *************************************************************/
// int main(void)
// {
//     // Init Daisy Field hardware
//     hw.Init();
//     float sr = hw.AudioSampleRate();

//     // Default starting waveform is 0 => WAVE_SIN
//     waveform_idx = 0;
//     // Middle of our range => say octave=2 if you prefer. We'll start at 0 here.
//     octave = 2;

//     // Initialize all voices
//     for(int i = 0; i < NUM_VOICES; i++)
//     {
//         voices[i].Init(sr);
//     }

//     // Start controls + audio
//     hw.StartAdc();
//     hw.StartAudio(AudioCallback);

//     while(1)
//     {
//         UpdateLeds();
//         // a small delay to avoid saturating CPU
//         System::Delay(1);
//     }
// }


/*              ORIGINAL CODE           */

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
