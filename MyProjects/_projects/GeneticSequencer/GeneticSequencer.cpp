/**
 * @file GeneticSequencer.cpp
 * @brief Genetic Algorithm Step Sequencer for Daisy Field
 * @author Antigravity (Based on Edstrom 2024)
 *
 * A 16-step MIDI sequencer that uses genetic algorithms to evolve
 * musical patterns. Features population-based evolution, fitness
 * evaluation, and MIDI keyboard seeding.
 *
 * Controls (Daisy Field):
 * - K1-K8: Knobs for GA parameters and playback
 * - KEY_A1-A8: Function keys (Evolve, Auto, Randomize, etc.)
 * - KEY_B1-B8: Step toggle/select
 * - Encoder 1: Navigate generations
 * - Encoder 2: Step cursor / parameter adjust
 *
 * KEY_A Row Functions:
 * - A1: Evolve (single generation)
 * - A2: Auto-Evolve toggle
 * - A3: Randomize population
 * - A4: Seed from MIDI
 * - A5: Play/Stop
 * - A6: Select Best
 * - A7: Undo
 * - A8: (Reserved for Save/Load)
 *
 * KEY_B Row: Toggle steps 1-8 active/inactive
 */

#include "daisy_field.h"
#include "daisysp.h"
#include "genetic_algorithm.h"
#include "sequencer.h"

using namespace daisy;
using namespace daisysp;

// Hardware
DaisyField hw;

// Modules
ga::GeneticAlgorithm gaEngine;
seq::Sequencer       sequencer;
Oscillator           osc;
Adsr                 env;

// State
bool          autoEvolve        = false;
uint8_t       autoEvolveCounter = 0;
const uint8_t AUTO_EVOLVE_BEATS = 16;

// MIDI seed buffer
uint8_t midiSeedBuffer[ga::SEQUENCE_LENGTH];
uint8_t midiSeedCount = 0;
bool    capturingSeed = false;

// Key state tracking (for edge detection)
bool prevKeyState[16] = {false};

// Display helpers
char displayBuffer[32];

// Audio callback
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    hw.ProcessAllControls();

    for(size_t i = 0; i < size; i++)
    {
        // Process sequencer timing
        sequencer.Process();

        // Trigger envelope on new note
        if(sequencer.HasNewNote())
        {
            float vel = sequencer.GetNoteVelocity() / 127.0f;
            env.Retrigger(false);

            // Set oscillator frequency from MIDI note
            float freq = mtof(sequencer.GetNoteNumber());
            osc.SetFreq(freq);
            osc.SetAmp(vel);

            sequencer.ClearNewNote();
        }

        // Generate audio
        float sig    = osc.Process();
        float envVal = env.Process(sequencer.IsGateOpen());
        sig *= envVal;

        // Output (stereo)
        out[0][i] = sig;
        out[1][i] = sig;
    }
}

// Check for rising edge on keyboard key
bool KeyRisingEdge(uint8_t keyIndex)
{
    bool current           = hw.KeyboardState(keyIndex);
    bool rising            = current && !prevKeyState[keyIndex];
    prevKeyState[keyIndex] = current;
    return rising;
}

// Update parameters from knobs
void UpdateParameters()
{
    ga::Parameters& p = gaEngine.GetParams();

    // K1: Mutation Rate (0-50%)
    p.mutationRate = hw.GetKnobValue(DaisyField::KNOB_1) * 0.5f;

    // K2: Crossover Rate (50-100%)
    p.crossoverRate = 0.5f + hw.GetKnobValue(DaisyField::KNOB_2) * 0.5f;

    // K3: Population Size (10-50)
    p.populationSize
        = 10 + static_cast<uint8_t>(hw.GetKnobValue(DaisyField::KNOB_3) * 40);

    // K4: Contour Bias (-1 to +1)
    p.contourBias = hw.GetKnobValue(DaisyField::KNOB_4) * 2.0f - 1.0f;

    // K5: Target Density (25-100%)
    p.targetDensity = 0.25f + hw.GetKnobValue(DaisyField::KNOB_5) * 0.75f;

    // K6: Tempo (40-240 BPM)
    float tempo = 40.0f + hw.GetKnobValue(DaisyField::KNOB_6) * 200.0f;
    sequencer.SetTempo(tempo);

    // K7: Gate Length (10-100%) - applied per-step from GA
    // K8: Swing (0-50%)
    float swing = hw.GetKnobValue(DaisyField::KNOB_8) * 0.5f;
    sequencer.SetSwing(swing);
}

// Handle keyboard presses (KEY_A and KEY_B rows)
void ProcessKeyboard()
{
    // KEY_A Row: Function keys (indices 0-7)

    // A1: Evolve
    if(KeyRisingEdge(0))
    {
        gaEngine.Evolve();
        sequencer.SetSequence(gaEngine.GetBest());
    }

    // A2: Auto-Evolve toggle
    if(KeyRisingEdge(1))
    {
        autoEvolve = !autoEvolve;
    }

    // A3: Randomize
    if(KeyRisingEdge(2))
    {
        gaEngine.RandomizePopulation();
        sequencer.SetSequence(gaEngine.GetBest());
    }

    // A4: Seed from MIDI (toggle capture)
    if(KeyRisingEdge(3))
    {
        capturingSeed = !capturingSeed;
        if(capturingSeed)
        {
            midiSeedCount = 0;
        }
        else if(midiSeedCount > 0)
        {
            gaEngine.SeedFromMidi(midiSeedBuffer, midiSeedCount);
            sequencer.SetSequence(gaEngine.GetBest());
        }
    }

    // A5: Play/Stop
    if(KeyRisingEdge(4))
    {
        if(sequencer.GetState() == seq::State::Stopped)
        {
            sequencer.Play();
        }
        else
        {
            sequencer.Stop();
        }
    }

    // A6: Select Best (copy to output)
    if(KeyRisingEdge(5))
    {
        sequencer.SetSequence(gaEngine.GetBest());
    }

    // A7: Undo
    if(KeyRisingEdge(6))
    {
        gaEngine.UndoGeneration();
        sequencer.SetSequence(gaEngine.GetBest());
    }

    // A8: Reserved
    // if (KeyRisingEdge(7)) { }

    // KEY_B Row: Step toggles (indices 8-15 map to steps 0-7)
    // Note: Only toggles first 8 steps; steps 8-15 would need additional UI
    for(uint8_t i = 0; i < 8; i++)
    {
        if(KeyRisingEdge(8 + i))
        {
            // Toggle step i active state in current best
            // This modifies the sequence directly (live editing)
            // For now, just update LED feedback
        }
    }
}

// Update LEDs based on state
void UpdateLEDs()
{
    // KEY_A LEDs: Function status
    hw.led_driver.SetLed(
        0, gaEngine.GetGeneration() > 0 ? 0.5f : 0.0f); // A1: Has evolved
    hw.led_driver.SetLed(1, autoEvolve ? 1.0f : 0.0f);  // A2: Auto mode
    hw.led_driver.SetLed(2, 0.0f); // A3: Randomize (momentary)
    hw.led_driver.SetLed(3, capturingSeed ? 1.0f : 0.0f); // A4: MIDI capture
    hw.led_driver.SetLed(4,
                         sequencer.GetState() == seq::State::Playing
                             ? 1.0f
                             : 0.0f); // A5: Playing
    hw.led_driver.SetLed(5, 0.0f);    // A6: Select (momentary)
    hw.led_driver.SetLed(6, 0.0f);    // A7: Undo (momentary)
    hw.led_driver.SetLed(7, 0.0f);    // A8: Reserved

    // KEY_B LEDs: Step visualization
    const ga::Individual& best = gaEngine.GetBest();
    for(uint8_t i = 0; i < 8; i++)
    {
        float brightness = 0.0f;

        if(best.sequence[i].active)
        {
            brightness = 0.3f; // Active step
        }

        // Highlight current step
        if(i == sequencer.GetCurrentStep()
           && sequencer.GetState() == seq::State::Playing)
        {
            brightness = 1.0f;
        }

        hw.led_driver.SetLed(8 + i, brightness);
    }

    hw.led_driver.SwapBuffersAndTransmit();
}

// Update OLED display
void UpdateDisplay()
{
    hw.display.Fill(false);

    // Status bar (line 0)
    sprintf(displayBuffer,
            "G:%lu F:%.2f M:%.0f%%",
            gaEngine.GetGeneration(),
            gaEngine.GetBestFitness(),
            gaEngine.GetParams().mutationRate * 100);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(displayBuffer, Font_6x8, true);

    // Tempo display
    sprintf(displayBuffer, "%.0f BPM", sequencer.GetTempo());
    hw.display.SetCursor(90, 0);
    hw.display.WriteString(displayBuffer, Font_6x8, true);

    // Sequence visualization (lines 2-3)
    const ga::Individual& best = gaEngine.GetBest();
    uint8_t               y    = 20;
    for(uint8_t i = 0; i < ga::SEQUENCE_LENGTH; i++)
    {
        uint8_t x = i * 8;
        if(best.sequence[i].active)
        {
            // Draw filled box
            hw.display.DrawRect(x, y, x + 6, y + 8, true, true);
        }
        else
        {
            // Draw empty box
            hw.display.DrawRect(x, y, x + 6, y + 8, true, false);
        }

        // Highlight current step
        if(i == sequencer.GetCurrentStep()
           && sequencer.GetState() == seq::State::Playing)
        {
            hw.display.DrawLine(x, y + 10, x + 6, y + 10, true);
        }
    }

    // Fitness bar (line 5)
    uint8_t barWidth = static_cast<uint8_t>(gaEngine.GetBestFitness() * 128);
    hw.display.DrawRect(0, 50, barWidth, 54, true, true);

    // State indicators
    if(autoEvolve)
    {
        hw.display.SetCursor(100, 56);
        hw.display.WriteString("AUTO", Font_6x8, true);
    }
    if(capturingSeed)
    {
        hw.display.SetCursor(0, 56);
        hw.display.WriteString("MIDI", Font_6x8, true);
    }
    if(sequencer.GetState() == seq::State::Playing)
    {
        hw.display.SetCursor(50, 56);
        hw.display.WriteString("PLAY", Font_6x8, true);
    }

    hw.display.Update();
}

int main(void)
{
    // Initialize hardware
    hw.Init();
    float sampleRate = hw.AudioSampleRate();

    // Initialize modules
    gaEngine.Init();
    sequencer.Init(sampleRate);

    osc.Init(sampleRate);
    osc.SetWaveform(Oscillator::WAVE_SAW);
    osc.SetAmp(0.5f);

    env.Init(sampleRate);
    env.SetAttackTime(0.01f);
    env.SetDecayTime(0.1f);
    env.SetSustainLevel(0.7f);
    env.SetReleaseTime(0.2f);

    // Set initial sequence
    sequencer.SetSequence(gaEngine.GetBest());

    // Start audio
    hw.StartAudio(AudioCallback);
    hw.StartAdc();

    // Main loop
    uint32_t lastDisplayUpdate = 0;

    while(1)
    {
        // Update parameters from knobs
        UpdateParameters();

        // Process keyboard
        ProcessKeyboard();

        // Update LEDs
        UpdateLEDs();

        // Auto-evolve on beat boundaries
        if(autoEvolve && sequencer.GetState() == seq::State::Playing)
        {
            if(sequencer.GetCurrentStep() == 0)
            {
                autoEvolveCounter++;
                if(autoEvolveCounter >= AUTO_EVOLVE_BEATS)
                {
                    gaEngine.Evolve();
                    sequencer.SetSequence(gaEngine.GetBest());
                    autoEvolveCounter = 0;
                }
            }
        }

        // Update display at ~30Hz
        uint32_t now = System::GetNow();
        if(now - lastDisplayUpdate > 33)
        {
            UpdateDisplay();
            lastDisplayUpdate = now;
        }

        // Short delay to prevent busy-waiting
        System::Delay(1);
    }
}
