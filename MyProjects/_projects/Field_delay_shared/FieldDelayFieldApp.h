#pragma once

#include "daisy_field.h"
#include "daisyhost/DaisyDelayFxCore.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace field_delay
{
using daisy::DaisyField;
using daisy::MidiEvent;
using daisy::NoteOffEvent;
using daisy::NoteOnEvent;
using daisy::System;
using daisyhost::DaisyDelayFxCore;
using daisyhost::DaisyDelayFxParameter;
using daisyhost::DaisyDelayFxSource;

namespace
{
DaisyField hw;
DaisyDelayFxCore* activeCore = nullptr;
const char* activeTitle = "";

float DSY_SDRAM_BSS
    delayStorage[DaisyDelayFxCore::kDelayLineCount]
                [DaisyDelayFxCore::kMaxDelaySamples];

constexpr std::size_t kAudioBlockSize = 48;
constexpr float       kTouchEpsilon = 0.012f;
constexpr uint32_t    kZoomMs = 1200;
constexpr uint32_t    kDisplayUpdateMs = 50;
constexpr uint32_t    kLedUpdateMs = 16;
constexpr uint32_t    kMainLoopDelayMs = 1;
constexpr std::size_t kMaxMidiEventsPerTick = 16;
constexpr std::size_t kBundleAlgorithmCount = 4;
constexpr std::size_t kMaxSnapshotParams = 32;

enum class FieldKeyRow
{
    kA,
    kB,
};

constexpr std::array<size_t, 8> kKnobLeds = {{
    DaisyField::LED_KNOB_1,
    DaisyField::LED_KNOB_2,
    DaisyField::LED_KNOB_3,
    DaisyField::LED_KNOB_4,
    DaisyField::LED_KNOB_5,
    DaisyField::LED_KNOB_6,
    DaisyField::LED_KNOB_7,
    DaisyField::LED_KNOB_8,
}};

constexpr std::array<size_t, 16> kKeyLeds = {{
    // Observed LED row order for this adapter is the inverse of the labels:
    // logical A values use LED_KEY_B*, logical B values use LED_KEY_A*.
    DaisyField::LED_KEY_B1,
    DaisyField::LED_KEY_B2,
    DaisyField::LED_KEY_B3,
    DaisyField::LED_KEY_B4,
    DaisyField::LED_KEY_B5,
    DaisyField::LED_KEY_B6,
    DaisyField::LED_KEY_B7,
    DaisyField::LED_KEY_B8,
    DaisyField::LED_KEY_A1,
    DaisyField::LED_KEY_A2,
    DaisyField::LED_KEY_A3,
    DaisyField::LED_KEY_A4,
    DaisyField::LED_KEY_A5,
    DaisyField::LED_KEY_A6,
    DaisyField::LED_KEY_A7,
    DaisyField::LED_KEY_A8,
}};

std::array<float, 8> knobValues{};
std::array<float, 8> layerEntryKnobValues{};
std::array<bool, 8>  layerKnobTouched{};
std::array<float, 4> cvValues{};
std::array<bool, 8>  bKeyDown{};
std::array<std::array<float, kMaxSnapshotParams>, kBundleAlgorithmCount>
    bundleSnapshots{};
std::array<bool, kBundleAlgorithmCount> bundleSnapshotValid{};
int                  activeLayer = 0;
int                  previousLayer = -1;
int                  zoomLayer = 0;
int                  zoomKnob = -1;
uint32_t             zoomStartMs = 0;
uint32_t             lastDisplayUpdateMs = 0;
uint32_t             lastLedUpdateMs = 0;
char                 zoomName[24] = {};
char                 zoomValue[24] = {};
char                 zoomUnits[8] = {};
bool                 bundleMode = false;

float Clamp01(float value)
{
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

std::size_t CurrentAlgorithmIndex()
{
    if(activeCore == nullptr)
    {
        return 0;
    }
    return daisyhost::DaisyDelayFxAlgorithmIndex(activeCore->GetSource());
}

float CurrentAlgorithmNormalized()
{
    return static_cast<float>(CurrentAlgorithmIndex())
           / static_cast<float>(kBundleAlgorithmCount - 1);
}

const char* CurrentAlgorithmLabel()
{
    if(activeCore == nullptr)
    {
        return "";
    }
    return daisyhost::GetDaisyDelayFxAlgorithmDescriptor(activeCore->GetSource())
        .label;
}

float ThreeStateLed(int state)
{
    return state == 0 ? 0.0f : (state == 1 ? 0.32f : 0.85f);
}

size_t KeyboardIndexForPhysicalKey(FieldKeyRow row, size_t oneBasedKey)
{
    if(oneBasedKey == 0 || oneBasedKey > 8)
    {
        return 0;
    }
    const size_t keyOffset = oneBasedKey - 1;
    // Observed Field keybed scan order for this adapter:
    // physical B1-B8 = 0..7, physical A1-A8 = 8..15.
    // LED enum order is different, so keep input and LED mappings separate.
    return (row == FieldKeyRow::kA ? 8 : 0) + keyOffset;
}

bool PhysicalKeyRisingEdge(FieldKeyRow row, size_t oneBasedKey)
{
    return hw.KeyboardRisingEdge(KeyboardIndexForPhysicalKey(row, oneBasedKey));
}

bool PhysicalKeyFallingEdge(FieldKeyRow row, size_t oneBasedKey)
{
    return hw.KeyboardFallingEdge(KeyboardIndexForPhysicalKey(row, oneBasedKey));
}

bool PhysicalKeyPressed(FieldKeyRow row, size_t oneBasedKey)
{
    return hw.KeyboardState(KeyboardIndexForPhysicalKey(row, oneBasedKey));
}

const char* LayerLabel(int layer)
{
    switch(layer)
    {
        case 1: return "SW1";
        case 2: return "SW2";
        default: return "BASE";
    }
}

const char* SynthModeLabel(int mode)
{
    switch(mode)
    {
        case 1: return "Pluck";
        case 2: return "Pad";
        default: return "Off";
    }
}

const char* SynthHoldLabel(int mode)
{
    switch(mode)
    {
        case 1: return "Latch";
        case 2: return "Drone";
        default: return "Moment";
    }
}

const char* WhiteKeyLabel(std::size_t zeroBasedIndex)
{
    static constexpr std::array<const char*, 8> kLabels = {{
        "C",
        "D",
        "E",
        "F",
        "G",
        "A",
        "B",
        "C",
    }};
    return zeroBasedIndex < kLabels.size() ? kLabels[zeroBasedIndex] : "?";
}

int WhiteKeyBaseOctave(std::size_t zeroBasedIndex)
{
    return zeroBasedIndex == 7 ? 5 : 4;
}

int CurrentLayer()
{
    if(hw.GetSwitch(DaisyField::SW_1)->Pressed())
    {
        return 1;
    }
    if(hw.GetSwitch(DaisyField::SW_2)->Pressed())
    {
        return 2;
    }
    return 0;
}

void EnterLayer(int layer)
{
    activeLayer = layer;
    previousLayer = layer;
    for(std::size_t i = 0; i < knobValues.size(); ++i)
    {
        layerEntryKnobValues[i] = knobValues[i];
        layerKnobTouched[i] = false;
    }
}

void RecordZoom(const DaisyDelayFxParameter* parameter,
                const char*                  valueText,
                int                          layer,
                int                          knob)
{
    if(parameter == nullptr)
    {
        return;
    }
    std::snprintf(zoomName, sizeof(zoomName), "%s", parameter->label.c_str());
    std::snprintf(zoomValue, sizeof(zoomValue), "%s", valueText);
    std::snprintf(zoomUnits,
                  sizeof(zoomUnits),
                  "%s",
                  parameter->unitLabel.empty() ? "%" : parameter->unitLabel.c_str());
    zoomLayer = layer;
    zoomKnob = knob;
    zoomStartMs = System::GetNow();
}

void RecordAlgorithmZoom()
{
    std::snprintf(zoomName, sizeof(zoomName), "Algorithm");
    std::snprintf(zoomValue, sizeof(zoomValue), "%.23s", CurrentAlgorithmLabel());
    std::snprintf(zoomUnits, sizeof(zoomUnits), "mode");
    zoomLayer = activeLayer;
    zoomKnob = -1;
    zoomStartMs = System::GetNow();
}

void SaveBundleSnapshot()
{
    if(activeCore == nullptr)
    {
        return;
    }
    const std::size_t index = CurrentAlgorithmIndex();
    const auto& parameters = activeCore->GetParameters();
    const std::size_t count = parameters.size() < kMaxSnapshotParams
                                  ? parameters.size()
                                  : kMaxSnapshotParams;
    for(std::size_t i = 0; i < count; ++i)
    {
        bundleSnapshots[index][i] = parameters[i].normalizedValue;
    }
    bundleSnapshotValid[index] = true;
}

void RestoreBundleSnapshot(std::size_t index)
{
    if(activeCore == nullptr || index >= kBundleAlgorithmCount
       || !bundleSnapshotValid[index])
    {
        return;
    }
    const auto& parameters = activeCore->GetParameters();
    const std::size_t count = parameters.size() < kMaxSnapshotParams
                                  ? parameters.size()
                                  : kMaxSnapshotParams;
    for(std::size_t i = 0; i < count; ++i)
    {
        activeCore->SetParameterValue(parameters[i].id,
                                      bundleSnapshots[index][i]);
    }
}

void SelectBundleAlgorithm(std::size_t index)
{
    if(activeCore == nullptr || index >= kBundleAlgorithmCount)
    {
        return;
    }
    SaveBundleSnapshot();
    activeCore->SetSource(daisyhost::DaisyDelayFxSourceForAlgorithmIndex(index));
    RestoreBundleSnapshot(index);
    EnterLayer(CurrentLayer());
    RecordAlgorithmZoom();
}

void AudioCallback(daisy::AudioHandle::InputBuffer  in,
                   daisy::AudioHandle::OutputBuffer out,
                   size_t                           size)
{
    if(activeCore == nullptr)
    {
        for(size_t i = 0; i < size; ++i)
        {
            out[0][i] = in[0][i];
            out[1][i] = in[1][i];
        }
        return;
    }
    activeCore->Process(in[0], in[1], out[0], out[1], size);
}

void ProcessLayeredKnobs()
{
    const int layer = CurrentLayer();
    if(layer != previousLayer)
    {
        EnterLayer(layer);
    }

    for(std::size_t i = 0; i < knobValues.size(); ++i)
    {
        knobValues[i] = hw.GetKnobValue(i);
        if(!layerKnobTouched[i])
        {
            if(std::fabs(knobValues[i] - layerEntryKnobValues[i]) < kTouchEpsilon)
            {
                continue;
            }
            layerKnobTouched[i] = true;
        }

        const char* parameterId
            = activeCore->GetParameterForLayerKnob(static_cast<std::size_t>(layer), i);
        if(parameterId[0] == '\0')
        {
            continue;
        }
        activeCore->SetParameterValue(parameterId, knobValues[i]);
        char valueText[24];
        activeCore->FormatParameterValue(parameterId, valueText, sizeof(valueText));
        RecordZoom(activeCore->FindParameter(parameterId),
                   valueText,
                   layer,
                   static_cast<int>(i));
    }
}

void ProcessAKeys()
{
    for(std::size_t i = 0; i < 8; ++i)
    {
        if(PhysicalKeyRisingEdge(FieldKeyRow::kA, i + 1))
        {
            if(bundleMode && i < 4)
            {
                SelectBundleAlgorithm(i);
            }
            else if(bundleMode && i == 4)
            {
                activeCore->SetInternalSynthMode(
                    (activeCore->GetInternalSynthMode() + 1) % 3);
            }
            else if(bundleMode && i == 5)
            {
                activeCore->SetInternalSynthHoldMode(
                    (activeCore->GetInternalSynthHoldMode() + 1) % 3);
            }
            else
            {
                activeCore->TriggerFieldKeyAction(i, true);
            }

            if(!bundleMode || i >= 4)
            {
                std::snprintf(zoomName,
                              sizeof(zoomName),
                              bundleMode && i == 4
                                  ? "A5 Synth"
                                  : (bundleMode && i == 5 ? "A6 Hold" : "A%u"),
                              static_cast<unsigned>(i + 1));
                if(bundleMode && i == 4)
                {
                    std::snprintf(zoomValue,
                                  sizeof(zoomValue),
                                  "%s",
                                  SynthModeLabel(activeCore->GetInternalSynthMode()));
                    std::snprintf(zoomUnits, sizeof(zoomUnits), "mode");
                }
                else if(bundleMode && i == 5)
                {
                    std::snprintf(zoomValue,
                                  sizeof(zoomValue),
                                  "%s",
                                  SynthHoldLabel(
                                      activeCore->GetInternalSynthHoldMode()));
                    std::snprintf(zoomUnits, sizeof(zoomUnits), "mode");
                }
                else
                {
                    std::snprintf(zoomValue,
                                  sizeof(zoomValue),
                                  "%d",
                                  activeCore->GetButtonState(i));
                    std::snprintf(zoomUnits, sizeof(zoomUnits), "state");
                }
                zoomLayer = activeLayer;
                zoomKnob = -1;
                zoomStartMs = System::GetNow();
            }
        }
        if(PhysicalKeyFallingEdge(FieldKeyRow::kA, i + 1))
        {
            if(!bundleMode || i >= 6)
            {
                activeCore->TriggerFieldKeyAction(i, false);
            }
        }
    }
}

void ProcessBKeys()
{
    for(std::size_t i = 0; i < 8; ++i)
    {
        const bool pressed = PhysicalKeyPressed(FieldKeyRow::kB, i + 1);
        if(PhysicalKeyRisingEdge(FieldKeyRow::kB, i + 1))
        {
            bKeyDown[i] = true;
            activeCore->TriggerFieldKeyAction(8 + i, true);
            std::snprintf(zoomName,
                          sizeof(zoomName),
                          "B%u Note",
                          static_cast<unsigned>(i + 1));
            std::snprintf(zoomValue,
                          sizeof(zoomValue),
                          "%s%d",
                          WhiteKeyLabel(i),
                          WhiteKeyBaseOctave(i)
                              + activeCore->GetKeyboardOctaveOffset());
            std::snprintf(zoomUnits, sizeof(zoomUnits), "key");
            zoomLayer = activeLayer;
            zoomKnob = -1;
            zoomStartMs = System::GetNow();
        }
        if(PhysicalKeyFallingEdge(FieldKeyRow::kB, i + 1)
           || (bKeyDown[i] && !pressed))
        {
            bKeyDown[i] = false;
            activeCore->TriggerFieldKeyAction(8 + i, false);
        }
    }
}

void ProcessMidi()
{
    hw.midi.Listen();
    std::size_t handled = 0;
    while(hw.midi.HasEvents() && handled < kMaxMidiEventsPerTick)
    {
        MidiEvent event = hw.midi.PopEvent();
        switch(event.type)
        {
            case daisy::NoteOn:
            {
                NoteOnEvent note = event.AsNoteOn();
                activeCore->HandleMidiEvent(0x90,
                                            static_cast<uint8_t>(note.note),
                                            static_cast<uint8_t>(note.velocity));
            }
            break;
            case daisy::NoteOff:
            {
                NoteOffEvent note = event.AsNoteOff();
                activeCore->HandleMidiEvent(0x80,
                                            static_cast<uint8_t>(note.note),
                                            static_cast<uint8_t>(note.velocity));
            }
            break;
            default:
                break;
        }
        ++handled;
    }
}

void ProcessControls()
{
    hw.ProcessAllControls();
    for(std::size_t i = 0; i < knobValues.size(); ++i)
    {
        knobValues[i] = hw.GetKnobValue(i);
    }
    if(previousLayer < 0)
    {
        EnterLayer(CurrentLayer());
    }
    ProcessLayeredKnobs();
    ProcessAKeys();
    ProcessBKeys();
    ProcessMidi();

    for(std::size_t i = 0; i < cvValues.size(); ++i)
    {
        cvValues[i] = hw.GetCvValue(i);
    }

    hw.SetCvOut1(static_cast<uint16_t>(Clamp01(knobValues[0]) * 4095.0f));
    hw.SetCvOut2(static_cast<uint16_t>(Clamp01(knobValues[1]) * 4095.0f));
}

void WriteDisplayLine(int y, const char* text)
{
    hw.display.SetCursor(0, y);
    hw.display.WriteString(text, Font_6x8, true);
}

void DrawZoom()
{
    char line[32];
    if(zoomKnob >= 0)
    {
        std::snprintf(line,
                      sizeof(line),
                      "%s K%d",
                      LayerLabel(zoomLayer),
                      zoomKnob + 1);
    }
    else
    {
        std::snprintf(line, sizeof(line), "%.31s", activeTitle);
    }
    WriteDisplayLine(0, line);
    WriteDisplayLine(14, zoomName);
    std::snprintf(line, sizeof(line), "%s %s", zoomValue, zoomUnits);
    WriteDisplayLine(28, line);

    float normalized = 0.0f;
    if(zoomKnob >= 0)
    {
        normalized = knobValues[static_cast<std::size_t>(zoomKnob)];
    }
    else if(bundleMode)
    {
        normalized = CurrentAlgorithmNormalized();
    }
    hw.display.DrawRect(0, 50, 127, 58, true, false);
    hw.display.DrawRect(0,
                        50,
                        static_cast<int>(Clamp01(normalized) * 127.0f),
                        58,
                        true,
                        true);
}

void DrawMain()
{
    char line[32];
    std::snprintf(line, sizeof(line), "%.20s", activeTitle);
    WriteDisplayLine(0, line);
    if(bundleMode)
    {
        std::snprintf(line,
                      sizeof(line),
                      "%.18s Oct %+d",
                      CurrentAlgorithmLabel(),
                      activeCore->GetKeyboardOctaveOffset());
    }
    else
    {
        std::snprintf(line,
                      sizeof(line),
                      "Layer %s Oct %+d",
                      LayerLabel(activeLayer),
                      activeCore->GetKeyboardOctaveOffset());
    }
    WriteDisplayLine(10, line);

    for(std::size_t i = 0; i < 4; ++i)
    {
        const char* parameterId = activeCore->GetParameterForLayerKnob(
            static_cast<std::size_t>(activeLayer), i);
        const auto* parameter = activeCore->FindParameter(parameterId);
        char valueText[24];
        activeCore->FormatParameterValue(parameterId, valueText, sizeof(valueText));
        std::snprintf(line,
                      sizeof(line),
                      "K%u %.8s %s",
                      static_cast<unsigned>(i + 1),
                      parameter != nullptr ? parameter->label.c_str() : "",
                      valueText);
        WriteDisplayLine(20 + static_cast<int>(i) * 10, line);
    }
}

void UpdateDisplay()
{
    hw.display.Fill(false);
    const uint32_t now = System::GetNow();
    if(zoomStartMs != 0 && now - zoomStartMs < kZoomMs)
    {
        DrawZoom();
    }
    else
    {
        DrawMain();
    }
    hw.display.Update();
}

void ShowStartupDisplay()
{
    hw.display.Fill(false);
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(activeTitle, Font_7x10, true);
    hw.display.SetCursor(0, 16);
    hw.display.WriteString("Field delay bundle", Font_6x8, true);
    hw.display.SetCursor(0, 28);
    hw.display.WriteString("Controls settling", Font_6x8, true);
    hw.display.Update();
}

void PrimeControls()
{
    for(std::size_t pass = 0; pass < 24; ++pass)
    {
        hw.ProcessAllControls();
        for(std::size_t i = 0; i < knobValues.size(); ++i)
        {
            knobValues[i] = hw.GetKnobValue(i);
        }
        System::Delay(1);
    }
    EnterLayer(CurrentLayer());
}

std::array<float, DaisyDelayFxCore::kFieldKeyCount> BundleFieldKeyLedValues()
{
    auto values = activeCore->GetFieldKeyLedValues();
    const std::size_t selected = CurrentAlgorithmIndex();
    for(std::size_t i = 0; i < 4; ++i)
    {
        values[i] = i == selected ? 0.85f : 0.03f;
    }
    values[4] = ThreeStateLed(activeCore->GetInternalSynthMode());
    values[5] = ThreeStateLed(activeCore->GetInternalSynthHoldMode());
    for(std::size_t i = 0; i < bKeyDown.size(); ++i)
    {
        if(bKeyDown[i])
        {
            values[8 + i] = 0.85f;
        }
    }
    return values;
}

void UpdateLeds()
{
    for(std::size_t i = 0; i < kKnobLeds.size(); ++i)
    {
        hw.led_driver.SetLed(kKnobLeds[i], knobValues[i] * 0.55f);
    }

    const auto keyLedValues = bundleMode ? BundleFieldKeyLedValues()
                                         : activeCore->GetFieldKeyLedValues();
    const bool blinkOn = ((System::GetNow() / 240) % 2) == 0;
    for(std::size_t i = 0; i < kKeyLeds.size(); ++i)
    {
        float value = keyLedValues[i];
        if(value > 0.1f && value < 0.5f)
        {
            value = blinkOn ? 0.45f : 0.04f;
        }
        hw.led_driver.SetLed(kKeyLeds[i], value);
    }
    hw.led_driver.SetLed(DaisyField::LED_SW_1, activeLayer == 1 ? 0.85f : 0.05f);
    hw.led_driver.SetLed(DaisyField::LED_SW_2, activeLayer == 2 ? 0.85f : 0.05f);
    hw.led_driver.SwapBuffersAndTransmit();
}

int RunConfiguredFieldDelayProject(DaisyDelayFxCore* core,
                                   const char*       title,
                                   bool              isBundle)
{
    activeCore = core;
    activeTitle = title;
    bundleMode = isBundle;
    activeCore->SetBundleMode(isBundle);
    bundleSnapshotValid.fill(false);
    previousLayer = -1;
    zoomStartMs = 0;
    lastDisplayUpdateMs = 0;
    lastLedUpdateMs = 0;

    hw.Init();
    hw.SetAudioBlockSize(kAudioBlockSize);
    hw.SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate::SAI_48KHZ);

    activeCore->AttachDelayStorage(&delayStorage[0][0],
                                   DaisyDelayFxCore::kDelayLineCount,
                                   DaisyDelayFxCore::kMaxDelaySamples);
    activeCore->Prepare(hw.AudioSampleRate(), kAudioBlockSize);

    ShowStartupDisplay();
    hw.StartAdc();
    PrimeControls();
    hw.midi.StartReceive();
    System::Delay(600);
    hw.StartAudio(AudioCallback);
    lastDisplayUpdateMs = System::GetNow() - kDisplayUpdateMs;
    lastLedUpdateMs = System::GetNow() - kLedUpdateMs;

    for(;;)
    {
        ProcessControls();
        const uint32_t now = System::GetNow();
        if(now - lastDisplayUpdateMs >= kDisplayUpdateMs)
        {
            UpdateDisplay();
            lastDisplayUpdateMs = now;
        }
        if(now - lastLedUpdateMs >= kLedUpdateMs)
        {
            UpdateLeds();
            lastLedUpdateMs = now;
        }
        System::Delay(kMainLoopDelayMs);
    }
    return 0;
}
} // namespace

int RunFieldDelayProject(DaisyDelayFxSource source, const char* title)
{
    static DaisyDelayFxCore core(source);
    return RunConfiguredFieldDelayProject(&core, title, false);
}

int RunFieldDelayBundleProject(const char* title)
{
    static DaisyDelayFxCore core(DaisyDelayFxSource::kMultiFxPedal);
    return RunConfiguredFieldDelayProject(&core, title, true);
}
} // namespace field_delay
