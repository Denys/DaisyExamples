#pragma once

#include <array>
#include <string>
#include <unordered_map>

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "DaisyHostPluginProcessor.h"

class DaisyHostPatchAudioProcessorEditor : public juce::AudioProcessorEditor,
                                           private juce::Slider::Listener,
                                           private juce::Button::Listener,
                                           private juce::KeyListener,
                                           private juce::Timer
{
  public:
    explicit DaisyHostPatchAudioProcessorEditor(DaisyHostPatchAudioProcessor&);
    ~DaisyHostPatchAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

  private:
    struct ControlUi
    {
        juce::Slider     slider;
        juce::Label      label;
        juce::TextButton learnButton;
        std::string      controlId;
    };

    juce::Rectangle<int>
    GetEditorBounds() const;
    juce::Rectangle<int>
    GetPatchPanelBounds() const;
    juce::Rectangle<int>
    GetHostToolsBounds() const;
    juce::Rectangle<float>
    RectFromPanel(const daisyhost::PanelRect& rect) const;
    void DrawDisplay(juce::Graphics& g, const juce::Rectangle<float>& area) const;
    void DrawPorts(juce::Graphics& g) const;
    void DrawHostTools(juce::Graphics& g) const;
    void ConfigureControl(ControlUi& control,
                          const juce::String& labelText,
                          const std::string& controlId);
    void ConfigureMouseFriendlySlider(juce::Slider& slider, double defaultValue);
    void RegisterKeyboardSource(juce::Component& component);
    void UpdateLearnButton(ControlUi& control);
    void ReleaseComputerKeyboardNotes();
    void UpdateComputerKeyboardUi();

    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
    bool keyStateChanged(bool isKeyDown, juce::Component* originatingComponent) override;
    void timerCallback() override;

    DaisyHostPatchAudioProcessor& processor_;
    std::array<ControlUi, 4>      knobs_;
    ControlUi                     dryWet_;
    juce::TextButton              encoderPressButton_;
    juce::ToggleButton            computerKeyboardToggle_;
    juce::Label                   computerKeyboardLabel_;
    juce::Label                   computerKeyboardOctaveLabel_;
    juce::ComboBox                computerKeyboardOctaveBox_;
    juce::MidiKeyboardComponent   midiKeyboard_;
    juce::Label                   midiTrackerLabel_;
    juce::Label                   midiTrackerStatusLabel_;
    juce::TextEditor              midiTrackerText_;
    std::array<juce::Slider, 4>   cvSliders_;
    std::array<juce::Label, 4>    cvLabels_;
    std::array<juce::ToggleButton, 2> gateButtons_;
    juce::ComboBox                testInputModeBox_;
    juce::Label                   testInputModeLabel_;
    juce::Slider                  testInputLevelSlider_;
    juce::Label                   testInputLevelLabel_;
    juce::TextButton              triggerImpulseButton_;
    std::unordered_map<int, int>  heldComputerKeyboardNotes_;
    bool                          octaveDownHeld_ = false;
    bool                          octaveUpHeld_   = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DaisyHostPatchAudioProcessorEditor)
};
