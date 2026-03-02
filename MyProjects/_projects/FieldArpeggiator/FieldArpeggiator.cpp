// Field Arpeggiator for Daisy Field
// Phase 2: Advanced Features Implementation
// Based on Arpeggiator_Project_Plan.md
//
// Phase 1 Features:
// - MIDI note-on capture into ring buffer (8 notes max)
// - Arp modes: Up, Down, UpDown, Random
// - Metro-based clock with BPM knob (40–240 BPM)
// - Gate length parameter
// - OLED: current note, mode, BPM
//
// Phase 2 Features:
// - External MIDI clock sync (24 PPQN)
// - 8-step pattern visualization on OLED
// - Clock source selection (Internal/External)
//
// DAISY FIELD CONTROLS:
// - Knob 1: BPM (40-240) - ignored when external sync
// - Knob 2: Octave Range (1-3)
// - Knob 3: Gate Length (10%-100%)
// - Knob 4: Swing (0-50%)
// - Knob 5-8: ADSR (Attack, Decay, Sustain, Release)
// - Keys A1-A4: Mode select (Up, Down, UpDown, Random)
// - Keys A5: Toggle External/Internal clock
// - Keys B1-B4: Octave shift (-1, 0, +1, +2)
// - SW1: Latch mode toggle
// - SW2: Hold/Pause

#include "daisy_field.h"
#include "daisysp.h"
#include <algorithm>
#include <cstdlib>
#include "field_ux.h"

using namespace daisy;
using namespace daisysp;

// ============================================================================
// CONSTANTS
// ============================================================================
constexpr int MAX_NOTES = 8;
constexpr int NUM_MODES = 4;

enum ArpMode
{
    ARP_UP = 0,
    ARP_DOWN,
    ARP_UP_DOWN,
    ARP_RANDOM
};
const char* MODE_NAMES[] = {"UP", "DOWN", "UP/DN", "RAND"};

// ============================================================================
// ARPEGGIATOR STATE
// ============================================================================
struct ArpState
{
    uint8_t notes[MAX_NOTES];      // Held MIDI notes
    uint8_t velocities[MAX_NOTES]; // Velocities
    int     noteCount;             // Number of held notes
    int     currentIndex;          // Current position in pattern
    bool    ascending;             // Direction for UpDown mode
    int     octaveOffset;          // Current octave in multi-octave mode
};

// ============================================================================
// HARDWARE & DSP
// ============================================================================
DaisyField     hw;
synth::FieldUX ux;

// Sound generation
Oscillator osc;
MoogLadder filter;
Adsr       env;
Metro      arpClock;

// Arpeggiator
ArpState arp;
ArpMode  currentMode = ARP_UP;
int      octaveRange = 1;    // 1-3 octaves
float    gateLength  = 0.5f; // 0.1-1.0
float    swingAmount = 0.0f; // 0-0.5
bool     latchMode   = false;
bool     paused      = false;
int      octaveShift = 0; // -1 to +2

// Timing
float bpm          = 120.0f;
float gateTimer    = 0.0f;
float stepDuration = 0.0f;
bool  gateActive   = false;
bool  swingNext    = false;

// MIDI Clock Sync (Phase 2)
bool     useExternalClock = false;
int      midiClockCounter = 0;
int      midiClockDivider = 6; // 24 PPQN / 6 = quarter notes, /3 = 8ths
float    externalBpm      = 120.0f;
uint32_t lastClockTime    = 0;
uint32_t clockInterval    = 0;
int      stepPosition     = 0; // For 8-step visualization
int      totalStepCount   = 8; // Pattern length for display

// Parameters
float filterCutoff = 2000.0f;
float filterRes    = 0.3f;

// Display-accessible envelope values (updated in AudioCallback)
float displayAttack  = 0.0f;
float displayDecay   = 0.0f;
float displaySustain = 0.0f;
float displayRelease = 0.0f;

// MIDI (using hardware MIDI via TRS jack)
uint8_t lastMidiNote = 0;

// Display
char displayBuffer[64];

// ============================================================================
// NOTE BUFFER MANAGEMENT
// ============================================================================
void AddNote(uint8_t note, uint8_t velocity)
{
    if(arp.noteCount >= MAX_NOTES)
        return;

    // Check if note already exists
    for(int i = 0; i < arp.noteCount; i++)
    {
        if(arp.notes[i] == note)
            return;
    }

    arp.notes[arp.noteCount]      = note;
    arp.velocities[arp.noteCount] = velocity;
    arp.noteCount++;

    // Sort notes for Up/Down modes
    for(int i = 0; i < arp.noteCount - 1; i++)
    {
        for(int j = i + 1; j < arp.noteCount; j++)
        {
            if(arp.notes[i] > arp.notes[j])
            {
                std::swap(arp.notes[i], arp.notes[j]);
                std::swap(arp.velocities[i], arp.velocities[j]);
            }
        }
    }
}

void RemoveNote(uint8_t note)
{
    if(latchMode)
        return; // Don't remove in latch mode

    for(int i = 0; i < arp.noteCount; i++)
    {
        if(arp.notes[i] == note)
        {
            // Shift remaining notes
            for(int j = i; j < arp.noteCount - 1; j++)
            {
                arp.notes[j]      = arp.notes[j + 1];
                arp.velocities[j] = arp.velocities[j + 1];
            }
            arp.noteCount--;
            break;
        }
    }

    // Reset index if needed
    if(arp.currentIndex >= arp.noteCount)
    {
        arp.currentIndex = 0;
    }
}

void ClearNotes()
{
    arp.noteCount    = 0;
    arp.currentIndex = 0;
    arp.octaveOffset = 0;
    arp.ascending    = true;
}

// ============================================================================
// ARPEGGIATOR LOGIC
// ============================================================================
uint8_t GetNextNote()
{
    if(arp.noteCount == 0)
        return 0;

    uint8_t note       = 0;
    int     totalSteps = arp.noteCount * octaveRange;

    switch(currentMode)
    {
        case ARP_UP:
            note = arp.notes[arp.currentIndex % arp.noteCount];
            note += (arp.currentIndex / arp.noteCount) * 12; // Octave
            arp.currentIndex = (arp.currentIndex + 1) % totalSteps;
            break;

        case ARP_DOWN:
        {
            int revIndex = totalSteps - 1 - arp.currentIndex;
            note         = arp.notes[revIndex % arp.noteCount];
            note += (revIndex / arp.noteCount) * 12;
            arp.currentIndex = (arp.currentIndex + 1) % totalSteps;
        }
        break;

        case ARP_UP_DOWN:
        {
            int idx = arp.currentIndex % arp.noteCount;
            int oct = arp.octaveOffset;
            note = arp.notes[arp.ascending ? idx : (arp.noteCount - 1 - idx)];
            note += oct * 12;

            arp.currentIndex++;
            if(arp.currentIndex >= arp.noteCount)
            {
                arp.currentIndex = 0;
                if(arp.ascending)
                {
                    arp.octaveOffset++;
                    if(arp.octaveOffset >= octaveRange)
                    {
                        arp.octaveOffset = octaveRange - 1;
                        arp.ascending    = false;
                    }
                }
                else
                {
                    arp.octaveOffset--;
                    if(arp.octaveOffset < 0)
                    {
                        arp.octaveOffset = 0;
                        arp.ascending    = true;
                    }
                }
            }
        }
        break;

        case ARP_RANDOM:
        {
            int randIdx = rand() % arp.noteCount;
            int randOct = rand() % octaveRange;
            note        = arp.notes[randIdx] + randOct * 12;
        }
        break;
    }

    return note + (octaveShift * 12);
}

void TriggerArpNote()
{
    if(arp.noteCount == 0 || paused)
        return;

    uint8_t note = GetNextNote();
    if(note == 0)
        return;

    // Update step position for visualization
    stepPosition = (stepPosition + 1) % totalStepCount;

    // Set oscillator frequency
    float freq = mtof(note);
    osc.SetFreq(freq);

    // Trigger envelope
    env.Retrigger(false);

    // Calculate gate duration with swing
    float stepTime = 60.0f / bpm / 2.0f; // 8th notes
    if(swingNext && swingAmount > 0.0f)
    {
        stepTime *= (1.0f + swingAmount);
    }
    else if(!swingNext && swingAmount > 0.0f)
    {
        stepTime *= (1.0f - swingAmount * 0.5f);
    }
    swingNext = !swingNext;

    stepDuration = stepTime * gateLength;
    gateTimer    = 0.0f;
    gateActive   = true;

    // MIDI output (via hardware MIDI)
    if(lastMidiNote > 0)
    {
        uint8_t noteOff[3] = {0x80, lastMidiNote, 0};
        hw.midi.SendMessage(noteOff, 3);
    }
    uint8_t noteOn[3] = {0x90, note, 100};
    hw.midi.SendMessage(noteOn, 3);
    lastMidiNote = note;
}

// ============================================================================
// DISPLAY
// ============================================================================
void UpdateDisplay()
{
    hw.display.Fill(false);

    // Line 1: Mode, BPM, Octave Range, Octave Shift
    float displayBpm = useExternalClock ? externalBpm : bpm;
    snprintf(displayBuffer,
             64,
             "%s %3.0f%s O:%d S:%+d",
             MODE_NAMES[currentMode],
             displayBpm,
             useExternalClock ? "E" : "I",
             octaveRange,
             octaveShift);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(displayBuffer, Font_6x8, true);

    // Line 2: Gate Length, Swing, Latch/Pause status
    snprintf(displayBuffer,
             64,
             "G:%2.0f%% Sw:%2.0f%% %s%s",
             gateLength * 100.0f,
             swingAmount * 100.0f,
             latchMode ? "L" : "",
             paused ? " ||" : "");
    hw.display.SetCursor(0, 10);
    hw.display.WriteString(displayBuffer, Font_6x8, true);

    // Line 3: ADSR values (from cached global variables)
    snprintf(displayBuffer,
             64,
             "A:%.0f D:%.0f S:%.0f R:%.0f",
             displayAttack,   // Already in ms
             displayDecay,    // Already in ms
             displaySustain,  // Already in %
             displayRelease); // Already in ms
    hw.display.SetCursor(0, 20);
    hw.display.WriteString(displayBuffer, Font_6x8, true);

    // Line 4: Notes or step visualization
    if(arp.noteCount > 0)
    {
        // Show 8-step pattern visualization
        int barY      = 30;
        int barWidth  = 15;
        int barHeight = 5;
        int spacing   = 1;

        for(int i = 0; i < 8; i++)
        {
            int  x         = i * (barWidth + spacing);
            bool isCurrent = (i == stepPosition % 8);

            // Draw box
            hw.display.DrawRect(
                x, barY, x + barWidth - 1, barY + barHeight, true, false);

            // Fill if current step AND gate active
            if(isCurrent && gateActive)
            {
                hw.display.DrawRect(x + 1,
                                    barY + 1,
                                    x + barWidth - 2,
                                    barY + barHeight - 1,
                                    true,
                                    true);
            }
            else if(isCurrent)
            {
                hw.display.DrawLine(x + barWidth / 2,
                                    barY + 1,
                                    x + barWidth / 2,
                                    barY + barHeight - 1,
                                    true);
            }
        }
    }
    else
    {
        hw.display.SetCursor(0, 30);
        hw.display.WriteString("Play MIDI keys...", Font_6x8, true);
    }

    hw.display.Update();
}

// ============================================================================
// LED FEEDBACK
// ============================================================================
void UpdateLEDs()
{
    // Note activity LEDs brightness
    float note_leds[4];
    for(int i = 0; i < 4; i++)
    {
        note_leds[i] = (i < arp.noteCount) ? 0.5f : 0.0f;
        if(i < arp.noteCount && gateActive
           && arp.notes[i]
                  == (lastMidiNote % 12 + (lastMidiNote / 12) * 12
                      - octaveShift * 12))
        {
            note_leds[i] = 1.0f;
        }
    }

    // Knob values for display
    float k_disp[8];
    for(int i = 0; i < 8; i++)
        k_disp[i] = hw.GetKnobValue(i);

    ux.UpdateLeds(
        currentMode, octaveShift, note_leds, latchMode, paused, k_disp);
}

// ============================================================================
// MIDI HANDLER
// ============================================================================
void HandleMidiMessage(MidiEvent m)
{
    switch(m.type)
    {
        case NoteOn:
            if(m.data[1] > 0)
            {
                AddNote(m.data[0], m.data[1]);
            }
            else
            {
                RemoveNote(m.data[0]);
            }
            break;
        case NoteOff: RemoveNote(m.data[0]); break;

        // Phase 2: MIDI Clock sync
        case SystemRealTime:
            switch(m.srt_type)
            {
                case TimingClock: // 0xF8 - 24 PPQN
                    if(useExternalClock)
                    {
                        // Calculate BPM from clock interval
                        uint32_t now = System::GetUs();
                        if(lastClockTime > 0)
                        {
                            clockInterval = now - lastClockTime;
                            // 24 PPQN, so quarter note = 24 clocks
                            // BPM = 60,000,000 us / (interval * 24)
                            if(clockInterval > 0)
                            {
                                externalBpm
                                    = 60000000.0f / (clockInterval * 24.0f);
                            }
                        }
                        lastClockTime = now;

                        midiClockCounter++;
                        if(midiClockCounter >= midiClockDivider)
                        {
                            midiClockCounter = 0;
                            TriggerArpNote();
                        }
                    }
                    break;
                case Start: // 0xFA
                    paused           = false;
                    stepPosition     = 0;
                    arp.currentIndex = 0;
                    midiClockCounter = 0;
                    break;
                case Stop: // 0xFC
                    paused = true;
                    break;
                case Continue: // 0xFB
                    paused = false;
                    break;
                default: break;
            }
            break;

        default: break;
    }
}

// ============================================================================
// AUDIO CALLBACK
// ============================================================================
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    hw.ProcessAnalogControls();
    hw.ProcessDigitalControls();

    // Process knobs via UX
    float k[8];
    ux.ProcessKnobs(k);

    // Read knobs
    bpm         = 40.0f + k[0] * 200.0f;              // 40-240 BPM
    octaveRange = 1 + static_cast<int>(k[1] * 2.99f); // 1-3
    gateLength  = 0.1f + k[2] * 0.9f;                 // 10-100%
    swingAmount = k[3] * 0.5f;                        // 0-50%

    // ADSR from knobs 5-8
    float attack  = 0.005f + k[4] * 0.5f; // Min 5ms to prevent clicks
    float decay   = 0.01f + k[5] * 0.5f;
    float sustain = k[6];
    float release = 0.01f + k[7] * 1.0f;
    env.SetAttackTime(attack);
    env.SetDecayTime(decay);
    env.SetSustainLevel(sustain);
    env.SetReleaseTime(release);

    // Store for display (in ms for A/D/R, % for S)
    displayAttack  = attack * 1000.0f;
    displayDecay   = decay * 1000.0f;
    displaySustain = sustain * 100.0f;
    displayRelease = release * 1000.0f;

    // Filter from sustain knob (repurpose)
    filterCutoff = 200.0f + sustain * 4800.0f;
    filter.SetFreq(filterCutoff);

    // Update clock rate
    arpClock.SetFreq(bpm / 60.0f * 2.0f); // 8th notes

    // Keys A1-A4: Mode select
    for(int i = 0; i < 4; i++)
    {
        if(hw.KeyboardRisingEdge(i + 8))
        { // A-row
            currentMode      = static_cast<ArpMode>(i);
            arp.currentIndex = 0;
            arp.ascending    = true;
            arp.octaveOffset = 0;
        }
    }

    // Key A5: Toggle Internal/External clock (Phase 2)
    if(hw.KeyboardRisingEdge(12)) // A5 is key 12
    {
        useExternalClock = !useExternalClock;
        midiClockCounter = 0;
        lastClockTime    = 0;
    }

    // Keys B1-B4: Octave shift
    for(int i = 0; i < 4; i++)
    {
        if(hw.KeyboardRisingEdge(i))
        {                        // B-row
            octaveShift = i - 1; // -1, 0, +1, +2
        }
    }

    // SW1: Latch toggle
    if(hw.sw[0].RisingEdge())
    {
        latchMode = !latchMode;
        if(!latchMode)
        {
            ClearNotes();
        }
    }

    // SW2: Pause toggle
    if(hw.sw[1].RisingEdge())
    {
        paused = !paused;
    }

    // Audio processing
    float sampleRate = hw.AudioSampleRate();

    for(size_t i = 0; i < size; i++)
    {
        // Arp clock
        if(arpClock.Process() && !paused)
        {
            TriggerArpNote();
        }

        // Gate timing
        if(gateActive)
        {
            gateTimer += 1.0f / sampleRate;
            if(gateTimer >= stepDuration)
            {
                gateActive = false;
                env.Retrigger(true); // Release
            }
        }

        // Envelope
        float envVal = env.Process(gateActive);

        // Oscillator
        float sig = osc.Process();

        // Filter
        sig = filter.Process(sig);

        // VCA
        sig *= envVal * 0.5f;

        // Output
        out[0][i] = sig;
        out[1][i] = sig;
    }
}

// ============================================================================
// MAIN
// ============================================================================
int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48);
    float sr = hw.AudioSampleRate();

    // Initialize DSP
    osc.Init(sr);
    osc.SetWaveform(Oscillator::WAVE_SAW);
    osc.SetAmp(0.8f);

    filter.Init(sr);
    filter.SetFreq(2000.0f);
    filter.SetRes(0.3f);

    env.Init(sr);
    env.SetAttackTime(0.01f);
    env.SetDecayTime(0.1f);
    env.SetSustainLevel(0.7f);
    env.SetReleaseTime(0.1f);

    arpClock.Init(4.0f, sr); // Default 120 BPM = 4 Hz for 8th notes
    ux.Init(&hw);

    // Initialize arpeggiator state
    ClearNotes();

    // Initialize hardware MIDI (TRS jack)
    hw.midi.StartReceive();

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1)
    {
        // Handle MIDI (hardware TRS jack)
        hw.midi.Listen();
        while(hw.midi.HasEvents())
        {
            HandleMidiMessage(hw.midi.PopEvent());
        }

        UpdateDisplay();
        UpdateLEDs();
        System::Delay(33); // ~30 FPS
    }
}
