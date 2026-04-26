#include "daisy_field.h"
#include "daisyhost/apps/SubharmoniqCore.h"
#include "daisysp.h"

#include <array>
#include <cmath>
#include <cstdio>

using namespace daisy;
using namespace daisyhost;
using namespace daisyhost::apps;

namespace
{
DaisyField      hw;
SubharmoniqCore core("node0");

constexpr size_t kAudioBlockSize = 48;
constexpr float  kTouchEpsilon   = 0.015f;
constexpr uint32_t kHoldMs       = 500;
constexpr size_t kMaxMidiEventsPerTick = 16;

enum class FieldKeyRow
{
    kA,
    kB,
};

constexpr size_t kKnobLeds[8] = {
    DaisyField::LED_KNOB_1,
    DaisyField::LED_KNOB_2,
    DaisyField::LED_KNOB_3,
    DaisyField::LED_KNOB_4,
    DaisyField::LED_KNOB_5,
    DaisyField::LED_KNOB_6,
    DaisyField::LED_KNOB_7,
    DaisyField::LED_KNOB_8,
};

constexpr size_t kKeyLeds[16] = {
    DaisyField::LED_KEY_A1,
    DaisyField::LED_KEY_A2,
    DaisyField::LED_KEY_A3,
    DaisyField::LED_KEY_A4,
    DaisyField::LED_KEY_A5,
    DaisyField::LED_KEY_A6,
    DaisyField::LED_KEY_A7,
    DaisyField::LED_KEY_A8,
    DaisyField::LED_KEY_B1,
    DaisyField::LED_KEY_B2,
    DaisyField::LED_KEY_B3,
    DaisyField::LED_KEY_B4,
    DaisyField::LED_KEY_B5,
    DaisyField::LED_KEY_B6,
    DaisyField::LED_KEY_B7,
    DaisyField::LED_KEY_B8,
};

std::array<float, 8> knobValues{};
std::array<float, 8> previousKnobValues{};
std::array<bool, 8>  knobInitialized{};
std::array<int, 4>   rhythmTargets{{1, 2, 3, 0}};
float                cvValues[4] = {};
bool                 sw1HeldConsumed = false;
uint32_t             sw1PressMs = 0;
int                  selectedSeqStep[2] = {0, 0};

float Clamp01(float value)
{
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

bool HasMeaningfulChange(float value, float previous, bool initialized)
{
    return initialized && fabsf(value - previous) > kTouchEpsilon;
}

size_t KeyboardIndexForPhysicalKey(FieldKeyRow row, size_t oneBasedKey)
{
    if(oneBasedKey == 0 || oneBasedKey > 8)
    {
        return 0;
    }

    const size_t keyOffset = oneBasedKey - 1;
    // libDaisy's scan index is row-reversed on the Field hardware labels.
    return (row == FieldKeyRow::kA ? 8 : 0) + keyOffset;
}

bool PhysicalKeyRisingEdge(FieldKeyRow row, size_t oneBasedKey)
{
    return hw.KeyboardRisingEdge(KeyboardIndexForPhysicalKey(row, oneBasedKey));
}

uint16_t ToDacCode(float normalized)
{
    return static_cast<uint16_t>(Clamp01(normalized) * 4095.0f);
}

DaisySubharmoniqRhythmTarget NextRhythmTarget(DaisySubharmoniqRhythmTarget target)
{
    switch(target)
    {
        case DaisySubharmoniqRhythmTarget::kOff:
            return DaisySubharmoniqRhythmTarget::kSeq1;
        case DaisySubharmoniqRhythmTarget::kSeq1:
            return DaisySubharmoniqRhythmTarget::kSeq2;
        case DaisySubharmoniqRhythmTarget::kSeq2:
            return DaisySubharmoniqRhythmTarget::kBoth;
        default:
            return DaisySubharmoniqRhythmTarget::kOff;
    }
}

const char* PageLabel(DaisySubharmoniqPage page)
{
    switch(page)
    {
        case DaisySubharmoniqPage::kVoice: return "Voice";
        case DaisySubharmoniqPage::kMix: return "Mix";
        case DaisySubharmoniqPage::kSeq: return "Seq";
        case DaisySubharmoniqPage::kRhythm: return "Rhythm";
        case DaisySubharmoniqPage::kFilter: return "Filter";
        case DaisySubharmoniqPage::kPatch: return "Patch";
        case DaisySubharmoniqPage::kMidi: return "MIDI";
        case DaisySubharmoniqPage::kAbout: return "About";
        default: return "Home";
    }
}

const char* ParameterIdForPageKnob(DaisySubharmoniqPage page, size_t knob)
{
    constexpr std::array<const char*, 8> kHomeParams = {
        "tempo", "cutoff", "resonance", "vca_decay",
        "vco1_pitch", "vco2_pitch", "drive", "output"};
    constexpr std::array<const char*, 8> kVoiceParams = {
        "vco1_pitch", "vco2_pitch", "vco1_sub1_div", "vco2_sub1_div",
        "vco1_sub2_div", "vco2_sub2_div", "", ""};
    constexpr std::array<const char*, 8> kMixParams = {
        "vco1_level", "vco1_sub1_level", "vco1_sub2_level", "vco2_level",
        "vco2_sub1_level", "vco2_sub2_level", "drive", "output"};
    constexpr std::array<const char*, 8> kSeqParams = {
        "seq1_step1", "seq1_step2", "seq1_step3", "seq1_step4",
        "seq2_step1", "seq2_step2", "seq2_step3", "seq2_step4"};
    constexpr std::array<const char*, 8> kRhythmParams = {
        "rhythm1_div", "rhythm2_div", "rhythm3_div", "rhythm4_div",
        "", "", "", ""};
    constexpr std::array<const char*, 8> kFilterParams = {
        "cutoff", "resonance", "vcf_env_amt", "vca_decay",
        "vcf_attack", "vcf_decay", "drive", "output"};
    constexpr std::array<const char*, 8> kPatchParams = {
        "root_cv", "cutoff_cv", "rhythm_cv", "sub_cv", "", "", "", ""};

    if(knob >= 8)
    {
        return "";
    }

    switch(page)
    {
        case DaisySubharmoniqPage::kVoice: return kVoiceParams[knob];
        case DaisySubharmoniqPage::kMix: return kMixParams[knob];
        case DaisySubharmoniqPage::kSeq: return kSeqParams[knob];
        case DaisySubharmoniqPage::kRhythm: return kRhythmParams[knob];
        case DaisySubharmoniqPage::kFilter: return kFilterParams[knob];
        case DaisySubharmoniqPage::kPatch: return kPatchParams[knob];
        default: return kHomeParams[knob];
    }
}

void AudioCallback(AudioHandle::InputBuffer,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    float* outputPtrs[2] = {out[0], out[1]};
    core.ProcessAudio(outputPtrs[0], outputPtrs[1], size);
}

void WriteDisplayLine(int y, const char* text)
{
    hw.display.SetCursor(0, y);
    hw.display.WriteString(text, Font_6x8, true);
}

void UpdateDisplay()
{
    char line[32];
    const DaisySubharmoniqPage page = core.GetActivePage();

    hw.display.Fill(false);

    std::snprintf(line, sizeof(line), "Subharmoniq %s", PageLabel(page));
    WriteDisplayLine(0, line);
    WriteDisplayLine(10, core.IsPlaying() ? "Play" : "Stop");
    std::snprintf(line,
                  sizeof(line),
                  "S1 %d S2 %d",
                  core.GetSequencerStepIndex(0) + 1,
                  core.GetSequencerStepIndex(1) + 1);
    WriteDisplayLine(20, line);

    if(page == DaisySubharmoniqPage::kFilter)
    {
        WriteDisplayLine(52, "R2: SVF/BPF Ladder");
    }
    else
    {
        WriteDisplayLine(52, "SW1<- SW2-> hold+tap OK");
    }
    hw.display.Update();
}

void UpdateLeds()
{
    for(size_t i = 0; i < 8; ++i)
    {
        hw.led_driver.SetLed(kKnobLeds[i], knobValues[i]);
    }

    for(size_t i = 0; i < 16; ++i)
    {
        float value = 0.02f;
        if(i < 4 && selectedSeqStep[0] == static_cast<int>(i))
        {
            value = 0.65f;
        }
        else if(i >= 8 && i < 12 && selectedSeqStep[1] == static_cast<int>(i - 8))
        {
            value = 0.65f;
        }
        else if(i >= 4 && i < 8)
        {
            value = 0.12f + 0.18f * static_cast<float>(rhythmTargets[i - 4]);
        }
        hw.led_driver.SetLed(kKeyLeds[i], value);
    }

    hw.led_driver.SetLed(DaisyField::LED_SW_1, sw1HeldConsumed ? 0.8f : 0.08f);
    hw.led_driver.SetLed(DaisyField::LED_SW_2, core.IsPlaying() ? 0.8f : 0.08f);
    hw.led_driver.SwapBuffersAndTransmit();
}

void ProcessKnobs()
{
    const DaisySubharmoniqPage page = core.GetActivePage();
    for(size_t i = 0; i < 8; ++i)
    {
        knobValues[i] = hw.GetKnobValue(i);
        if(!knobInitialized[i])
        {
            previousKnobValues[i] = knobValues[i];
            knobInitialized[i]    = true;
            continue;
        }
        const char* parameterId = ParameterIdForPageKnob(page, i);
        if(parameterId[0] == '\0')
        {
            continue;
        }
        if(HasMeaningfulChange(knobValues[i], previousKnobValues[i], knobInitialized[i]))
        {
            previousKnobValues[i] = knobValues[i];
            knobInitialized[i] = true;
            core.SetParameterValue(parameterId, knobValues[i]);
        }
    }
}

void ProcessSwitchMenu()
{
    Switch* sw1 = hw.GetSwitch(DaisyField::SW_1);
    Switch* sw2 = hw.GetSwitch(DaisyField::SW_2);

    if(sw1->RisingEdge())
    {
        sw1PressMs = System::GetNow();
        sw1HeldConsumed = false;
    }

    const bool sw1Held = sw1->Pressed()
                         && (System::GetNow() - sw1PressMs) >= kHoldMs;
    if(sw1Held && sw2->RisingEdge())
    {
        core.SetEncoderPress(true);
        core.SetEncoderPress(false);
        sw1HeldConsumed = true;
        return;
    }

    if(sw1->FallingEdge() && !sw1HeldConsumed)
    {
        core.MenuRotate(-1);
    }
    if(sw2->RisingEdge() && !sw1->Pressed())
    {
        core.MenuRotate(1);
    }
}

void ProcessKeys()
{
    for(size_t i = 0; i < 4; ++i)
    {
        if(PhysicalKeyRisingEdge(FieldKeyRow::kA, i + 1))
        {
            selectedSeqStep[0] = static_cast<int>(i);
            core.SetActivePage(DaisySubharmoniqPage::kSeq);
        }
        if(PhysicalKeyRisingEdge(FieldKeyRow::kB, i + 1))
        {
            selectedSeqStep[1] = static_cast<int>(i);
            core.SetActivePage(DaisySubharmoniqPage::kSeq);
        }
    }

    for(size_t i = 0; i < 4; ++i)
    {
        if(PhysicalKeyRisingEdge(FieldKeyRow::kA, i + 5))
        {
            const auto next = NextRhythmTarget(core.GetRhythmTarget(i));
            core.SetRhythmTarget(i, next);
            rhythmTargets[i] = static_cast<int>(next);
            core.SetActivePage(DaisySubharmoniqPage::kRhythm);
        }
    }

    if(PhysicalKeyRisingEdge(FieldKeyRow::kB, 5))
    {
        const int next = (static_cast<int>(core.GetQuantizeMode()) + 1) % 5;
        core.SetQuantizeMode(static_cast<DaisySubharmoniqQuantizeMode>(next));
    }
    if(PhysicalKeyRisingEdge(FieldKeyRow::kB, 6))
    {
        const int current = core.GetSeqOctaveRange();
        core.SetSeqOctaveRange(current == 1 ? 2 : (current == 2 ? 5 : 1));
    }
    if(PhysicalKeyRisingEdge(FieldKeyRow::kB, 7))
    {
        core.TriggerMomentaryAction("play_toggle");
    }
    if(PhysicalKeyRisingEdge(FieldKeyRow::kB, 8))
    {
        core.TriggerMomentaryAction("reset");
    }
}

void ProcessCvGateAndMidi()
{
    for(size_t i = 0; i < 4; ++i)
    {
        cvValues[i] = hw.GetCvValue(i);
        const float normalized = i == 0 ? Clamp01(0.5f + cvValues[i] * 0.5f)
                                        : Clamp01(cvValues[i]);
        core.SetCvInput(i + 1, normalized);
    }

    core.TriggerGate(hw.gate_in.State());

    hw.midi.Listen();
    size_t midiEventsHandled = 0;
    while(hw.midi.HasEvents() && midiEventsHandled < kMaxMidiEventsPerTick)
    {
        MidiEvent event = hw.midi.PopEvent();
        switch(event.type)
        {
            case NoteOn:
            {
                const NoteOnEvent note = event.AsNoteOn();
                core.HandleMidiEvent(0x90,
                                     static_cast<uint8_t>(note.note),
                                     static_cast<uint8_t>(note.velocity));
            }
            break;
            case NoteOff:
            {
                const NoteOffEvent note = event.AsNoteOff();
                core.HandleMidiEvent(0x80,
                                     static_cast<uint8_t>(note.note),
                                     static_cast<uint8_t>(note.velocity));
            }
            break;
            case SystemRealTime:
                if(event.srt_type == TimingClock)
                {
                    core.HandleMidiEvent(0xF8, 0, 0);
                }
                break;
            default:
                break;
        }
        ++midiEventsHandled;
    }

    hw.SetCvOut1(ToDacCode(core.GetSequencerCv(0)));
    hw.SetCvOut2(ToDacCode(core.GetSequencerCv(1)));
    dsy_gpio_write(&hw.gate_out,
                   core.GetGateOutputPulse() > 0.5f ? 1 : 0);
}

void ProcessControls()
{
    hw.ProcessAllControls();
    ProcessKnobs();
    ProcessSwitchMenu();
    ProcessKeys();
    ProcessCvGateAndMidi();
}
} // namespace

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(kAudioBlockSize);
    core.Prepare(hw.AudioSampleRate(), kAudioBlockSize);
    core.ResetToDefaultState(0);

    hw.StartAdc();
    hw.midi.StartReceive();
    hw.StartAudio(AudioCallback);

    for(;;)
    {
        ProcessControls();
        UpdateDisplay();
        UpdateLeds();
        System::Delay(16);
    }
}
