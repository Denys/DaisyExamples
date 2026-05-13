#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "daisyhost/DaisySubharmoniqCore.h"
#include "daisyhost/HostSessionState.h"
#include "../src/juce/DaisyHostPluginProcessor.h"

namespace
{
constexpr double kSampleRate = 48000.0;
constexpr int    kBlockSize  = 64;

daisyhost::HostSessionState MakeFieldSubharmoniqSession(
    daisyhost::DaisySubharmoniqPage page)
{
    daisyhost::HostSessionState state;
    state.boardId        = "daisy_field";
    state.selectedNodeId = "node0";
    state.entryNodeId    = "node0";
    state.outputNodeId   = "node0";
    state.nodes.push_back({"node0", "subharmoniq", 0u});
    state.parameterValues["node0/param/state/page_schema"] = 1.0f;
    state.parameterValues["node0/param/state/page"]
        = static_cast<float>(static_cast<int>(page));
    state.parameterValues["node0/param/output"] = 0.90f;
    return state;
}

daisyhost::HostSessionState MakeFieldSubharmoniqStartupSession()
{
    daisyhost::HostSessionState state;
    state.boardId        = "daisy_field";
    state.selectedNodeId = "node0";
    state.entryNodeId    = "node0";
    state.outputNodeId   = "node0";
    state.nodes.push_back({"node0", "subharmoniq", 0u});
    return state;
}

void LoadSession(DaisyHostPatchAudioProcessor& processor,
                 const daisyhost::HostSessionState& state)
{
    const std::string serialized = state.Serialize();
    processor.setStateInformation(serialized.data(),
                                  static_cast<int>(serialized.size()));
}

float ConsumeBlockEnergy(const juce::AudioBuffer<float>& buffer)
{
    float energy = 0.0f;
    for(int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const auto* samples = buffer.getReadPointer(channel);
        for(int frame = 0; frame < buffer.getNumSamples(); ++frame)
        {
            EXPECT_TRUE(std::isfinite(samples[frame]));
            energy += std::abs(samples[frame]);
        }
    }
    return energy;
}

float RenderMidiNoteEnergy(DaisyHostPatchAudioProcessor& processor, int midiNote)
{
    juce::AudioBuffer<float> buffer(2, kBlockSize);
    juce::MidiBuffer         midi;
    float                    energy = 0.0f;

    for(int block = 0; block < 64; ++block)
    {
        buffer.clear();
        midi.clear();
        if(block == 0)
        {
            midi.addEvent(juce::MidiMessage::noteOn(1,
                                                    midiNote,
                                                    static_cast<juce::uint8>(100)),
                          0);
        }

        processor.processBlock(buffer, midi);
        energy += ConsumeBlockEnergy(buffer);
    }

    buffer.clear();
    midi.clear();
    midi.addEvent(juce::MidiMessage::noteOff(1, midiNote), 0);
    processor.processBlock(buffer, midi);
    return energy + ConsumeBlockEnergy(buffer);
}

float RenderVirtualKeyboardNoteEnergy(DaisyHostPatchAudioProcessor& processor,
                                      int                           midiNote)
{
    juce::AudioBuffer<float> buffer(2, kBlockSize);
    juce::MidiBuffer         midi;
    float                    energy = 0.0f;

    processor.SetVirtualKeyboardNote(midiNote, true, 1.0f);
    for(int block = 0; block < 64; ++block)
    {
        buffer.clear();
        midi.clear();
        processor.processBlock(buffer, midi);
        energy += ConsumeBlockEnergy(buffer);
    }

    processor.SetVirtualKeyboardNote(midiNote, false, 0.0f);
    buffer.clear();
    midi.clear();
    processor.processBlock(buffer, midi);
    return energy + ConsumeBlockEnergy(buffer);
}
} // namespace

TEST(DaisyHostPluginProcessorTest,
     FieldSubharmoniqRestoredVcfPageKeepsRepeatedMidiAudible)
{
    DaisyHostPatchAudioProcessor processor;
    LoadSession(processor,
                MakeFieldSubharmoniqSession(daisyhost::DaisySubharmoniqPage::kVcf));

    processor.setPlayConfigDetails(2, 2, kSampleRate, kBlockSize);
    processor.prepareToPlay(kSampleRate, kBlockSize);

    ASSERT_TRUE(processor.IsDaisyFieldBoard());
    ASSERT_EQ(processor.GetActiveAppId().toStdString(), "subharmoniq");
    ASSERT_EQ(processor.GetFieldKnobTargetId(7), "node0/control/output");
    EXPECT_GT(processor.GetFieldKnobValue(7), 0.50f);

    std::vector<float> noteEnergies;
    for(const int note : {48, 52, 55, 60})
    {
        noteEnergies.push_back(RenderMidiNoteEnergy(processor, note));
    }

    for(std::size_t index = 0; index < noteEnergies.size(); ++index)
    {
        EXPECT_GT(noteEnergies[index], 0.001f) << "note index " << index;
    }
}

TEST(DaisyHostPluginProcessorTest,
     FieldSubharmoniqFreshStartupKeepsMidiAudible)
{
    DaisyHostPatchAudioProcessor processor;
    LoadSession(processor, MakeFieldSubharmoniqStartupSession());

    processor.setPlayConfigDetails(2, 2, kSampleRate, kBlockSize);
    processor.prepareToPlay(kSampleRate, kBlockSize);

    ASSERT_TRUE(processor.IsDaisyFieldBoard());
    ASSERT_EQ(processor.GetActiveAppId().toStdString(), "subharmoniq");

    const float noteEnergy = RenderMidiNoteEnergy(processor, 60);
    EXPECT_GT(noteEnergy, 0.001f);
}

TEST(DaisyHostPluginProcessorTest,
     FieldSubharmoniqVirtualKeyboardKeepsMidiAudible)
{
    DaisyHostPatchAudioProcessor processor;
    LoadSession(processor, MakeFieldSubharmoniqStartupSession());

    processor.setPlayConfigDetails(2, 2, kSampleRate, kBlockSize);
    processor.prepareToPlay(kSampleRate, kBlockSize);

    ASSERT_TRUE(processor.IsDaisyFieldBoard());
    ASSERT_EQ(processor.GetActiveAppId().toStdString(), "subharmoniq");

    const float noteEnergy = RenderVirtualKeyboardNoteEnergy(processor, 60);
    EXPECT_GT(noteEnergy, 0.001f);
}

TEST(DaisyHostPluginProcessorTest,
     FieldSubharmoniqLowComputerKeyboardOctaveKeepsMidiAudible)
{
    DaisyHostPatchAudioProcessor processor;
    LoadSession(processor, MakeFieldSubharmoniqStartupSession());

    processor.setPlayConfigDetails(2, 2, kSampleRate, kBlockSize);
    processor.prepareToPlay(kSampleRate, kBlockSize);

    ASSERT_TRUE(processor.IsDaisyFieldBoard());
    ASSERT_EQ(processor.GetActiveAppId().toStdString(), "subharmoniq");

    const float noteEnergy = RenderMidiNoteEnergy(processor, 34);
    EXPECT_GT(noteEnergy, 0.001f);
}
