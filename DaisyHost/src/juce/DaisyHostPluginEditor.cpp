#include "DaisyHostPluginEditor.h"

#include <algorithm>
#include <cmath>
#include <type_traits>
#include <utility>

#include "daisyhost/AppRegistry.h"
#include "daisyhost/ComputerKeyboardMidi.h"
#include "daisyhost/StandaloneUiPolicy.h"

namespace
{
juce::Colour BackgroundColour()
{
    return juce::Colour::fromRGB(20, 23, 27);
}

juce::Colour PatchPanelColour()
{
    return juce::Colour::fromRGB(108, 116, 128);
}

juce::Colour PatchPanelShadow()
{
    return juce::Colour::fromRGB(63, 69, 77);
}

juce::Colour DrawerColour()
{
    return juce::Colour::fromRGB(49, 55, 62);
}

juce::Colour DrawerInsetColour()
{
    return juce::Colour::fromRGB(36, 40, 45);
}

juce::Colour AccentColour()
{
    return juce::Colour::fromRGB(235, 154, 56);
}

juce::Colour CreamColour()
{
    return juce::Colour::fromRGB(243, 242, 236);
}

juce::Colour InkColour()
{
    return juce::Colour::fromRGB(25, 28, 34);
}

juce::Colour TraceColour()
{
    return juce::Colour::fromRGB(224, 229, 234);
}

juce::Colour TextColour()
{
    return juce::Colour::fromRGB(249, 249, 246);
}

juce::Colour MutedTextColour()
{
    return juce::Colour::fromRGB(214, 218, 222);
}

juce::Colour PortColour(daisyhost::VirtualPortType type)
{
    switch(type)
    {
        case daisyhost::VirtualPortType::kAudio:
            return juce::Colour::fromRGB(92, 208, 218);
        case daisyhost::VirtualPortType::kCv:
            return juce::Colour::fromRGB(104, 182, 255);
        case daisyhost::VirtualPortType::kGate:
            return juce::Colour::fromRGB(239, 226, 128);
        case daisyhost::VirtualPortType::kMidi:
            return AccentColour();
        default: return AccentColour();
    }
}

juce::Font TitleFont(float size)
{
    return juce::Font(juce::FontOptions(size, juce::Font::bold));
}

juce::Font BodyFont(float size)
{
    return juce::Font(juce::FontOptions(size));
}

juce::Rectangle<float> SquareInside(const juce::Rectangle<float>& area, float scale = 1.0f)
{
    const float side = std::min(area.getWidth(), area.getHeight()) * scale;
    return {area.getCentreX() - side * 0.5f,
            area.getCentreY() - side * 0.5f,
            side,
            side};
}

int OneBasedIndexFromLabel(const std::string& label)
{
    for(auto it = label.rbegin(); it != label.rend(); ++it)
    {
        if(*it >= '1' && *it <= '9')
        {
            return *it - '0';
        }
    }
    return 0;
}

daisyhost::UiRect ToUiRect(const juce::Rectangle<int>& rect)
{
    return {rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight()};
}

juce::Rectangle<int> ToJuceRect(const daisyhost::UiRect& rect)
{
    return {rect.x, rect.y, rect.width, rect.height};
}

constexpr std::size_t kVisibleRackNodeCount = 2;
constexpr int kRackTopologyNode0Only        = 0;
constexpr int kRackTopologyNode1Only        = 1;
constexpr int kRackTopologyNode0ToNode1     = 2;
constexpr int kRackTopologyNode1ToNode0     = 3;

juce::String GetRackTopologyLabel(int preset)
{
    switch(preset)
    {
        case kRackTopologyNode0Only: return "Node 0 Only";
        case kRackTopologyNode1Only: return "Node 1 Only";
        case kRackTopologyNode0ToNode1: return "Node 0 -> Node 1";
        case kRackTopologyNode1ToNode0: return "Node 1 -> Node 0";
        default: return "Node 0 Only";
    }
}

juce::String GetHostedAppDisplayNameForId(const std::string& appId)
{
    for(const auto& registration : daisyhost::GetHostedAppRegistrations())
    {
        if(registration.appId == appId)
        {
            return registration.displayName;
        }
    }

    return appId.empty() ? "Unassigned" : juce::String(appId);
}

template <typename Processor, typename = void>
struct HasRackEditorApi : std::false_type
{};

template <typename Processor>
struct HasRackEditorApi<
    Processor,
    std::void_t<decltype(std::declval<const Processor&>().GetRackNodeCount()),
                decltype(std::declval<const Processor&>().GetSelectedRackNodeId()),
                decltype(std::declval<Processor&>().SetSelectedRackNodeId(
                    std::declval<const std::string&>())),
                decltype(std::declval<const Processor&>().GetRackNodeId(
                    std::declval<std::size_t>())),
                decltype(std::declval<const Processor&>().GetRackNodeAppId(
                    std::declval<std::size_t>())),
                decltype(std::declval<const Processor&>().GetRackNodeAppDisplayName(
                    std::declval<std::size_t>())),
                decltype(std::declval<Processor&>().SetRackNodeAppId(
                    std::declval<std::size_t>(),
                    std::declval<const std::string&>())),
                decltype(std::declval<const Processor&>().GetRackTopologyPreset()),
                decltype(std::declval<Processor&>().SetRackTopologyPreset(0)),
                decltype(std::declval<const Processor&>().GetRackNodeRoleLabel(
                    std::declval<std::size_t>()))>> : std::true_type
{};

template <typename Processor>
std::size_t GetRackNodeCountCompat(const Processor& processor)
{
    if constexpr(HasRackEditorApi<Processor>::value)
    {
        return processor.GetRackNodeCount();
    }
    else
    {
        juce::ignoreUnused(processor);
        return kVisibleRackNodeCount;
    }
}

template <typename Processor>
juce::String GetSelectedRackNodeIdCompat(const Processor& processor)
{
    if constexpr(HasRackEditorApi<Processor>::value)
    {
        return processor.GetSelectedRackNodeId();
    }

    juce::ignoreUnused(processor);
    return "node0";
}

template <typename Processor>
bool SetSelectedRackNodeIdCompat(Processor& processor, const std::string& nodeId)
{
    if constexpr(HasRackEditorApi<Processor>::value)
    {
        return processor.SetSelectedRackNodeId(nodeId);
    }

    juce::ignoreUnused(processor);
    return nodeId == "node0";
}

template <typename Processor>
juce::String GetRackNodeIdCompat(const Processor& processor, std::size_t index)
{
    if constexpr(HasRackEditorApi<Processor>::value)
    {
        return processor.GetRackNodeId(index);
    }

    juce::ignoreUnused(processor);
    return index == 0 ? "node0" : "node1";
}

template <typename Processor>
juce::String GetRackNodeAppIdCompat(const Processor& processor, std::size_t index)
{
    if constexpr(HasRackEditorApi<Processor>::value)
    {
        return processor.GetRackNodeAppId(index);
    }

    if(index == 0)
    {
        return processor.GetActiveAppId();
    }

    return daisyhost::GetDefaultHostedAppId();
}

template <typename Processor>
juce::String GetRackNodeAppDisplayNameCompat(const Processor& processor,
                                             std::size_t      index)
{
    if constexpr(HasRackEditorApi<Processor>::value)
    {
        return processor.GetRackNodeAppDisplayName(index);
    }

    if(index == 0)
    {
        return processor.GetActiveAppDisplayName();
    }

    return GetHostedAppDisplayNameForId(
        GetRackNodeAppIdCompat(processor, index).toStdString());
}

template <typename Processor>
bool SetRackNodeAppIdCompat(Processor&               processor,
                            std::size_t              index,
                            const std::string& requestedAppId)
{
    if constexpr(HasRackEditorApi<Processor>::value)
    {
        return processor.SetRackNodeAppId(index, requestedAppId);
    }

    return index == 0 && processor.SetActiveAppId(requestedAppId);
}

template <typename Processor>
int GetRackTopologyPresetCompat(const Processor& processor)
{
    if constexpr(HasRackEditorApi<Processor>::value)
    {
        return processor.GetRackTopologyPreset();
    }
    else
    {
        juce::ignoreUnused(processor);
        return kRackTopologyNode0Only;
    }
}

template <typename Processor>
void SetRackTopologyPresetCompat(Processor& processor, int preset)
{
    if constexpr(HasRackEditorApi<Processor>::value)
    {
        processor.SetRackTopologyPreset(preset);
        return;
    }

    juce::ignoreUnused(processor, preset);
}

template <typename Processor>
juce::String GetRackNodeRoleLabelCompat(const Processor& processor,
                                        std::size_t      index)
{
    if constexpr(HasRackEditorApi<Processor>::value)
    {
        return processor.GetRackNodeRoleLabel(index);
    }
    else
    {
        juce::ignoreUnused(processor);
        return index == 0 ? "Entry" : "Inactive";
    }
}

bool HasSerialRackTopology(int preset)
{
    return preset == kRackTopologyNode0ToNode1 || preset == kRackTopologyNode1ToNode0;
}

void DrawDaisyHostBadge(juce::Graphics& g,
                        const juce::Rectangle<float>& area,
                        bool compact)
{
    const auto center = area.getCentre();
    const float petalRadius = compact ? 7.0f : 9.0f;
    const float orbit       = compact ? 12.0f : 15.0f;

    g.setColour(CreamColour().withAlpha(0.95f));
    for(int i = 0; i < 8; ++i)
    {
        const float angle = juce::MathConstants<float>::twoPi
                            * static_cast<float>(i) / 8.0f;
        g.fillEllipse(center.x + std::cos(angle) * orbit - petalRadius,
                      center.y + std::sin(angle) * orbit - petalRadius,
                      petalRadius * 2.0f,
                      petalRadius * 2.0f);
    }

    g.setColour(AccentColour());
    g.fillEllipse(center.x - (compact ? 8.0f : 10.0f),
                  center.y - (compact ? 8.0f : 10.0f),
                  compact ? 16.0f : 20.0f,
                  compact ? 16.0f : 20.0f);

    auto iconArea     = area;
    const auto module = iconArea.removeFromBottom(compact ? 18.0f : 28.0f)
                            .reduced(compact ? 12.0f : 20.0f, 0.0f);
    g.setColour(InkColour().withAlpha(0.90f));
    g.fillRoundedRectangle(module, 6.0f);
    g.setColour(CreamColour().withAlpha(0.75f));
    g.drawRoundedRectangle(module, 6.0f, 1.2f);
    g.drawLine(module.getX() + 10.0f,
               module.getCentreY(),
               module.getRight() - 10.0f,
               module.getCentreY(),
               1.2f);
}
} // namespace

DaisyHostPatchAudioProcessorEditor::DaisyHostPatchAudioProcessorEditor(
    DaisyHostPatchAudioProcessor& processor)
: AudioProcessorEditor(&processor),
  processor_(processor),
  midiKeyboard_(processor_.GetVirtualKeyboardState(),
                juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setOpaque(true);
    setWantsKeyboardFocus(true);
    setSize(1280, 840);
    addKeyListener(this);

    ConfigureControl(dryWet_,
                     processor_.GetTopControlLabel(0),
                     processor_.GetTopControlId(0));
    ConfigureMouseFriendlySlider(dryWet_.slider, 0.5);
    addAndMakeVisible(dryWet_.slider);
    addAndMakeVisible(dryWet_.label);
    addAndMakeVisible(dryWet_.learnButton);
    RegisterKeyboardSource(dryWet_.slider);
    RegisterKeyboardSource(dryWet_.learnButton);

    ConfigureControl(knobs_[0],
                     processor_.GetTopControlLabel(1),
                     processor_.GetTopControlId(1));
    ConfigureControl(knobs_[1],
                     processor_.GetTopControlLabel(2),
                     processor_.GetTopControlId(2));
    ConfigureControl(knobs_[2],
                     processor_.GetTopControlLabel(3),
                     processor_.GetTopControlId(3));
    ConfigureControl(knobs_[3],
                     "UNUSED",
                     "");

    for(auto& control : knobs_)
    {
        ConfigureMouseFriendlySlider(control.slider, 0.0);
        addAndMakeVisible(control.slider);
        addAndMakeVisible(control.label);
        addAndMakeVisible(control.learnButton);
        RegisterKeyboardSource(control.slider);
        RegisterKeyboardSource(control.learnButton);
    }

    knobs_[3].slider.setVisible(false);
    knobs_[3].label.setVisible(false);
    knobs_[3].learnButton.setVisible(false);

    encoderPressButton_.setButtonText("PUSH");
    encoderPressButton_.setClickingTogglesState(false);
    encoderPressButton_.addListener(this);
    encoderPressButton_.setColour(juce::TextButton::buttonColourId,
                                  InkColour().withAlpha(0.82f));
    encoderPressButton_.setColour(juce::TextButton::textColourOffId, CreamColour());
    encoderPressButton_.setColour(juce::TextButton::buttonOnColourId,
                                  AccentColour().darker(0.25f));
    addAndMakeVisible(encoderPressButton_);
    RegisterKeyboardSource(encoderPressButton_);

    for(std::size_t i = 0; i < cvSliders_.size(); ++i)
    {
        cvSliders_[i].setSliderStyle(juce::Slider::LinearVertical);
        cvSliders_[i].setTextBoxStyle(
            juce::Slider::TextBoxBelow, false, 48, 18);
        cvSliders_[i].setRange(0.0, 5.0, 0.01);
        cvSliders_[i].setValue(2.5);
        cvSliders_[i].addListener(this);
        ConfigureMouseFriendlySlider(cvSliders_[i], 2.5);
        cvSliders_[i].setColour(juce::Slider::thumbColourId,
                                PortColour(daisyhost::VirtualPortType::kCv));
        cvSliders_[i].setColour(juce::Slider::trackColourId,
                                PortColour(daisyhost::VirtualPortType::kCv).withAlpha(0.55f));
        cvLabels_[i].setText("CV " + juce::String(static_cast<int>(i + 1)),
                             juce::dontSendNotification);
        cvLabels_[i].setJustificationType(juce::Justification::centred);
        cvLabels_[i].setColour(juce::Label::textColourId, TextColour());
        addAndMakeVisible(cvSliders_[i]);
        addAndMakeVisible(cvLabels_[i]);
        RegisterKeyboardSource(cvSliders_[i]);
    }

    for(std::size_t i = 0; i < gateButtons_.size(); ++i)
    {
        gateButtons_[i].setButtonText(
            "Gate " + juce::String(static_cast<int>(i + 1)));
        gateButtons_[i].setClickingTogglesState(true);
        gateButtons_[i].addListener(this);
        gateButtons_[i].setColour(juce::ToggleButton::textColourId, TextColour());
    addAndMakeVisible(gateButtons_[i]);
    RegisterKeyboardSource(gateButtons_[i]);
    }

    rackHeaderLabel_.setText("Visible Rack", juce::dontSendNotification);
    rackHeaderLabel_.setJustificationType(juce::Justification::centredLeft);
    rackHeaderLabel_.setColour(juce::Label::textColourId, TextColour());
    addAndMakeVisible(rackHeaderLabel_);

    rackSelectedNodeLabel_.setText("", juce::dontSendNotification);
    rackSelectedNodeLabel_.setJustificationType(juce::Justification::centredLeft);
    rackSelectedNodeLabel_.setColour(juce::Label::textColourId,
                                     TextColour().withAlpha(0.72f));
    addAndMakeVisible(rackSelectedNodeLabel_);

    rackTopologyLabel_.setText("Topology", juce::dontSendNotification);
    rackTopologyLabel_.setJustificationType(juce::Justification::centredLeft);
    rackTopologyLabel_.setColour(juce::Label::textColourId, TextColour());
    addAndMakeVisible(rackTopologyLabel_);

    {
        int itemId = 1;
        for(const auto& registration : daisyhost::GetHostedAppRegistrations())
        {
            availableAppIds_.push_back(registration.appId);
            for(auto& rackNode : rackNodes_)
            {
                rackNode.appSelectorBox.addItem(registration.displayName, itemId);
            }
            ++itemId;
        }
    }

    for(std::size_t i = 0; i < rackNodes_.size(); ++i)
    {
        auto& rackNode = rackNodes_[i];
        rackNode.nodeIdLabel.setText("node" + juce::String(static_cast<int>(i)),
                                     juce::dontSendNotification);
        rackNode.nodeIdLabel.setJustificationType(juce::Justification::centredLeft);
        rackNode.nodeIdLabel.setColour(juce::Label::textColourId, TextColour());
        addAndMakeVisible(rackNode.nodeIdLabel);

        rackNode.roleLabel.setText("Inactive", juce::dontSendNotification);
        rackNode.roleLabel.setJustificationType(juce::Justification::centredRight);
        rackNode.roleLabel.setColour(juce::Label::textColourId,
                                     TextColour().withAlpha(0.72f));
        addAndMakeVisible(rackNode.roleLabel);

        rackNode.appSelectorBox.onChange = [this, i]() {
            const int selected = rackNodes_[i].appSelectorBox.getSelectedId();
            if(selected <= 0
               || static_cast<std::size_t>(selected - 1) >= availableAppIds_.size())
            {
                return;
            }

            if(SetRackNodeAppIdCompat(
                   processor_,
                   i,
                   availableAppIds_[static_cast<std::size_t>(selected - 1)]))
            {
                UpdateRackUi();
                UpdateTopControlUi();
                resized();
            }
        };
        addAndMakeVisible(rackNode.appSelectorBox);
        RegisterKeyboardSource(rackNode.appSelectorBox);
    }

    rackTopologyBox_.addItem(GetRackTopologyLabel(kRackTopologyNode0Only), 1);
    rackTopologyBox_.addItem(GetRackTopologyLabel(kRackTopologyNode1Only), 2);
    rackTopologyBox_.addItem(GetRackTopologyLabel(kRackTopologyNode0ToNode1), 3);
    rackTopologyBox_.addItem(GetRackTopologyLabel(kRackTopologyNode1ToNode0), 4);
    rackTopologyBox_.onChange = [this]() {
        SetRackTopologyPresetCompat(processor_, rackTopologyBox_.getSelectedId() - 1);
        UpdateRackUi();
        resized();
    };
    addAndMakeVisible(rackTopologyBox_);
    RegisterKeyboardSource(rackTopologyBox_);

    testInputModeLabel_.setText("Audio In 1 Source", juce::dontSendNotification);
    testInputModeLabel_.setJustificationType(juce::Justification::centredLeft);
    testInputModeLabel_.setColour(juce::Label::textColourId, TextColour());
    addAndMakeVisible(testInputModeLabel_);

    const std::array<int, 7> audioInputModeOrder = {
        {DaisyHostPatchAudioProcessor::kHostInput,
         DaisyHostPatchAudioProcessor::kSineInput,
         DaisyHostPatchAudioProcessor::kTriangleInput,
         DaisyHostPatchAudioProcessor::kSquareInput,
         DaisyHostPatchAudioProcessor::kSawInput,
         DaisyHostPatchAudioProcessor::kNoiseInput,
         DaisyHostPatchAudioProcessor::kImpulseInput}};
    for(const int mode : audioInputModeOrder)
    {
        testInputModeBox_.addItem(processor_.GetTestInputModeName(mode), mode + 1);
    }
    testInputModeBox_.onChange = [this]() {
        processor_.SetTestInputMode(testInputModeBox_.getSelectedId() - 1);
    };
    addAndMakeVisible(testInputModeBox_);
    RegisterKeyboardSource(testInputModeBox_);

    testInputLevelLabel_.setText("Audio In 1 Level (+/-10V)",
                                 juce::dontSendNotification);
    testInputLevelLabel_.setJustificationType(juce::Justification::centredLeft);
    testInputLevelLabel_.setColour(juce::Label::textColourId, TextColour());
    addAndMakeVisible(testInputLevelLabel_);

    testInputLevelSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    testInputLevelSlider_.setTextBoxStyle(
        juce::Slider::TextBoxRight, false, 54, 18);
    testInputLevelSlider_.setRange(0.0, 10.0, 0.01);
    testInputLevelSlider_.addListener(this);
    ConfigureMouseFriendlySlider(testInputLevelSlider_, 3.5);
    testInputLevelSlider_.setColour(juce::Slider::thumbColourId, AccentColour());
    addAndMakeVisible(testInputLevelSlider_);
    RegisterKeyboardSource(testInputLevelSlider_);

    testInputFrequencyLabel_.setText("Audio In 1 Freq (Hz)",
                                     juce::dontSendNotification);
    testInputFrequencyLabel_.setJustificationType(
        juce::Justification::centredLeft);
    testInputFrequencyLabel_.setColour(juce::Label::textColourId, TextColour());
    addAndMakeVisible(testInputFrequencyLabel_);

    testInputFrequencySlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    testInputFrequencySlider_.setTextBoxStyle(
        juce::Slider::TextBoxRight, false, 62, 18);
    testInputFrequencySlider_.setRange(20.0, 5000.0, 1.0);
    testInputFrequencySlider_.setSkewFactorFromMidPoint(440.0);
    testInputFrequencySlider_.addListener(this);
    ConfigureMouseFriendlySlider(testInputFrequencySlider_, 220.0);
    testInputFrequencySlider_.setColour(juce::Slider::thumbColourId,
                                        AccentColour());
    addAndMakeVisible(testInputFrequencySlider_);
    RegisterKeyboardSource(testInputFrequencySlider_);

    triggerImpulseButton_.setButtonText("Fire Impulse");
    triggerImpulseButton_.addListener(this);
    triggerImpulseButton_.setColour(juce::TextButton::buttonColourId,
                                    InkColour().withAlpha(0.70f));
    addAndMakeVisible(triggerImpulseButton_);
    RegisterKeyboardSource(triggerImpulseButton_);

    cvGeneratorLabel_.setText("CV Generators", juce::dontSendNotification);
    cvGeneratorLabel_.setJustificationType(juce::Justification::centredLeft);
    cvGeneratorLabel_.setColour(juce::Label::textColourId, TextColour());
    addAndMakeVisible(cvGeneratorLabel_);

    for(std::size_t i = 0; i < cvGeneratorTitles_.size(); ++i)
    {
        cvGeneratorTitles_[i].setText("CV " + juce::String(static_cast<int>(i + 1)),
                                      juce::dontSendNotification);
        cvGeneratorTitles_[i].setJustificationType(
            juce::Justification::centredLeft);
        cvGeneratorTitles_[i].setColour(juce::Label::textColourId, TextColour());
        addAndMakeVisible(cvGeneratorTitles_[i]);

        cvGeneratorModeBoxes_[i].addItem("Manual", 1);
        cvGeneratorModeBoxes_[i].addItem("Generator", 2);
        cvGeneratorModeBoxes_[i].onChange = [this, i]() {
            processor_.SetCvSourceMode(i, cvGeneratorModeBoxes_[i].getSelectedId() - 1);
            UpdateCvGeneratorEditorUi();
        };
        addAndMakeVisible(cvGeneratorModeBoxes_[i]);
        RegisterKeyboardSource(cvGeneratorModeBoxes_[i]);

        for(int waveform = 0;
            waveform <= static_cast<int>(daisyhost::BasicWaveform::kSaw);
            ++waveform)
        {
            cvGeneratorWaveformBoxes_[i].addItem(
                daisyhost::GetBasicWaveformName(waveform), waveform + 1);
        }
        cvGeneratorWaveformBoxes_[i].onChange = [this, i]() {
            processor_.SetCvWaveform(
                i, cvGeneratorWaveformBoxes_[i].getSelectedId() - 1);
        };
        addAndMakeVisible(cvGeneratorWaveformBoxes_[i]);
        RegisterKeyboardSource(cvGeneratorWaveformBoxes_[i]);

        auto configureCvSlider = [this](juce::Slider& slider, double defaultValue) {
            slider.setSliderStyle(juce::Slider::LinearHorizontal);
            slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 18);
            slider.addListener(this);
            ConfigureMouseFriendlySlider(slider, defaultValue);
            slider.setColour(juce::Slider::thumbColourId,
                             PortColour(daisyhost::VirtualPortType::kCv));
            addAndMakeVisible(slider);
            RegisterKeyboardSource(slider);
        };

        cvGeneratorFrequencySliders_[i].setRange(0.01, 20.0, 0.01);
        cvGeneratorFrequencySliders_[i].setSkewFactorFromMidPoint(1.0);
        cvGeneratorFrequencySliders_[i].setTextValueSuffix(" Hz");
        configureCvSlider(cvGeneratorFrequencySliders_[i], 1.0);

        cvGeneratorAmplitudeSliders_[i].setRange(0.0, 2.5, 0.01);
        cvGeneratorAmplitudeSliders_[i].setTextValueSuffix(" V");
        configureCvSlider(cvGeneratorAmplitudeSliders_[i], 2.5);

        cvGeneratorBiasSliders_[i].setRange(0.0, 5.0, 0.01);
        cvGeneratorBiasSliders_[i].setTextValueSuffix(" V");
        configureCvSlider(cvGeneratorBiasSliders_[i], 2.5);
    }

    computerKeyboardLabel_.setText("Keyboard MIDI", juce::dontSendNotification);
    computerKeyboardLabel_.setJustificationType(juce::Justification::centredLeft);
    computerKeyboardLabel_.setColour(juce::Label::textColourId, TextColour());
    addAndMakeVisible(computerKeyboardLabel_);

    computerKeyboardToggle_.setButtonText("Enable A/W/S/E");
    computerKeyboardToggle_.setClickingTogglesState(true);
    computerKeyboardToggle_.addListener(this);
    computerKeyboardToggle_.setColour(juce::ToggleButton::textColourId, TextColour());
    addAndMakeVisible(computerKeyboardToggle_);
    RegisterKeyboardSource(computerKeyboardToggle_);

    computerKeyboardOctaveLabel_.setText("Octave", juce::dontSendNotification);
    computerKeyboardOctaveLabel_.setJustificationType(
        juce::Justification::centredLeft);
    computerKeyboardOctaveLabel_.setColour(juce::Label::textColourId, TextColour());
    addAndMakeVisible(computerKeyboardOctaveLabel_);

    for(int octave = daisyhost::ComputerKeyboardMidi::kMinOctave;
        octave <= daisyhost::ComputerKeyboardMidi::kMaxOctave;
        ++octave)
    {
        computerKeyboardOctaveBox_.addItem(
            "C" + juce::String(octave), octave + 1);
    }
    computerKeyboardOctaveBox_.onChange = [this]() {
        processor_.SetComputerKeyboardOctave(
            computerKeyboardOctaveBox_.getSelectedId() - 1);
        UpdateComputerKeyboardUi();
    };
    addAndMakeVisible(computerKeyboardOctaveBox_);
    RegisterKeyboardSource(computerKeyboardOctaveBox_);

    midiKeyboard_.setAvailableRange(24, 96);
    midiKeyboard_.setKeyWidth(14.0f);
    midiKeyboard_.setScrollButtonsVisible(false);
    addAndMakeVisible(midiKeyboard_);
    RegisterKeyboardSource(midiKeyboard_);

    midiTrackerLabel_.setText("MIDI Tracker", juce::dontSendNotification);
    midiTrackerLabel_.setJustificationType(juce::Justification::centredLeft);
    midiTrackerLabel_.setColour(juce::Label::textColourId, TextColour());
    addAndMakeVisible(midiTrackerLabel_);

    midiTrackerStatusLabel_.setText("", juce::dontSendNotification);
    midiTrackerStatusLabel_.setJustificationType(
        juce::Justification::centredLeft);
    midiTrackerStatusLabel_.setColour(juce::Label::textColourId,
                                      TextColour().withAlpha(0.75f));
    addAndMakeVisible(midiTrackerStatusLabel_);

    midiTrackerText_.setMultiLine(true);
    midiTrackerText_.setReadOnly(true);
    midiTrackerText_.setScrollbarsShown(true);
    midiTrackerText_.setCaretVisible(false);
    midiTrackerText_.setColour(juce::TextEditor::backgroundColourId,
                               DrawerInsetColour());
    midiTrackerText_.setColour(juce::TextEditor::outlineColourId,
                               juce::Colours::white.withAlpha(0.10f));
    midiTrackerText_.setColour(juce::TextEditor::textColourId, TextColour());
    addAndMakeVisible(midiTrackerText_);

    UpdateRackUi();
    UpdateTopControlUi();
    UpdateComputerKeyboardUi();
    UpdateCvGeneratorEditorUi();
    startTimerHz(30);
}

DaisyHostPatchAudioProcessorEditor::~DaisyHostPatchAudioProcessorEditor()
{
    stopTimer();
    ReleaseComputerKeyboardNotes();

    for(auto& control : knobs_)
    {
        control.slider.removeListener(this);
        control.learnButton.removeListener(this);
    }
    dryWet_.slider.removeListener(this);
    dryWet_.learnButton.removeListener(this);
    encoderPressButton_.removeListener(this);

    for(auto& slider : cvSliders_)
    {
        slider.removeListener(this);
    }

    for(auto& button : gateButtons_)
    {
        button.removeListener(this);
    }

    testInputLevelSlider_.removeListener(this);
    testInputFrequencySlider_.removeListener(this);
    for(std::size_t i = 0; i < cvGeneratorFrequencySliders_.size(); ++i)
    {
        cvGeneratorFrequencySliders_[i].removeListener(this);
        cvGeneratorAmplitudeSliders_[i].removeListener(this);
        cvGeneratorBiasSliders_[i].removeListener(this);
    }
    triggerImpulseButton_.removeListener(this);
    computerKeyboardToggle_.removeListener(this);
}

void DaisyHostPatchAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(BackgroundColour());

    const auto patchPanel = GetPatchPanelBounds().toFloat();
    const auto hostTools  = GetHostToolsBounds().toFloat();

    juce::ColourGradient panelGradient(PatchPanelColour(),
                                       patchPanel.getTopLeft(),
                                       PatchPanelShadow(),
                                       patchPanel.getBottomRight(),
                                       false);
    g.setGradientFill(panelGradient);
    g.fillRoundedRectangle(patchPanel, 26.0f);
    g.setColour(juce::Colours::white.withAlpha(0.14f));
    g.drawRoundedRectangle(patchPanel.reduced(1.0f), 26.0f, 1.6f);

    juce::ColourGradient drawerGradient(DrawerColour(),
                                        hostTools.getTopLeft(),
                                        DrawerInsetColour(),
                                        hostTools.getBottomRight(),
                                        false);
    g.setGradientFill(drawerGradient);
    g.fillRoundedRectangle(hostTools, 22.0f);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawRoundedRectangle(hostTools.reduced(1.0f), 22.0f, 1.2f);

    DrawPanelDecorations(g);
    DrawPatchTraces(g);
    DrawDisplay(g, RectFromPanel(processor_.GetBoardProfile().display.panelBounds));
    DrawPorts(g);
    DrawPanelTexts(g);
    DrawHostTools(g);

    g.setColour(TextColour().withAlpha(0.55f));
    g.setFont(BodyFont(11.5f));
    g.drawText(
        "Standalone note: if JUCE mutes live input to avoid feedback, switch the input source in the mirrored host drawer.",
        juce::Rectangle<int>(GetEditorBounds().getX(),
                             GetEditorBounds().getBottom() - 18,
                             GetEditorBounds().getWidth(),
                             16),
        juce::Justification::centred,
        true);
}

void DaisyHostPatchAudioProcessorEditor::resized()
{
    const auto& profile = processor_.GetBoardProfile();
    const auto* ctrl1   = FindSurfaceControlById(profile.nodeId + "/surface/ctrl1_mix");
    const auto* ctrl2   = FindSurfaceControlById(profile.nodeId + "/surface/ctrl2_primary");
    const auto* ctrl3   = FindSurfaceControlById(profile.nodeId + "/surface/ctrl3_secondary");
    const auto* ctrl4   = FindSurfaceControlById(profile.nodeId + "/surface/ctrl4_feedback");
    const auto* push    = FindSurfaceControlById(profile.nodeId + "/surface/enc1_push");

    if(ctrl1 != nullptr)
    {
        LayoutRotaryControl(dryWet_, *ctrl1);
    }
    if(ctrl2 != nullptr)
    {
        LayoutRotaryControl(knobs_[0], *ctrl2);
    }
    if(ctrl3 != nullptr)
    {
        LayoutRotaryControl(knobs_[1], *ctrl3);
    }
    if(ctrl4 != nullptr)
    {
        LayoutRotaryControl(knobs_[2], *ctrl4);
    }
    if(push != nullptr)
    {
        LayoutButtonControl(encoderPressButton_, *push);
    }

    auto drawer = GetHostToolsBounds().reduced(18);
    rackHeaderLabel_.setBounds(drawer.removeFromTop(20));
    rackSelectedNodeLabel_.setBounds(drawer.removeFromTop(18));
    drawer.removeFromTop(8);

    auto rackCardsArea = drawer.removeFromTop(92);
    const int rackCardGap = 12;
    const int rackCardWidth = (rackCardsArea.getWidth() - rackCardGap) / 2;
    for(std::size_t i = 0; i < rackNodes_.size(); ++i)
    {
        auto card = juce::Rectangle<int>(rackCardsArea.getX()
                                             + static_cast<int>(i) * (rackCardWidth + rackCardGap),
                                         rackCardsArea.getY(),
                                         rackCardWidth,
                                         rackCardsArea.getHeight());
        rackNodeCardBounds_[i] = card;

        auto content = card.reduced(12, 10);
        auto topRow  = content.removeFromTop(18);
        rackNodes_[i].nodeIdLabel.setBounds(topRow.removeFromLeft(topRow.getWidth() / 2));
        rackNodes_[i].roleLabel.setBounds(topRow);
        content.removeFromTop(8);
        rackNodes_[i].appSelectorBox.setBounds(content.removeFromTop(28));
    }

    drawer.removeFromTop(8);
    rackTopologyLabel_.setBounds(drawer.removeFromTop(18));
    rackTopologyBox_.setBounds(drawer.removeFromTop(28));
    drawer.removeFromTop(10);

    knobs_[3].slider.setBounds({});
    knobs_[3].label.setBounds({});
    knobs_[3].learnButton.setBounds({});

    testInputModeLabel_.setBounds(drawer.removeFromTop(18));
    testInputModeBox_.setBounds(drawer.removeFromTop(28));
    drawer.removeFromTop(6);
    testInputLevelLabel_.setBounds(drawer.removeFromTop(18));
    testInputLevelSlider_.setBounds(drawer.removeFromTop(28));
    drawer.removeFromTop(6);
    testInputFrequencyLabel_.setBounds(drawer.removeFromTop(18));
    testInputFrequencySlider_.setBounds(drawer.removeFromTop(28));
    drawer.removeFromTop(6);
    triggerImpulseButton_.setBounds(drawer.removeFromTop(28));
    drawer.removeFromTop(10);

    auto lowerArea = drawer;
    auto leftColumn
        = lowerArea.removeFromLeft((lowerArea.getWidth() - 10) / 2);
    lowerArea.removeFromLeft(10);
    auto rightColumn = lowerArea;

    computerKeyboardLabel_.setBounds(leftColumn.removeFromTop(18));
    computerKeyboardToggle_.setBounds(leftColumn.removeFromTop(24));
    leftColumn.removeFromTop(4);
    computerKeyboardOctaveLabel_.setBounds(leftColumn.removeFromTop(18));
    computerKeyboardOctaveBox_.setBounds(leftColumn.removeFromTop(28));
    leftColumn.removeFromTop(6);
    midiKeyboard_.setBounds(leftColumn.removeFromTop(72));
    leftColumn.removeFromTop(10);

    midiTrackerLabel_.setBounds(leftColumn.removeFromTop(18));
    midiTrackerStatusLabel_.setBounds(leftColumn.removeFromTop(18));
    midiTrackerText_.setBounds(leftColumn.removeFromTop(110));

    cvGeneratorLabel_.setBounds(rightColumn.removeFromTop(18));
    rightColumn.removeFromTop(4);

    auto generatorGrid = rightColumn.removeFromTop(314);
    const int cardGap = 8;
    const int cardWidth = (generatorGrid.getWidth() - cardGap) / 2;
    const int cardHeight = (generatorGrid.getHeight() - cardGap) / 2;
    for(std::size_t i = 0; i < cvGeneratorTitles_.size(); ++i)
    {
        const int row = static_cast<int>(i / 2);
        const int col = static_cast<int>(i % 2);
        auto card = juce::Rectangle<int>(generatorGrid.getX() + col * (cardWidth + cardGap),
                                         generatorGrid.getY() + row * (cardHeight + cardGap),
                                         cardWidth,
                                         cardHeight);

        cvGeneratorTitles_[i].setBounds(card.removeFromTop(18));
        cvGeneratorModeBoxes_[i].setBounds(card.removeFromTop(24));
        card.removeFromTop(4);
        cvGeneratorWaveformBoxes_[i].setBounds(card.removeFromTop(24));
        card.removeFromTop(4);
        cvGeneratorFrequencySliders_[i].setBounds(card.removeFromTop(22));
        card.removeFromTop(2);
        cvGeneratorAmplitudeSliders_[i].setBounds(card.removeFromTop(22));
        card.removeFromTop(2);
        cvGeneratorBiasSliders_[i].setBounds(card.removeFromTop(22));
    }

    rightColumn.removeFromTop(8);
    auto cvArea = rightColumn.removeFromTop(118);
    const int cvWidth = (cvArea.getWidth() - 18) / 4;
    for(std::size_t i = 0; i < cvSliders_.size(); ++i)
    {
        auto column = cvArea.removeFromLeft(cvWidth);
        cvLabels_[i].setBounds(column.removeFromTop(18));
        cvSliders_[i].setBounds(column.withTrimmedTop(4));
        cvArea.removeFromLeft(6);
    }

    rightColumn.removeFromTop(8);
    auto gateArea = rightColumn.removeFromTop(28);
    gateButtons_[0].setBounds(gateArea.removeFromLeft(gateArea.getWidth() / 2 - 6));
    gateArea.removeFromLeft(12);
    gateButtons_[1].setBounds(gateArea);
}

void DaisyHostPatchAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    grabKeyboardFocus();

    for(std::size_t i = 0; i < rackNodeCardBounds_.size(); ++i)
    {
        if(rackNodeCardBounds_[i].contains(event.getPosition()))
        {
            if(SetSelectedRackNodeIdCompat(
                   processor_,
                   GetRackNodeIdCompat(processor_, i).toStdString()))
            {
                UpdateRackUi();
                UpdateTopControlUi();
                resized();
            }
            return;
        }
    }

    const auto& profile = processor_.GetBoardProfile();
    if(const auto* encoder = FindSurfaceControlById(profile.nodeId + "/surface/enc1"))
    {
        if(RectFromPanel(encoder->panelBounds).contains(event.position))
        {
            processor_.PulseEncoderPress();
            encoderPressButton_.setToggleState(true, juce::dontSendNotification);
        }
    }
}

void DaisyHostPatchAudioProcessorEditor::mouseWheelMove(
    const juce::MouseEvent&        event,
    const juce::MouseWheelDetails& wheel)
{
    const auto& profile = processor_.GetBoardProfile();
    if(const auto* encoder = FindSurfaceControlById(profile.nodeId + "/surface/enc1"))
    {
        if(RectFromPanel(encoder->panelBounds).contains(event.position))
        {
            const float deltaY = wheel.deltaY != 0.0f ? wheel.deltaY : wheel.deltaX;
            if(std::abs(deltaY) > 0.0f)
            {
                processor_.RotateEncoder(deltaY > 0.0f ? -1 : 1);
            }
        }
    }
}

juce::Rectangle<int> DaisyHostPatchAudioProcessorEditor::GetEditorBounds() const
{
    return getLocalBounds().reduced(18);
}

juce::Rectangle<int> DaisyHostPatchAudioProcessorEditor::GetPatchPanelBounds() const
{
    auto bounds = GetEditorBounds();
    auto left = bounds.removeFromLeft(static_cast<int>(bounds.getWidth() * 0.64f));
    return left.reduced(8, 30);
}

juce::Rectangle<int> DaisyHostPatchAudioProcessorEditor::GetPatchPanelContentBounds() const
{
    return GetPatchPanelBounds().reduced(26);
}

juce::Rectangle<int> DaisyHostPatchAudioProcessorEditor::GetHostToolsBounds() const
{
    auto bounds = GetEditorBounds();
    bounds.removeFromLeft(static_cast<int>(bounds.getWidth() * 0.64f) + 12);
    return bounds;
}

juce::Rectangle<float> DaisyHostPatchAudioProcessorEditor::RectFromPanel(
    const daisyhost::PanelRect& rect) const
{
    const auto panel = GetPatchPanelBounds().toFloat();
    return {panel.getX() + rect.x * panel.getWidth(),
            panel.getY() + rect.y * panel.getHeight(),
            rect.width * panel.getWidth(),
            rect.height * panel.getHeight()};
}

const daisyhost::PanelControlSlotSpec*
DaisyHostPatchAudioProcessorEditor::FindSurfaceControlByTarget(
    const std::string& targetId) const
{
    const auto& controls = processor_.GetBoardProfile().surfaceControls;
    for(const auto& control : controls)
    {
        if(control.targetId == targetId)
        {
            return &control;
        }
    }
    return nullptr;
}

const daisyhost::PanelControlSlotSpec*
DaisyHostPatchAudioProcessorEditor::FindSurfaceControlById(
    const std::string& id) const
{
    const auto& controls = processor_.GetBoardProfile().surfaceControls;
    for(const auto& control : controls)
    {
        if(control.id == id)
        {
            return &control;
        }
    }
    return nullptr;
}

juce::Justification DaisyHostPatchAudioProcessorEditor::ToJustification(
    daisyhost::TextAlignment alignment) const
{
    switch(alignment)
    {
        case daisyhost::TextAlignment::kCenter:
            return juce::Justification::centred;
        case daisyhost::TextAlignment::kRight:
            return juce::Justification::centredRight;
        case daisyhost::TextAlignment::kLeft:
        default: return juce::Justification::centredLeft;
    }
}

void DaisyHostPatchAudioProcessorEditor::DrawPanelDecorations(juce::Graphics& g) const
{
    const auto& profile = processor_.GetBoardProfile();
    for(const auto& decoration : profile.decorations)
    {
        const auto area = RectFromPanel(decoration.panelBounds);
        switch(decoration.kind)
        {
            case daisyhost::PanelDecorationKind::kSeedModule:
                DrawSeedModule(g, area);
                break;

            case daisyhost::PanelDecorationKind::kDisplayFrame:
                g.setColour(juce::Colours::black.withAlpha(0.20f));
                g.fillRoundedRectangle(area, 14.0f);
                g.setColour(juce::Colours::white.withAlpha(0.16f));
                g.drawRoundedRectangle(area, 14.0f, 1.4f);
                break;

            default:
                g.setColour(juce::Colours::white.withAlpha(
                    decoration.emphasized ? 0.22f : 0.13f));
                g.drawRoundedRectangle(area,
                                       area.getHeight() * decoration.cornerRadius,
                                       decoration.emphasized ? 1.8f : 1.2f);
                break;
        }
    }
}

void DaisyHostPatchAudioProcessorEditor::DrawPanelTexts(juce::Graphics& g) const
{
    const auto& profile = processor_.GetBoardProfile();

    for(const auto& text : profile.texts)
    {
        g.setColour(TextColour().withAlpha(text.bold ? 0.98f : 0.88f));
        g.setFont(text.bold ? TitleFont(text.pointSize) : BodyFont(text.pointSize));
        g.drawText(text.text,
                   RectFromPanel(text.panelBounds).toNearestInt(),
                   ToJustification(text.alignment),
                   false);
    }

    for(const auto& control : profile.surfaceControls)
    {
        if(control.detailLabel.empty())
        {
            continue;
        }

        const auto slot = RectFromPanel(control.panelBounds);
        juce::Rectangle<float> detailArea(slot.getX() - 8.0f,
                                          slot.getBottom() + 34.0f,
                                          slot.getWidth() + 16.0f,
                                          13.0f);
        if(control.kind == daisyhost::ControlKind::kButton)
        {
            detailArea = detailArea.translated(0.0f, 12.0f);
        }

        juce::String detailText = control.detailLabel;
        if(control.id == profile.nodeId + "/surface/ctrl1_mix")
        {
            detailText = processor_.GetTopControlDetailLabel(0);
        }
        else if(control.id == profile.nodeId + "/surface/ctrl2_primary")
        {
            detailText = processor_.GetTopControlDetailLabel(1);
        }
        else if(control.id == profile.nodeId + "/surface/ctrl3_secondary")
        {
            detailText = processor_.GetTopControlDetailLabel(2);
        }
        else if(control.id == profile.nodeId + "/surface/ctrl4_feedback")
        {
            detailText = processor_.GetTopControlDetailLabel(3);
        }

        g.setColour(CreamColour().withAlpha(0.68f));
        g.setFont(BodyFont(10.5f));
        g.drawText(detailText,
                   detailArea.toNearestInt(),
                   juce::Justification::centred,
                   false);
    }

    juce::String selectedAppDisplay = processor_.GetActiveAppDisplayName();
    const auto selectedNodeId       = GetSelectedRackNodeIdCompat(processor_);
    const auto rackNodeCount = std::min(GetRackNodeCountCompat(processor_),
                                        rackNodes_.size());
    for(std::size_t i = 0; i < rackNodeCount; ++i)
    {
        if(GetRackNodeIdCompat(processor_, i) == selectedNodeId)
        {
            selectedAppDisplay = GetRackNodeAppDisplayNameCompat(processor_, i);
            break;
        }
    }

    g.setColour(TextColour().withAlpha(0.86f));
    g.setFont(TitleFont(13.0f));
    g.drawText(selectedAppDisplay,
               RectFromPanel({0.47f, 0.045f, 0.20f, 0.03f}).toNearestInt(),
               juce::Justification::centred,
               false);
}

void DaisyHostPatchAudioProcessorEditor::DrawPatchTraces(juce::Graphics& g) const
{
    const auto& profile = processor_.GetBoardProfile();
    const auto* ctrl1   = FindSurfaceControlById(profile.nodeId + "/surface/ctrl1_mix");
    const auto* ctrl2   = FindSurfaceControlById(profile.nodeId + "/surface/ctrl2_primary");
    const auto* ctrl3   = FindSurfaceControlById(profile.nodeId + "/surface/ctrl3_secondary");
    const auto* ctrl4   = FindSurfaceControlById(profile.nodeId + "/surface/ctrl4_feedback");

    const daisyhost::PanelControlSlotSpec* topSlots[] = {ctrl1, ctrl2, ctrl3, ctrl4};
    std::size_t topIndex = 0;
    for(const auto& port : profile.ports)
    {
        if(port.type != daisyhost::VirtualPortType::kCv
           || port.direction != daisyhost::PortDirection::kInput
           || topIndex >= std::size(topSlots)
           || topSlots[topIndex] == nullptr)
        {
            continue;
        }

        const auto jack = RectFromPanel(port.panelBounds);
        const auto knob = RectFromPanel(topSlots[topIndex]->panelBounds);
        juce::Path trace;
        trace.startNewSubPath(jack.getCentreX(), jack.getBottom());
        trace.cubicTo(jack.getCentreX(),
                      jack.getBottom() + 34.0f,
                      knob.getCentreX(),
                      knob.getY() - 28.0f,
                      knob.getCentreX(),
                      knob.getY());
        g.setColour(TraceColour().withAlpha(0.55f));
        g.strokePath(trace, juce::PathStrokeType(1.4f));
        ++topIndex;
    }

    const auto displayArea = RectFromPanel(profile.display.panelBounds);
    const auto audioFrame = [&]() -> juce::Rectangle<float> {
        for(const auto& decoration : profile.decorations)
        {
            if(decoration.kind == daisyhost::PanelDecorationKind::kAudioSection)
            {
                return RectFromPanel(decoration.panelBounds);
            }
        }
        return {};
    }();

    if(!audioFrame.isEmpty())
    {
        for(int i = 0; i < 5; ++i)
        {
            const float offset = static_cast<float>(i) * 9.0f;
            juce::Path trace;
            trace.startNewSubPath(displayArea.getX() + 30.0f + offset,
                                  displayArea.getBottom() - 2.0f);
            trace.cubicTo(displayArea.getX() + 48.0f + offset,
                          displayArea.getBottom() + 20.0f,
                          audioFrame.getX() + 40.0f + offset * 3.0f,
                          audioFrame.getY() - 20.0f,
                          audioFrame.getX() + 60.0f + offset * 4.0f,
                          audioFrame.getY() + 4.0f);
            g.setColour(TraceColour().withAlpha(0.42f));
            g.strokePath(trace, juce::PathStrokeType(1.2f));
        }
    }

    if(const auto* encoder = FindSurfaceControlById(profile.nodeId + "/surface/enc1"))
    {
        const auto area = RectFromPanel(encoder->panelBounds);
        g.setColour(juce::Colours::black.withAlpha(0.36f));
        g.fillEllipse(SquareInside(area, 0.92f));
        g.setColour(CreamColour().withAlpha(0.75f));
        g.drawEllipse(SquareInside(area, 0.92f), 2.0f);
        g.setColour(InkColour());
        g.fillEllipse(SquareInside(area, 0.72f));
        g.setColour(AccentColour());
        g.fillEllipse(area.getCentreX() - 5.0f, area.getY() + 10.0f, 10.0f, 10.0f);
    }
}

void DaisyHostPatchAudioProcessorEditor::DrawSeedModule(
    juce::Graphics&              g,
    const juce::Rectangle<float>& area) const
{
    g.setColour(juce::Colour::fromRGB(179, 105, 28));
    g.fillRoundedRectangle(area, 12.0f);
    g.setColour(juce::Colour::fromRGB(233, 192, 132));
    g.drawRoundedRectangle(area.reduced(1.0f), 12.0f, 1.5f);

    auto inner = area.reduced(10.0f);
    g.setColour(juce::Colour::fromRGB(88, 44, 10));
    g.fillRoundedRectangle(inner.removeFromTop(area.getHeight() * 0.58f), 6.0f);

    auto pins = area.reduced(5.0f);
    const float pinWidth  = 6.0f;
    const float pinHeight = 12.0f;
    for(int i = 0; i < 10; ++i)
    {
        const float y = pins.getY() + 10.0f + static_cast<float>(i) * 12.0f;
        g.setColour(CreamColour().withAlpha(0.85f));
        g.fillRoundedRectangle(area.getX() + 4.0f, y, pinWidth, pinHeight, 2.0f);
        g.fillRoundedRectangle(area.getRight() - 10.0f, y, pinWidth, pinHeight, 2.0f);
    }

    g.setColour(CreamColour().withAlpha(0.86f));
    g.setFont(TitleFont(11.0f));
    g.drawText("Seed",
               juce::Rectangle<int>(static_cast<int>(area.getX()),
                                    static_cast<int>(area.getBottom() - 28.0f),
                                    static_cast<int>(area.getWidth()),
                                    16),
               juce::Justification::centred);
    g.setFont(BodyFont(8.0f));
    g.drawText("DaisyHost",
               juce::Rectangle<int>(static_cast<int>(area.getX()),
                                    static_cast<int>(area.getY() + 6.0f),
                                    static_cast<int>(area.getWidth()),
                                    12),
               juce::Justification::centred);
}

void DaisyHostPatchAudioProcessorEditor::DrawDisplay(
    juce::Graphics&               g,
    const juce::Rectangle<float>& area) const
{
    const auto selectedNodeId = GetSelectedRackNodeIdCompat(processor_);
    juce::String selectedAppDisplay;
    const auto rackNodeCount = std::min(GetRackNodeCountCompat(processor_),
                                        rackNodes_.size());
    for(std::size_t i = 0; i < rackNodeCount; ++i)
    {
        if(GetRackNodeIdCompat(processor_, i) == selectedNodeId)
        {
            selectedAppDisplay = GetRackNodeAppDisplayNameCompat(processor_, i);
            break;
        }
    }
    if(selectedAppDisplay.isEmpty())
    {
        selectedAppDisplay = processor_.GetActiveAppDisplayName();
    }

    const auto displayHeaderArea = juce::Rectangle<float>(
        area.getX() - 2.0f, area.getY() - 20.0f, area.getWidth() + 4.0f, 16.0f);
    g.setColour(TextColour().withAlpha(0.88f));
    g.setFont(BodyFont(11.0f));
    g.drawText(selectedNodeId + " / " + selectedAppDisplay,
               displayHeaderArea.toNearestInt(),
               juce::Justification::centredLeft,
               true);

    g.setColour(juce::Colour::fromRGB(13, 15, 18));
    g.fillRoundedRectangle(area, 8.0f);
    g.setColour(CreamColour().withAlpha(0.65f));
    g.drawRoundedRectangle(area, 8.0f, 1.5f);

    const auto display = processor_.GetDisplayModelSnapshot();
    const auto origin  = area.reduced(10.0f);

    for(const auto& bar : display.bars)
    {
        juce::Rectangle<float> barRect(origin.getX() + static_cast<float>(bar.x),
                                       origin.getY() + static_cast<float>(bar.y),
                                       static_cast<float>(bar.width),
                                       static_cast<float>(bar.height));
        g.setColour(CreamColour().withAlpha(0.18f));
        g.drawRect(barRect);
        g.setColour(AccentColour());
        g.fillRect(barRect.withWidth(barRect.getWidth() * bar.normalized));
    }

    for(const auto& text : display.texts)
    {
        g.setColour(text.inverted ? InkColour() : CreamColour());
        g.setFont(text.y <= 4 ? TitleFont(16.0f) : BodyFont(13.0f));
        g.drawText(text.text,
                   juce::Rectangle<float>(origin.getX() + static_cast<float>(text.x),
                                          origin.getY() + static_cast<float>(text.y),
                                          area.getWidth() - 20.0f,
                                          18.0f),
                   juce::Justification::centredLeft,
                   false);
    }
}

void DaisyHostPatchAudioProcessorEditor::DrawPorts(juce::Graphics& g) const
{
    const auto& profile = processor_.GetBoardProfile();
    const auto  patch   = GetPatchPanelBounds().toFloat();

    for(const auto& port : profile.ports)
    {
        const auto bounds    = RectFromPanel(port.panelBounds);
        const auto socketBox = SquareInside(bounds, 0.92f);

        float activity = 0.0f;
        if(port.type == daisyhost::VirtualPortType::kAudio)
        {
            const auto oneBased = static_cast<std::size_t>(
                std::max(0, OneBasedIndexFromLabel(port.label)));
            const auto index = oneBased > 0 ? oneBased - 1 : 0;
            activity = (port.direction == daisyhost::PortDirection::kInput)
                           ? processor_.GetAudioInputPeak(index)
                           : processor_.GetAudioOutputPeak(index);
        }
        else if(port.type == daisyhost::VirtualPortType::kMidi)
        {
            activity = processor_.GetMidiActivity();
        }
        else if(port.type == daisyhost::VirtualPortType::kGate
                && port.direction == daisyhost::PortDirection::kInput)
        {
            const auto oneBased = static_cast<std::size_t>(
                std::max(0, OneBasedIndexFromLabel(port.label)));
            const auto index = oneBased > 0 ? oneBased - 1 : 0;
            activity = processor_.GetGateValue(index) ? 1.0f : 0.0f;
        }

        const auto colour = PortColour(port.type);
        g.setColour(juce::Colours::black.withAlpha(0.26f));
        g.fillEllipse(socketBox.expanded(6.0f));
        g.setColour(juce::Colour::fromRGB(178, 177, 176));
        g.fillEllipse(socketBox.expanded(4.0f));
        g.setColour(colour.withAlpha(0.18f + activity * 0.62f));
        g.fillEllipse(socketBox.reduced(2.0f));
        g.setColour(juce::Colour::fromRGB(58, 47, 34));
        g.fillEllipse(socketBox.reduced(socketBox.getWidth() * 0.18f));
        g.setColour(juce::Colours::white.withAlpha(0.32f));
        g.drawEllipse(socketBox.expanded(4.0f), 1.3f);

        juce::Rectangle<float> labelArea(bounds.getX() - 12.0f,
                                         bounds.getBottom() + 2.0f,
                                         bounds.getWidth() + 24.0f,
                                         15.0f);
        if(bounds.getY() < patch.getY() + patch.getHeight() * 0.20f)
        {
            labelArea = {bounds.getX() - 12.0f,
                         bounds.getBottom() + 4.0f,
                         bounds.getWidth() + 24.0f,
                         14.0f};
        }
        else if(bounds.getX() > patch.getRight() - 180.0f
                && bounds.getY() < patch.getY() + patch.getHeight() * 0.45f)
        {
            labelArea = {bounds.getX() - 32.0f,
                         bounds.getY() - 18.0f,
                         bounds.getWidth() + 64.0f,
                         14.0f};
        }

        g.setColour(TextColour());
        g.setFont(BodyFont(10.5f));
        g.drawText(port.label,
                   labelArea.toNearestInt(),
                   juce::Justification::centred,
                   false);
    }
}

void DaisyHostPatchAudioProcessorEditor::DrawHostTools(juce::Graphics& g) const
{
    auto panel = GetHostToolsBounds().toFloat().reduced(16.0f);
    const auto menuModel = processor_.GetMenuModelSnapshot();

    juce::Rectangle<int> rackArea = rackHeaderLabel_.getBounds()
                                        .getUnion(rackSelectedNodeLabel_.getBounds())
                                        .getUnion(rackTopologyLabel_.getBounds())
                                        .getUnion(rackTopologyBox_.getBounds());
    for(const auto& cardBounds : rackNodeCardBounds_)
    {
        rackArea = rackArea.getUnion(cardBounds);
    }

    DrawDaisyHostBadge(g,
                       {panel.getRight() - 68.0f, panel.getY() - 2.0f, 54.0f, 54.0f},
                       true);
    DrawRackHeader(g, rackArea.toFloat().expanded(8.0f, 8.0f));

    auto body = panel;
    const float rackHeight = static_cast<float>(rackArea.getBottom()) - panel.getY();
    body.removeFromTop(std::max(0.0f, rackHeight) + 20.0f);

    g.setColour(TextColour());
    g.setFont(TitleFont(18.0f));
    g.drawText("Selected Node Mirror",
               body.removeFromTop(20.0f).toNearestInt(),
               juce::Justification::centredLeft,
               false);

    g.setFont(BodyFont(11.5f));
    g.setColour(TextColour().withAlpha(0.72f));
    g.drawText("Patch controls, encoder/menu state, CV/gate inputs, and host tools apply to the selected rack node.",
               body.removeFromTop(34.0f).toNearestInt(),
               juce::Justification::topLeft,
               true);

    const auto menuBox = body.removeFromTop(92.0f);
    g.setColour(juce::Colours::black.withAlpha(0.18f));
    g.fillRoundedRectangle(menuBox, 14.0f);
    g.setColour(juce::Colours::white.withAlpha(0.10f));
    g.drawRoundedRectangle(menuBox, 14.0f, 1.0f);

    auto menuTextArea = menuBox.reduced(12.0f, 10.0f);
    const auto* currentSection = [&]() -> const daisyhost::MenuSection* {
        for(const auto& section : menuModel.sections)
        {
            if(section.id == menuModel.currentSectionId)
            {
                return &section;
            }
        }
        return nullptr;
    }();
    g.setColour(TextColour());
    g.setFont(TitleFont(14.0f));
    const juce::String menuTitle
        = menuModel.isOpen
              ? juce::String("Menu: ")
                    + (currentSection != nullptr ? currentSection->title.c_str()
                                                 : menuModel.currentSectionId.c_str())
                           : "Menu Closed";
    g.drawText(menuTitle,
               menuTextArea.removeFromTop(18.0f).toNearestInt(),
               juce::Justification::centredLeft,
               false);

    g.setFont(BodyFont(10.5f));
    g.setColour(TextColour().withAlpha(0.72f));
    g.drawText("Click ENC 1 / PUSH, or wheel over ENC 1 to browse.",
               menuTextArea.removeFromTop(16.0f).toNearestInt(),
               juce::Justification::centredLeft,
               false);

    if(currentSection != nullptr)
    {
        g.setColour(TextColour().withAlpha(0.92f));
        for(int row = 0; row < 4; ++row)
        {
            const int itemIndex = row + std::max(0, menuModel.currentSelection - 1);
            if(itemIndex >= static_cast<int>(currentSection->items.size()))
            {
                break;
            }

            const auto& item = currentSection->items[static_cast<std::size_t>(itemIndex)];
            juce::String line
                = (itemIndex == menuModel.currentSelection ? "> " : "  ");
            line += juce::String(item.label);
            if(!item.valueText.empty())
            {
                line += ": " + juce::String(item.valueText);
            }
            g.drawText(line,
                       menuTextArea.removeFromTop(14.0f).toNearestInt(),
                       juce::Justification::centredLeft,
                       false);
        }
    }

    g.setColour(TextColour().withAlpha(0.58f));
    g.drawText(processor_.GetBuildIdentityText(),
               body.removeFromTop(14.0f).toNearestInt(),
               juce::Justification::centredLeft,
               false);
    const auto highlights = processor_.GetReleaseHighlightLines();
    if(highlights.size() > 0)
    {
        g.drawText(highlights[0],
                   body.removeFromTop(14.0f).toNearestInt(),
                   juce::Justification::centredLeft,
                   false);
    }
    if(highlights.size() > 1)
    {
        g.drawText(highlights[1],
                   body.removeFromTop(14.0f).toNearestInt(),
                   juce::Justification::centredLeft,
                   false);
    }
}

void DaisyHostPatchAudioProcessorEditor::DrawRackHeader(
    juce::Graphics&               g,
    const juce::Rectangle<float>& area) const
{
    g.setColour(juce::Colours::black.withAlpha(0.15f));
    g.fillRoundedRectangle(area, 18.0f);
    g.setColour(juce::Colours::white.withAlpha(0.08f));
    g.drawRoundedRectangle(area, 18.0f, 1.1f);

    const auto selectedNodeId = GetSelectedRackNodeIdCompat(processor_);
    const int topologyPreset  = GetRackTopologyPresetCompat(processor_);

    for(std::size_t i = 0; i < rackNodeCardBounds_.size(); ++i)
    {
        const auto cardBounds = rackNodeCardBounds_[i].toFloat();
        const bool isSelected = GetRackNodeIdCompat(processor_, i) == selectedNodeId;

        g.setColour(DrawerInsetColour().withAlpha(isSelected ? 0.98f : 0.86f));
        g.fillRoundedRectangle(cardBounds, 14.0f);

        g.setColour((isSelected ? AccentColour() : juce::Colours::white)
                        .withAlpha(isSelected ? 0.92f : 0.14f));
        g.drawRoundedRectangle(cardBounds, 14.0f, isSelected ? 2.2f : 1.0f);

        if(isSelected)
        {
            g.setColour(AccentColour().withAlpha(0.18f));
            g.fillRoundedRectangle(cardBounds.reduced(4.0f), 10.0f);
        }
    }

    if(HasSerialRackTopology(topologyPreset))
    {
        const std::size_t sourceIndex = topologyPreset == kRackTopologyNode0ToNode1 ? 0u : 1u;
        const std::size_t destIndex   = topologyPreset == kRackTopologyNode0ToNode1 ? 1u : 0u;
        const auto sourceBounds       = rackNodeCardBounds_[sourceIndex].toFloat();
        const auto destBounds         = rackNodeCardBounds_[destIndex].toFloat();
        const bool leftToRight        = sourceBounds.getCentreX() < destBounds.getCentreX();
        const float startX = leftToRight ? sourceBounds.getRight() - 6.0f
                                         : sourceBounds.getX() + 6.0f;
        const float endX = leftToRight ? destBounds.getX() + 6.0f
                                       : destBounds.getRight() - 6.0f;
        const float topY    = sourceBounds.getCentreY() - 8.0f;
        const float bottomY = sourceBounds.getCentreY() + 8.0f;

        g.setColour(juce::Colours::black.withAlpha(0.25f));
        g.drawLine(startX, topY + 1.5f, endX, topY + 1.5f, 4.0f);
        g.drawLine(startX, bottomY + 1.5f, endX, bottomY + 1.5f, 4.0f);

        g.setColour(PortColour(daisyhost::VirtualPortType::kAudio).withAlpha(0.90f));
        g.drawLine(startX, topY, endX, topY, 2.6f);
        g.drawLine(startX, bottomY, endX, bottomY, 2.6f);

        const float arrowX = leftToRight ? endX - 3.0f : endX + 3.0f;
        juce::Path arrowHead;
        arrowHead.startNewSubPath(endX, sourceBounds.getCentreY());
        arrowHead.lineTo(arrowX + (leftToRight ? -8.0f : 8.0f),
                         sourceBounds.getCentreY() - 6.0f);
        arrowHead.lineTo(arrowX + (leftToRight ? -8.0f : 8.0f),
                         sourceBounds.getCentreY() + 6.0f);
        arrowHead.closeSubPath();
        g.fillPath(arrowHead);
    }
}

void DaisyHostPatchAudioProcessorEditor::LayoutRotaryControl(
    ControlUi&                             control,
    const daisyhost::PanelControlSlotSpec& slot,
    bool                                  showLearnButton)
{
    const auto area = RectFromPanel(slot.panelBounds).toNearestInt();
    control.slider.setBounds(area);
    control.label.setBounds(area.withY(area.getBottom() + 2).withHeight(18));
    control.learnButton.setVisible(showLearnButton);
    if(showLearnButton)
    {
        control.learnButton.setBounds(
            area.withY(area.getBottom() + 20).withHeight(20));
    }
}

void DaisyHostPatchAudioProcessorEditor::LayoutButtonControl(
    juce::Button&                           button,
    const daisyhost::PanelControlSlotSpec& slot)
{
    button.setBounds(RectFromPanel(slot.panelBounds).toNearestInt());
}

void DaisyHostPatchAudioProcessorEditor::ConfigureControl(
    ControlUi&          control,
    const juce::String& labelText,
    const std::string&  controlId)
{
    control.controlId = controlId;
    control.slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    control.slider.setTextBoxStyle(
        juce::Slider::TextBoxBelow, false, 58, 18);
    control.slider.setRange(0.0, 1.0, 0.001);
    control.slider.addListener(this);
    control.slider.setColour(juce::Slider::rotarySliderFillColourId, CreamColour());
    control.slider.setColour(juce::Slider::rotarySliderOutlineColourId,
                             InkColour().withAlpha(0.86f));
    control.slider.setColour(juce::Slider::thumbColourId, AccentColour());
    control.slider.setColour(juce::Slider::textBoxTextColourId, TextColour());
    control.slider.setColour(juce::Slider::textBoxBackgroundColourId,
                             InkColour().withAlpha(0.35f));
    control.slider.setColour(juce::Slider::textBoxOutlineColourId,
                             juce::Colours::white.withAlpha(0.16f));
    control.label.setText(labelText, juce::dontSendNotification);
    control.label.setJustificationType(juce::Justification::centred);
    control.label.setColour(juce::Label::textColourId, TextColour());
    control.learnButton.addListener(this);
    control.learnButton.setColour(juce::TextButton::buttonColourId,
                                  InkColour().withAlpha(0.72f));
    control.learnButton.setColour(juce::TextButton::buttonOnColourId,
                                  AccentColour().darker(0.35f));
    control.learnButton.setColour(juce::TextButton::textColourOffId, CreamColour());
}

void DaisyHostPatchAudioProcessorEditor::ConfigureMouseFriendlySlider(
    juce::Slider& slider,
    double        defaultValue)
{
    slider.setScrollWheelEnabled(true);
    slider.setMouseDragSensitivity(260);
    slider.setDoubleClickReturnValue(true, defaultValue);
    slider.setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void DaisyHostPatchAudioProcessorEditor::RegisterKeyboardSource(
    juce::Component& component)
{
    component.addKeyListener(this);
}

void DaisyHostPatchAudioProcessorEditor::UpdateRackUi()
{
    const auto selectedNodeId = GetSelectedRackNodeIdCompat(processor_);
    juce::String selectedNodeDisplayName;

    rackTopologyBox_.setSelectedId(GetRackTopologyPresetCompat(processor_) + 1,
                                   juce::dontSendNotification);
    rackTopologyBox_.setEnabled(HasRackEditorApi<DaisyHostPatchAudioProcessor>::value);

    for(std::size_t i = 0; i < rackNodes_.size(); ++i)
    {
        auto& rackNode       = rackNodes_[i];
        const auto nodeId    = GetRackNodeIdCompat(processor_, i);
        const auto roleLabel = GetRackNodeRoleLabelCompat(processor_, i);
        const auto appId     = GetRackNodeAppIdCompat(processor_, i).toStdString();
        const bool isSelected = nodeId == selectedNodeId;

        rackNode.nodeIdLabel.setText(nodeId, juce::dontSendNotification);
        rackNode.nodeIdLabel.setColour(juce::Label::textColourId,
                                       isSelected ? AccentColour() : TextColour());
        rackNode.roleLabel.setText(roleLabel, juce::dontSendNotification);
        rackNode.roleLabel.setColour(juce::Label::textColourId,
                                     isSelected ? AccentColour().brighter(0.15f)
                                                : TextColour().withAlpha(0.72f));

        int selectedAppItemId = 0;
        for(std::size_t appIndex = 0; appIndex < availableAppIds_.size(); ++appIndex)
        {
            if(availableAppIds_[appIndex] == appId)
            {
                selectedAppItemId = static_cast<int>(appIndex + 1);
                break;
            }
        }
        rackNode.appSelectorBox.setSelectedId(selectedAppItemId,
                                              juce::dontSendNotification);
        rackNode.appSelectorBox.setEnabled(
            HasRackEditorApi<DaisyHostPatchAudioProcessor>::value || i == 0);

        if(isSelected)
        {
            selectedNodeDisplayName = GetRackNodeAppDisplayNameCompat(processor_, i);
        }
    }

    if(selectedNodeDisplayName.isEmpty())
    {
        selectedNodeDisplayName = processor_.GetActiveAppDisplayName();
    }

    rackSelectedNodeLabel_.setText("Selected node: " + selectedNodeId + "  |  "
                                       + selectedNodeDisplayName,
                                   juce::dontSendNotification);
}

void DaisyHostPatchAudioProcessorEditor::UpdateCvGeneratorEditorUi()
{
    for(std::size_t i = 0; i < cvGeneratorTitles_.size(); ++i)
    {
        cvGeneratorModeBoxes_[i].setSelectedId(processor_.GetCvSourceMode(i) + 1,
                                               juce::dontSendNotification);
        cvGeneratorWaveformBoxes_[i].setSelectedId(processor_.GetCvWaveform(i) + 1,
                                                   juce::dontSendNotification);
        cvGeneratorFrequencySliders_[i].setValue(processor_.GetCvFrequencyHz(i),
                                                 juce::dontSendNotification);
        cvGeneratorAmplitudeSliders_[i].setValue(processor_.GetCvAmplitudeVolts(i),
                                                 juce::dontSendNotification);
        cvGeneratorBiasSliders_[i].setValue(processor_.GetCvBiasVolts(i),
                                            juce::dontSendNotification);

        const bool generatorEnabled = processor_.GetCvSourceMode(i) != 0;
        cvGeneratorWaveformBoxes_[i].setEnabled(generatorEnabled);
        cvGeneratorFrequencySliders_[i].setEnabled(generatorEnabled);
        cvGeneratorAmplitudeSliders_[i].setEnabled(generatorEnabled);
        cvGeneratorBiasSliders_[i].setEnabled(generatorEnabled);
    }
}

void DaisyHostPatchAudioProcessorEditor::UpdateLearnButton(ControlUi& control)
{
    if(processor_.IsLearning(control.controlId))
    {
        control.learnButton.setButtonText("Send CC...");
    }
    else
    {
        control.learnButton.setButtonText(
            processor_.GetMidiBindingLabel(control.controlId));
    }
}

DaisyHostPatchAudioProcessorEditor::ControlUi&
DaisyHostPatchAudioProcessorEditor::GetTopControlUi(std::size_t slotIndex)
{
    switch(slotIndex)
    {
        case 0: return dryWet_;
        case 1: return knobs_[0];
        case 2: return knobs_[1];
        case 3: return knobs_[2];
        default: return knobs_[3];
    }
}

const DaisyHostPatchAudioProcessorEditor::ControlUi&
DaisyHostPatchAudioProcessorEditor::GetTopControlUi(std::size_t slotIndex) const
{
    switch(slotIndex)
    {
        case 0: return dryWet_;
        case 1: return knobs_[0];
        case 2: return knobs_[1];
        case 3: return knobs_[2];
        default: return knobs_[3];
    }
}

void DaisyHostPatchAudioProcessorEditor::UpdateTopControlUi()
{
    for(std::size_t i = 0; i < 4; ++i)
    {
        auto& control = GetTopControlUi(i);
        control.controlId = processor_.GetTopControlId(i);
        control.label.setText(processor_.GetTopControlLabel(i),
                              juce::dontSendNotification);
    }
}

void DaisyHostPatchAudioProcessorEditor::ReleaseComputerKeyboardNotes()
{
    for(const auto& heldNote : heldComputerKeyboardNotes_)
    {
        processor_.SetVirtualKeyboardNote(heldNote.second, false, 0.0f);
    }
    heldComputerKeyboardNotes_.clear();
    octaveDownHeld_ = false;
    octaveUpHeld_   = false;
    processor_.AllVirtualKeyboardNotesOff();
}

void DaisyHostPatchAudioProcessorEditor::UpdateComputerKeyboardUi()
{
    const bool enabled = processor_.GetComputerKeyboardEnabled();
    computerKeyboardToggle_.setToggleState(enabled, juce::dontSendNotification);
    computerKeyboardOctaveBox_.setSelectedId(
        processor_.GetComputerKeyboardOctave() + 1,
        juce::dontSendNotification);
    computerKeyboardOctaveBox_.setEnabled(enabled);
    midiKeyboard_.setEnabled(enabled);
    midiKeyboard_.setLowestVisibleKey(
        juce::jlimit(24, 72, processor_.GetComputerKeyboardOctave() * 12));
}

void DaisyHostPatchAudioProcessorEditor::ApplyWindowIconIfNeeded()
{
    if(windowIconApplied_)
    {
        return;
    }

    auto* documentWindow = findParentComponentOfClass<juce::DocumentWindow>();
    if(documentWindow == nullptr)
    {
        return;
    }

    juce::Image iconImage(juce::Image::ARGB, 128, 128, true);
    juce::Graphics iconGraphics(iconImage);
    DrawDaisyHostBadge(iconGraphics, {0.0f, 0.0f, 128.0f, 128.0f}, false);
    documentWindow->setIcon(iconImage);
    windowIconApplied_ = true;
}

void DaisyHostPatchAudioProcessorEditor::HideStandaloneMuteBannerIfNeeded()
{
    if(!processor_.IsStandaloneWrapper() || standaloneMuteBannerHidden_)
    {
        return;
    }

    auto* parent = getParentComponent();
    if(parent == nullptr)
    {
        return;
    }

    const auto editorBounds = ToUiRect(getBounds());
    juce::Component* banner = nullptr;

    for(int i = 0; i < parent->getNumChildComponents(); ++i)
    {
        auto* child = parent->getChildComponent(i);
        if(child == this || !child->isVisible())
        {
            continue;
        }

        if(daisyhost::IsStandaloneMuteBannerCandidate(editorBounds,
                                                      ToUiRect(child->getBounds())))
        {
            banner = child;
            break;
        }
    }

    if(banner == nullptr)
    {
        return;
    }

    const auto adjustedBounds = ToJuceRect(
        daisyhost::AdjustEditorBoundsForHiddenMuteBanner(editorBounds,
                                                         ToUiRect(banner->getBounds())));

    banner->setVisible(false);
    if(getBounds() != adjustedBounds)
    {
        setBounds(adjustedBounds);
    }
    toFront(false);
    standaloneMuteBannerHidden_ = true;
}

void DaisyHostPatchAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    for(std::size_t i = 0; i < 4; ++i)
    {
        if(slider == &GetTopControlUi(i).slider)
        {
            processor_.SetTopControlValue(i, static_cast<float>(slider->getValue()));
            return;
        }
    }

    if(slider == &testInputLevelSlider_)
    {
        processor_.SetTestInputLevel(static_cast<float>(slider->getValue()));
        return;
    }

    if(slider == &testInputFrequencySlider_)
    {
        processor_.SetTestInputFrequencyHz(static_cast<float>(slider->getValue()));
        return;
    }

    for(std::size_t i = 0; i < cvGeneratorFrequencySliders_.size(); ++i)
    {
        if(slider == &cvGeneratorFrequencySliders_[i])
        {
            processor_.SetCvFrequencyHz(i, static_cast<float>(slider->getValue()));
            return;
        }

        if(slider == &cvGeneratorAmplitudeSliders_[i])
        {
            processor_.SetCvAmplitudeVolts(i, static_cast<float>(slider->getValue()));
            return;
        }

        if(slider == &cvGeneratorBiasSliders_[i])
        {
            processor_.SetCvBiasVolts(i, static_cast<float>(slider->getValue()));
            return;
        }
    }

    for(std::size_t i = 0; i < cvSliders_.size(); ++i)
    {
        if(slider == &cvSliders_[i])
        {
            processor_.SetCvManualVoltage(i, static_cast<float>(slider->getValue()));
            return;
        }
    }
}

void DaisyHostPatchAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    for(std::size_t i = 0; i < 4; ++i)
    {
        auto& control = GetTopControlUi(i);
        if(button == &control.learnButton)
        {
            if(processor_.IsLearning(control.controlId))
            {
                processor_.CancelMidiLearn();
            }
            else
            {
                processor_.BeginMidiLearn(control.controlId);
            }
            return;
        }
    }

    if(button == &encoderPressButton_)
    {
        processor_.PulseEncoderPress();
        encoderPressButton_.setToggleState(false, juce::dontSendNotification);
        return;
    }

    if(button == &triggerImpulseButton_)
    {
        processor_.TriggerImpulse();
        return;
    }

    if(button == &computerKeyboardToggle_)
    {
        processor_.SetComputerKeyboardEnabled(
            computerKeyboardToggle_.getToggleState());
        if(!computerKeyboardToggle_.getToggleState())
        {
            ReleaseComputerKeyboardNotes();
        }
        UpdateComputerKeyboardUi();
        return;
    }

    for(std::size_t i = 0; i < gateButtons_.size(); ++i)
    {
        if(button == &gateButtons_[i])
        {
            processor_.SetGateValue(i, gateButtons_[i].getToggleState());
            return;
        }
    }
}

bool DaisyHostPatchAudioProcessorEditor::keyPressed(
    const juce::KeyPress& key,
    juce::Component*      originatingComponent)
{
    juce::ignoreUnused(originatingComponent);

    if(!processor_.GetComputerKeyboardEnabled())
    {
        return false;
    }

    const auto textCharacter = key.getTextCharacter();
    if(textCharacter <= 0 || textCharacter > 127)
    {
        return false;
    }

    const char keyChar = static_cast<char>(textCharacter);
    if(daisyhost::ComputerKeyboardMidi::IsOctaveDownKey(keyChar))
    {
        if(!octaveDownHeld_)
        {
            processor_.SetComputerKeyboardOctave(
                processor_.GetComputerKeyboardOctave() - 1);
            octaveDownHeld_ = true;
            UpdateComputerKeyboardUi();
        }
        return true;
    }

    if(daisyhost::ComputerKeyboardMidi::IsOctaveUpKey(keyChar))
    {
        if(!octaveUpHeld_)
        {
            processor_.SetComputerKeyboardOctave(
                processor_.GetComputerKeyboardOctave() + 1);
            octaveUpHeld_ = true;
            UpdateComputerKeyboardUi();
        }
        return true;
    }

    const int midiNote = daisyhost::ComputerKeyboardMidi::KeyToMidiNote(
        keyChar, processor_.GetComputerKeyboardOctave());
    if(midiNote < 0)
    {
        return false;
    }

    const int keyCode = key.getKeyCode();
    if(heldComputerKeyboardNotes_.find(keyCode) == heldComputerKeyboardNotes_.end())
    {
        heldComputerKeyboardNotes_[keyCode] = midiNote;
        processor_.SetVirtualKeyboardNote(midiNote, true, 1.0f);
    }

    return true;
}

bool DaisyHostPatchAudioProcessorEditor::keyStateChanged(
    bool             isKeyDown,
    juce::Component* originatingComponent)
{
    juce::ignoreUnused(isKeyDown, originatingComponent);

    bool handledAny = false;
    for(auto it = heldComputerKeyboardNotes_.begin();
        it != heldComputerKeyboardNotes_.end();)
    {
        if(!juce::KeyPress::isKeyCurrentlyDown(it->first))
        {
            processor_.SetVirtualKeyboardNote(it->second, false, 0.0f);
            it = heldComputerKeyboardNotes_.erase(it);
            handledAny = true;
        }
        else
        {
            ++it;
        }
    }

    if(octaveDownHeld_ && !juce::KeyPress::isKeyCurrentlyDown('Z')
       && !juce::KeyPress::isKeyCurrentlyDown('z'))
    {
        octaveDownHeld_ = false;
        handledAny      = true;
    }

    if(octaveUpHeld_ && !juce::KeyPress::isKeyCurrentlyDown('X')
       && !juce::KeyPress::isKeyCurrentlyDown('x'))
    {
        octaveUpHeld_ = false;
        handledAny    = true;
    }

    return handledAny;
}

void DaisyHostPatchAudioProcessorEditor::timerCallback()
{
    UpdateRackUi();
    UpdateTopControlUi();
    for(std::size_t i = 0; i < 4; ++i)
    {
        auto& control = GetTopControlUi(i);
        control.slider.setValue(processor_.GetTopControlValue(i),
                                juce::dontSendNotification);
        UpdateLearnButton(control);
    }

    encoderPressButton_.setToggleState(false, juce::dontSendNotification);

    for(std::size_t i = 0; i < cvSliders_.size(); ++i)
    {
        cvSliders_[i].setValue(processor_.GetCvVoltage(i),
                               juce::dontSendNotification);
    }

    for(std::size_t i = 0; i < gateButtons_.size(); ++i)
    {
        gateButtons_[i].setToggleState(processor_.GetGateValue(i),
                                       juce::dontSendNotification);
    }

    testInputModeBox_.setSelectedId(processor_.GetTestInputMode() + 1,
                                    juce::dontSendNotification);
    testInputLevelSlider_.setValue(processor_.GetTestInputLevel(),
                                   juce::dontSendNotification);
    testInputFrequencySlider_.setValue(processor_.GetTestInputFrequencyHz(),
                                       juce::dontSendNotification);
    UpdateComputerKeyboardUi();
    processor_.RefreshMidiInputStatus();
    midiTrackerStatusLabel_.setText(processor_.GetMidiInputStatusText(),
                                    juce::dontSendNotification);
    midiTrackerText_.setText(
        processor_.GetRecentMidiEventLines().joinIntoString("\n"),
        false);

    UpdateCvGeneratorEditorUi();
    ApplyWindowIconIfNeeded();
    HideStandaloneMuteBannerIfNeeded();
    repaint();
}
