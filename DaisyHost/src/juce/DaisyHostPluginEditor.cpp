#include "DaisyHostPluginEditor.h"

#include <algorithm>

#include "daisyhost/ComputerKeyboardMidi.h"

namespace
{
juce::Colour BackgroundColour()
{
    return juce::Colour::fromRGB(25, 27, 30);
}

juce::Colour PanelColour()
{
    return juce::Colour::fromRGB(68, 72, 78);
}

juce::Colour HostToolsColour()
{
    return juce::Colour::fromRGB(40, 43, 48);
}

juce::Colour AccentColour()
{
    return juce::Colour::fromRGB(236, 146, 37);
}

juce::Colour TextColour()
{
    return juce::Colour::fromRGB(245, 245, 245);
}

juce::Colour PortColour(daisyhost::VirtualPortType type)
{
    switch(type)
    {
        case daisyhost::VirtualPortType::kAudio:
            return juce::Colour::fromRGB(83, 193, 201);
        case daisyhost::VirtualPortType::kCv:
            return juce::Colour::fromRGB(90, 173, 255);
        case daisyhost::VirtualPortType::kGate:
            return juce::Colour::fromRGB(234, 220, 124);
        case daisyhost::VirtualPortType::kMidi:
            return juce::Colour::fromRGB(236, 146, 37);
        default: return AccentColour();
    }
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

juce::Font TitleFont(float size)
{
    return juce::Font(juce::FontOptions(size, juce::Font::bold));
}

juce::Font BodyFont(float size)
{
    return juce::Font(juce::FontOptions(size));
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
    setSize(1220, 760);
    addKeyListener(this);

    for(std::size_t i = 0; i < knobs_.size(); ++i)
    {
        ConfigureControl(knobs_[i],
                         "CTRL " + juce::String(static_cast<int>(i + 1)),
                         processor_.GetBoardProfile().controls[i].id);
        ConfigureMouseFriendlySlider(knobs_[i].slider, 0.0);
        addAndMakeVisible(knobs_[i].slider);
        addAndMakeVisible(knobs_[i].label);
        addAndMakeVisible(knobs_[i].learnButton);
        RegisterKeyboardSource(knobs_[i].slider);
        RegisterKeyboardSource(knobs_[i].learnButton);
    }

    ConfigureControl(dryWet_, "DRY/WET", processor_.GetBoardProfile().controls[4].id);
    dryWet_.controlId = daisyhost::apps::MultiDelayCore::MakeDryWetControlId("node0");
    ConfigureMouseFriendlySlider(dryWet_.slider, 0.5);
    addAndMakeVisible(dryWet_.slider);
    addAndMakeVisible(dryWet_.label);
    addAndMakeVisible(dryWet_.learnButton);
    RegisterKeyboardSource(dryWet_.slider);
    RegisterKeyboardSource(dryWet_.learnButton);

    encoderPressButton_.setButtonText("Push");
    encoderPressButton_.setClickingTogglesState(true);
    encoderPressButton_.addListener(this);
    addAndMakeVisible(encoderPressButton_);
    RegisterKeyboardSource(encoderPressButton_);

    for(std::size_t i = 0; i < cvSliders_.size(); ++i)
    {
        cvSliders_[i].setSliderStyle(juce::Slider::LinearVertical);
        cvSliders_[i].setTextBoxStyle(
            juce::Slider::TextBoxBelow, false, 48, 18);
        cvSliders_[i].setRange(0.0, 1.0, 0.001);
        cvSliders_[i].setValue(0.5);
        cvSliders_[i].addListener(this);
        ConfigureMouseFriendlySlider(cvSliders_[i], 0.5);
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
        addAndMakeVisible(gateButtons_[i]);
        RegisterKeyboardSource(gateButtons_[i]);
    }

    testInputModeLabel_.setText("Test Input", juce::dontSendNotification);
    testInputModeLabel_.setJustificationType(juce::Justification::centredLeft);
    testInputModeLabel_.setColour(juce::Label::textColourId, TextColour());
    addAndMakeVisible(testInputModeLabel_);

    for(int mode = 0; mode < DaisyHostPatchAudioProcessor::kNumTestInputModes; ++mode)
    {
        testInputModeBox_.addItem(processor_.GetTestInputModeName(mode), mode + 1);
    }
    testInputModeBox_.onChange = [this]() {
        processor_.SetTestInputMode(testInputModeBox_.getSelectedId() - 1);
    };
    addAndMakeVisible(testInputModeBox_);
    RegisterKeyboardSource(testInputModeBox_);

    testInputLevelLabel_.setText("Level", juce::dontSendNotification);
    testInputLevelLabel_.setJustificationType(juce::Justification::centredLeft);
    testInputLevelLabel_.setColour(juce::Label::textColourId, TextColour());
    addAndMakeVisible(testInputLevelLabel_);

    testInputLevelSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    testInputLevelSlider_.setTextBoxStyle(
        juce::Slider::TextBoxRight, false, 52, 18);
    testInputLevelSlider_.setRange(0.0, 1.0, 0.001);
    testInputLevelSlider_.addListener(this);
    ConfigureMouseFriendlySlider(testInputLevelSlider_, 0.35);
    addAndMakeVisible(testInputLevelSlider_);
    RegisterKeyboardSource(testInputLevelSlider_);

    triggerImpulseButton_.setButtonText("Fire Impulse");
    triggerImpulseButton_.addListener(this);
    addAndMakeVisible(triggerImpulseButton_);
    RegisterKeyboardSource(triggerImpulseButton_);

    computerKeyboardLabel_.setText("Virtual MIDI Keys",
                                   juce::dontSendNotification);
    computerKeyboardLabel_.setJustificationType(juce::Justification::centredLeft);
    computerKeyboardLabel_.setColour(juce::Label::textColourId, TextColour());
    addAndMakeVisible(computerKeyboardLabel_);

    computerKeyboardToggle_.setButtonText("A/W/S/E Enabled");
    computerKeyboardToggle_.setClickingTogglesState(true);
    computerKeyboardToggle_.addListener(this);
    addAndMakeVisible(computerKeyboardToggle_);
    RegisterKeyboardSource(computerKeyboardToggle_);

    computerKeyboardOctaveLabel_.setText("Octave",
                                         juce::dontSendNotification);
    computerKeyboardOctaveLabel_.setJustificationType(
        juce::Justification::centredLeft);
    computerKeyboardOctaveLabel_.setColour(juce::Label::textColourId,
                                           TextColour());
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
    midiKeyboard_.setKeyWidth(16.0f);
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
                                      juce::Colours::white.withAlpha(0.75f));
    addAndMakeVisible(midiTrackerStatusLabel_);

    midiTrackerText_.setMultiLine(true);
    midiTrackerText_.setReadOnly(true);
    midiTrackerText_.setScrollbarsShown(true);
    midiTrackerText_.setCaretVisible(false);
    midiTrackerText_.setColour(juce::TextEditor::backgroundColourId,
                               juce::Colour::fromRGB(22, 24, 27));
    midiTrackerText_.setColour(juce::TextEditor::outlineColourId,
                               juce::Colours::white.withAlpha(0.14f));
    midiTrackerText_.setColour(juce::TextEditor::textColourId, TextColour());
    addAndMakeVisible(midiTrackerText_);

    UpdateComputerKeyboardUi();

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
    triggerImpulseButton_.removeListener(this);
    computerKeyboardToggle_.removeListener(this);
}

void DaisyHostPatchAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(BackgroundColour());

    const auto editorBounds = GetEditorBounds().toFloat();
    const auto patchPanel   = GetPatchPanelBounds().toFloat();
    const auto hostTools    = GetHostToolsBounds().toFloat();

    g.setColour(PanelColour());
    g.fillRoundedRectangle(patchPanel, 18.0f);
    g.setColour(juce::Colours::white.withAlpha(0.14f));
    g.drawRoundedRectangle(patchPanel.reduced(1.0f), 18.0f, 2.0f);

    g.setColour(HostToolsColour());
    g.fillRoundedRectangle(hostTools, 18.0f);
    g.setColour(juce::Colours::white.withAlpha(0.10f));
    g.drawRoundedRectangle(hostTools.reduced(1.0f), 18.0f, 1.5f);

    g.setColour(TextColour());
    g.setFont(TitleFont(30.0f));
    g.drawText("DaisyHost Patch",
               juce::Rectangle<int>(patchPanel.getX() + 24.0f,
                                    patchPanel.getY() + 16.0f,
                                    340,
                                    36),
               juce::Justification::centredLeft);

    g.setFont(BodyFont(13.0f));
    g.setColour(juce::Colours::white.withAlpha(0.78f));
    g.drawText("MultiDelay core running against a virtual Daisy Patch panel",
               juce::Rectangle<int>(patchPanel.getX() + 26.0f,
                                    patchPanel.getY() + 52.0f,
                                    420,
                                    18),
               juce::Justification::centredLeft);

    DrawDisplay(g, RectFromPanel(processor_.GetBoardProfile().display.panelBounds));
    DrawPorts(g);
    DrawHostTools(g);

    g.setColour(juce::Colours::white.withAlpha(0.55f));
    g.setFont(BodyFont(12.0f));
    g.drawText("Standalone note: if JUCE mutes live input to avoid a feedback loop, use the test input tools.",
               juce::Rectangle<int>(editorBounds.getX(),
                                    editorBounds.getBottom() - 18.0f,
                                    static_cast<int>(editorBounds.getWidth()),
                                    16),
               juce::Justification::centred);
}

void DaisyHostPatchAudioProcessorEditor::resized()
{
    const auto patchPanel = GetPatchPanelBounds();
    const auto hostTools  = GetHostToolsBounds();

    for(std::size_t i = 0; i < 4; ++i)
    {
        const auto& spec = processor_.GetBoardProfile().controls[i];
        auto        area = RectFromPanel(spec.panelBounds).toNearestInt();
        knobs_[i].slider.setBounds(area);
        knobs_[i].label.setBounds(area.withY(area.getBottom() + 4).withHeight(18));
        knobs_[i].learnButton.setBounds(
            area.withY(area.getBottom() + 24).withHeight(22));
    }

    const auto encoderArea
        = RectFromPanel(processor_.GetBoardProfile().controls[4].panelBounds)
              .toNearestInt();
    dryWet_.slider.setBounds(encoderArea);
    dryWet_.label.setBounds(
        encoderArea.withY(encoderArea.getBottom() + 4).withHeight(18));
    dryWet_.learnButton.setBounds(
        encoderArea.withY(encoderArea.getBottom() + 24).withHeight(22));
    encoderPressButton_.setBounds(
        encoderArea.withY(encoderArea.getBottom() + 52).withHeight(26));

    auto section = hostTools.reduced(18);
    testInputModeLabel_.setBounds(section.removeFromTop(18));
    testInputModeBox_.setBounds(section.removeFromTop(28));
    section.removeFromTop(12);
    testInputLevelLabel_.setBounds(section.removeFromTop(18));
    testInputLevelSlider_.setBounds(section.removeFromTop(28));
    section.removeFromTop(12);
    triggerImpulseButton_.setBounds(section.removeFromTop(28));
    section.removeFromTop(16);

    computerKeyboardLabel_.setBounds(section.removeFromTop(18));
    computerKeyboardToggle_.setBounds(section.removeFromTop(26));
    section.removeFromTop(8);
    computerKeyboardOctaveLabel_.setBounds(section.removeFromTop(18));
    computerKeyboardOctaveBox_.setBounds(section.removeFromTop(28));
    section.removeFromTop(8);
    midiKeyboard_.setBounds(section.removeFromTop(82));
    section.removeFromTop(10);

    midiTrackerLabel_.setBounds(section.removeFromTop(18));
    midiTrackerStatusLabel_.setBounds(section.removeFromTop(18));
    midiTrackerText_.setBounds(section.removeFromTop(96));
    section.removeFromTop(12);

    auto cvArea = section.removeFromTop(122);
    const int cvWidth = (cvArea.getWidth() - 18) / 4;
    for(std::size_t i = 0; i < cvSliders_.size(); ++i)
    {
        auto column = cvArea.removeFromLeft(cvWidth);
        cvLabels_[i].setBounds(column.removeFromTop(18));
        cvSliders_[i].setBounds(column.withTrimmedTop(6));
        cvArea.removeFromLeft(6);
    }

    section.removeFromTop(12);
    auto gateArea = section.removeFromTop(28);
    gateButtons_[0].setBounds(gateArea.removeFromLeft(gateArea.getWidth() / 2 - 6));
    gateArea.removeFromLeft(12);
    gateButtons_[1].setBounds(gateArea);
}

void DaisyHostPatchAudioProcessorEditor::mouseDown(const juce::MouseEvent&)
{
    grabKeyboardFocus();
}

juce::Rectangle<int> DaisyHostPatchAudioProcessorEditor::GetEditorBounds() const
{
    return getLocalBounds().reduced(18);
}

juce::Rectangle<int> DaisyHostPatchAudioProcessorEditor::GetPatchPanelBounds() const
{
    auto bounds = GetEditorBounds();
    auto left   = bounds.removeFromLeft(static_cast<int>(bounds.getWidth() * 0.76f));
    return left;
}

juce::Rectangle<int> DaisyHostPatchAudioProcessorEditor::GetHostToolsBounds() const
{
    auto bounds = GetEditorBounds();
    bounds.removeFromLeft(static_cast<int>(bounds.getWidth() * 0.76f) + 12);
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

void DaisyHostPatchAudioProcessorEditor::DrawDisplay(
    juce::Graphics&              g,
    const juce::Rectangle<float>& area) const
{
    g.setColour(juce::Colour::fromRGB(18, 20, 22));
    g.fillRoundedRectangle(area, 8.0f);
    g.setColour(AccentColour().withAlpha(0.85f));
    g.drawRoundedRectangle(area, 8.0f, 1.5f);

    const auto display = processor_.GetDisplayModelSnapshot();
    const auto origin  = area.reduced(10.0f);

    for(const auto& bar : display.bars)
    {
        juce::Rectangle<float> barRect(origin.getX() + static_cast<float>(bar.x),
                                       origin.getY() + static_cast<float>(bar.y),
                                       static_cast<float>(bar.width),
                                       static_cast<float>(bar.height));
        g.setColour(juce::Colours::white.withAlpha(0.18f));
        g.drawRect(barRect);
        g.setColour(AccentColour());
        g.fillRect(barRect.withWidth(barRect.getWidth() * bar.normalized));
    }

    for(const auto& text : display.texts)
    {
        g.setColour(text.inverted ? juce::Colours::black : TextColour());
        g.setFont(text.y <= 4 ? TitleFont(16.0f) : BodyFont(14.0f));
        g.drawText(text.text,
                   juce::Rectangle<float>(origin.getX() + static_cast<float>(text.x),
                                          origin.getY() + static_cast<float>(text.y),
                                          area.getWidth() - 16.0f,
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
        const auto bounds = RectFromPanel(port.panelBounds);
        const float diameter
            = std::min(bounds.getWidth(), bounds.getHeight() * 0.95f);
        juce::Rectangle<float> jackArea(bounds.getCentreX() - diameter * 0.5f,
                                        bounds.getCentreY() - diameter * 0.5f + 4.0f,
                                        diameter,
                                        diameter);

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
        g.setColour(juce::Colours::white.withAlpha(0.18f));
        g.fillEllipse(jackArea.expanded(5.0f));
        g.setColour(colour.withAlpha(0.25f + activity * 0.65f));
        g.fillEllipse(jackArea.reduced(3.5f));
        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.fillEllipse(jackArea.reduced(diameter * 0.18f));
        g.setColour(juce::Colours::white.withAlpha(0.30f));
        g.drawEllipse(jackArea.expanded(5.0f), 1.5f);

        juce::Rectangle<float> labelArea(bounds.getX() - 16.0f,
                                         bounds.getY() - 18.0f,
                                         bounds.getWidth() + 32.0f,
                                         14.0f);
        if(bounds.getBottom() > patch.getBottom() - 70.0f)
        {
            labelArea = labelArea.translated(0.0f, -8.0f);
        }

        g.setColour(TextColour());
        g.setFont(BodyFont(11.0f));
        g.drawText(port.label,
                   labelArea.toNearestInt(),
                   juce::Justification::centred,
                   false);
    }
}

void DaisyHostPatchAudioProcessorEditor::DrawHostTools(juce::Graphics& g) const
{
    const auto panel = GetHostToolsBounds().toFloat();
    const auto inner = panel.reduced(18.0f);

    g.setColour(TextColour());
    g.setFont(TitleFont(22.0f));
    g.drawText("Host Tools",
               juce::Rectangle<int>(inner.getX(),
                                    inner.getY(),
                                    static_cast<int>(inner.getWidth()),
                                    24),
               juce::Justification::centredLeft);

    g.setFont(BodyFont(12.0f));
    g.setColour(juce::Colours::white.withAlpha(0.70f));
    g.drawText("Use these for standalone testing when no hardware or safe live input is available.",
               juce::Rectangle<int>(inner.getX(),
                                    inner.getY() + 28.0f,
                                    static_cast<int>(inner.getWidth()),
                                    30),
               juce::Justification::topLeft,
               true);

    g.drawText("Mouse-play the keyboard below, or use A/W/S/E... and Z/X for octave shift.",
               juce::Rectangle<int>(inner.getX(),
                                    inner.getY() + 58.0f,
                                    static_cast<int>(inner.getWidth()),
                                    32),
               juce::Justification::topLeft,
               true);

    g.drawText(
        "MIDI learn listens for controller CC messages. Note input drives the preview tone.",
        juce::Rectangle<int>(inner.getX(),
                             inner.getY() + 92.0f,
                             static_cast<int>(inner.getWidth()),
                             30),
        juce::Justification::topLeft,
        true);
}

void DaisyHostPatchAudioProcessorEditor::ConfigureControl(
    ControlUi&          control,
    const juce::String& labelText,
    const std::string&  controlId)
{
    control.controlId = controlId;
    control.slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    control.slider.setTextBoxStyle(
        juce::Slider::TextBoxBelow, false, 56, 18);
    control.slider.setRange(0.0, 1.0, 0.001);
    control.slider.addListener(this);
    control.label.setText(labelText, juce::dontSendNotification);
    control.label.setJustificationType(juce::Justification::centred);
    control.label.setColour(juce::Label::textColourId, TextColour());
    control.learnButton.addListener(this);
    control.learnButton.setColour(juce::TextButton::buttonColourId,
                                  AccentColour().darker(0.6f));
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

void DaisyHostPatchAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    for(std::size_t i = 0; i < knobs_.size(); ++i)
    {
        if(slider == &knobs_[i].slider)
        {
            processor_.SetKnobValue(i, static_cast<float>(slider->getValue()));
            return;
        }
    }

    if(slider == &dryWet_.slider)
    {
        processor_.SetDryWetValue(static_cast<float>(slider->getValue()));
        return;
    }

    if(slider == &testInputLevelSlider_)
    {
        processor_.SetTestInputLevel(static_cast<float>(slider->getValue()));
        return;
    }

    for(std::size_t i = 0; i < cvSliders_.size(); ++i)
    {
        if(slider == &cvSliders_[i])
        {
            processor_.SetCvValue(i, static_cast<float>(slider->getValue()));
            return;
        }
    }
}

void DaisyHostPatchAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    for(auto& control : knobs_)
    {
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

    if(button == &dryWet_.learnButton)
    {
        if(processor_.IsLearning(dryWet_.controlId))
        {
            processor_.CancelMidiLearn();
        }
        else
        {
            processor_.BeginMidiLearn(dryWet_.controlId);
        }
        return;
    }

    if(button == &encoderPressButton_)
    {
        processor_.SetEncoderPressed(encoderPressButton_.getToggleState());
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
    for(std::size_t i = 0; i < knobs_.size(); ++i)
    {
        knobs_[i].slider.setValue(processor_.GetKnobValue(i),
                                  juce::dontSendNotification);
        UpdateLearnButton(knobs_[i]);
    }

    dryWet_.slider.setValue(processor_.GetDryWetValue(), juce::dontSendNotification);
    UpdateLearnButton(dryWet_);
    encoderPressButton_.setToggleState(processor_.GetEncoderPressed(),
                                       juce::dontSendNotification);

    for(std::size_t i = 0; i < cvSliders_.size(); ++i)
    {
        cvSliders_[i].setValue(processor_.GetCvValue(i), juce::dontSendNotification);
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
    UpdateComputerKeyboardUi();
    processor_.RefreshMidiInputStatus();
    midiTrackerStatusLabel_.setText(processor_.GetMidiInputStatusText(),
                                    juce::dontSendNotification);
    midiTrackerText_.setText(
        processor_.GetRecentMidiEventLines().joinIntoString("\n"),
        false);

    repaint();
}
