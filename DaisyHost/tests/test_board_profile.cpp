#include <gtest/gtest.h>

#include "daisyhost/BoardProfile.h"
#include "daisyhost/apps/MultiDelayCore.h"

namespace
{
const daisyhost::PanelControlSlotSpec* FindSurfaceControl(
    const daisyhost::BoardProfile& profile,
    const std::string&             id)
{
    for(const auto& control : profile.surfaceControls)
    {
        if(control.id == id)
        {
            return &control;
        }
    }
    return nullptr;
}

const daisyhost::PanelDecorationSpec* FindDecoration(
    const daisyhost::BoardProfile&     profile,
    daisyhost::PanelDecorationKind kind)
{
    for(const auto& decoration : profile.decorations)
    {
        if(decoration.kind == kind)
        {
            return &decoration;
        }
    }
    return nullptr;
}

bool HasText(const daisyhost::BoardProfile& profile, const std::string& text)
{
    for(const auto& spec : profile.texts)
    {
        if(spec.text == text)
        {
            return true;
        }
    }
    return false;
}

TEST(BoardProfileTest, DaisyPatchProfileHasExpectedControlsAndPorts)
{
    const daisyhost::BoardProfile profile
        = daisyhost::MakeDaisyPatchProfile("nodeA");

    EXPECT_EQ(profile.boardId, "daisy_patch");
    EXPECT_EQ(profile.nodeId, "nodeA");
    EXPECT_EQ(profile.controls.size(), 6u);
    EXPECT_EQ(profile.display.width, 128);
    EXPECT_EQ(profile.display.height, 64);
    EXPECT_EQ(profile.surfaceControls.size(), 6u);
    EXPECT_FALSE(profile.decorations.empty());
    EXPECT_FALSE(profile.texts.empty());

    std::size_t audioInputs  = 0;
    std::size_t audioOutputs = 0;
    std::size_t cvInputs     = 0;
    std::size_t gateInputs   = 0;
    std::size_t midiPorts    = 0;

    for(const auto& port : profile.ports)
    {
        switch(port.type)
        {
            case daisyhost::VirtualPortType::kAudio:
                if(port.direction == daisyhost::PortDirection::kInput)
                {
                    ++audioInputs;
                }
                else
                {
                    ++audioOutputs;
                }
                break;
            case daisyhost::VirtualPortType::kCv: ++cvInputs; break;
            case daisyhost::VirtualPortType::kGate:
                if(port.direction == daisyhost::PortDirection::kInput)
                {
                    ++gateInputs;
                }
                break;
            case daisyhost::VirtualPortType::kMidi: ++midiPorts; break;
        }
    }

    EXPECT_EQ(audioInputs, 4u);
    EXPECT_EQ(audioOutputs, 4u);
    EXPECT_EQ(cvInputs, 4u);
    EXPECT_EQ(gateInputs, 2u);
    EXPECT_EQ(midiPorts, 2u);
}

TEST(BoardProfileTest, DaisyPatchProfileExposesFinalSurfaceControlHierarchy)
{
    const std::string             nodeId  = "nodeA";
    const daisyhost::BoardProfile profile = daisyhost::MakeDaisyPatchProfile(nodeId);

    const auto* ctrl1 = FindSurfaceControl(profile, nodeId + "/surface/ctrl1_mix");
    const auto* ctrl2 = FindSurfaceControl(profile, nodeId + "/surface/ctrl2_primary");
    const auto* ctrl3 = FindSurfaceControl(profile, nodeId + "/surface/ctrl3_secondary");
    const auto* ctrl4 = FindSurfaceControl(profile, nodeId + "/surface/ctrl4_feedback");
    const auto* enc1  = FindSurfaceControl(profile, nodeId + "/surface/enc1");
    const auto* push  = FindSurfaceControl(profile, nodeId + "/surface/enc1_push");

    ASSERT_NE(ctrl1, nullptr);
    ASSERT_NE(ctrl2, nullptr);
    ASSERT_NE(ctrl3, nullptr);
    ASSERT_NE(ctrl4, nullptr);
    ASSERT_NE(enc1, nullptr);
    ASSERT_NE(push, nullptr);

    EXPECT_EQ(ctrl1->targetId, daisyhost::apps::MultiDelayCore::MakeDryWetControlId(nodeId));
    EXPECT_EQ(ctrl2->targetId, daisyhost::apps::MultiDelayCore::MakeKnobControlId(nodeId, 1));
    EXPECT_EQ(ctrl3->targetId, daisyhost::apps::MultiDelayCore::MakeKnobControlId(nodeId, 2));
    EXPECT_EQ(ctrl4->targetId, daisyhost::apps::MultiDelayCore::MakeKnobControlId(nodeId, 4));
    EXPECT_EQ(enc1->targetId, daisyhost::apps::MultiDelayCore::MakeEncoderControlId(nodeId));
    EXPECT_EQ(push->targetId,
              daisyhost::apps::MultiDelayCore::MakeEncoderButtonControlId(nodeId));

    EXPECT_LT(ctrl1->panelBounds.x, ctrl2->panelBounds.x);
    EXPECT_LT(ctrl2->panelBounds.x, ctrl3->panelBounds.x);
    EXPECT_LT(ctrl3->panelBounds.x, ctrl4->panelBounds.x);
    EXPECT_GT(enc1->panelBounds.y, ctrl1->panelBounds.y);
    EXPECT_LT(push->panelBounds.y, enc1->panelBounds.y);
}

TEST(BoardProfileTest, DaisyPatchProfileCarriesPatchArtworkMetadata)
{
    const daisyhost::BoardProfile profile
        = daisyhost::MakeDaisyPatchProfile("nodeA");

    EXPECT_NE(FindDecoration(profile, daisyhost::PanelDecorationKind::kCvBay), nullptr);
    EXPECT_NE(FindDecoration(profile, daisyhost::PanelDecorationKind::kGateColumn), nullptr);
    EXPECT_NE(FindDecoration(profile, daisyhost::PanelDecorationKind::kAudioSection), nullptr);
    EXPECT_NE(FindDecoration(profile, daisyhost::PanelDecorationKind::kMidiSection), nullptr);
    EXPECT_NE(FindDecoration(profile, daisyhost::PanelDecorationKind::kSeedModule), nullptr);

    EXPECT_TRUE(HasText(profile, "DAISY PATCH"));
    EXPECT_TRUE(HasText(profile, "DAISYHOST"));
    EXPECT_TRUE(HasText(profile, "CV INPUTS"));
    EXPECT_TRUE(HasText(profile, "AUDIO"));
    EXPECT_TRUE(HasText(profile, "MIDI"));
    EXPECT_TRUE(HasText(profile, "ELECTROSMITH"));
}

TEST(BoardProfileTest, DaisyPatchProfilePlacesTopRowAndSectionPortsLikePatch)
{
    const std::string             nodeId  = "nodeA";
    const daisyhost::BoardProfile profile = daisyhost::MakeDaisyPatchProfile(nodeId);

    std::size_t cvTopRowPorts   = 0;
    std::size_t gateColumnPorts = 0;
    std::size_t bottomAudio     = 0;
    std::size_t bottomMidi      = 0;

    for(const auto& port : profile.ports)
    {
        if(port.type == daisyhost::VirtualPortType::kCv
           && port.direction == daisyhost::PortDirection::kInput
           && port.panelBounds.y < 0.12f)
        {
            ++cvTopRowPorts;
        }

        if(port.type == daisyhost::VirtualPortType::kGate && port.panelBounds.x > 0.85f)
        {
            ++gateColumnPorts;
        }

        if(port.type == daisyhost::VirtualPortType::kAudio && port.panelBounds.y > 0.74f)
        {
            ++bottomAudio;
        }

        if(port.type == daisyhost::VirtualPortType::kMidi && port.panelBounds.y > 0.74f)
        {
            ++bottomMidi;
        }
    }

    EXPECT_EQ(cvTopRowPorts, 4u);
    EXPECT_EQ(gateColumnPorts, 3u);
    EXPECT_EQ(bottomAudio, 8u);
    EXPECT_EQ(bottomMidi, 2u);
}
} // namespace
