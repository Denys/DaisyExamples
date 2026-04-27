#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

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
    void mouseWheelMove(const juce::MouseEvent& event,
                        const juce::MouseWheelDetails& wheel) override;

  private:
    struct ControlUi
    {
        juce::Slider     slider;
        juce::Label      label;
        juce::TextButton learnButton;
        std::string      controlId;
    };

    struct RackNodeUi
    {
        juce::Label    nodeIdLabel;
        juce::Label    roleLabel;
        juce::ComboBox appSelectorBox;
    };

    juce::Rectangle<int>
    GetEditorBounds() const;
    juce::Rectangle<int>
    GetBoardPanelBounds() const;
    juce::Rectangle<int>
    GetBoardPanelContentBounds() const;
    juce::Rectangle<int>
    GetHostToolsBounds() const;
    juce::Rectangle<float>
    RectFromPanel(const daisyhost::PanelRect& rect) const;
    const daisyhost::PanelControlSlotSpec*
    FindSurfaceControlByTarget(const std::string& targetId) const;
    const daisyhost::PanelControlSlotSpec*
    FindSurfaceControlById(const std::string& id) const;
    juce::Justification
    ToJustification(daisyhost::TextAlignment alignment) const;
    void DrawPanelDecorations(juce::Graphics& g) const;
    void DrawPassiveSurfaceControls(juce::Graphics& g) const;
    void DrawPanelIndicators(juce::Graphics& g) const;
    void DrawPanelTexts(juce::Graphics& g) const;
    void DrawBoardTraces(juce::Graphics& g) const;
    void DrawSeedModule(juce::Graphics& g,
                        const juce::Rectangle<float>& area) const;
    void DrawDisplay(juce::Graphics& g, const juce::Rectangle<float>& area) const;
    void DrawPorts(juce::Graphics& g) const;
    void DrawHostTools(juce::Graphics& g) const;
    void DrawRackHeader(juce::Graphics& g,
                        const juce::Rectangle<float>& area) const;
    void LayoutRotaryControl(ControlUi& control,
                             const daisyhost::PanelControlSlotSpec& slot,
                             bool showLearnButton = true);
    void LayoutButtonControl(juce::Button& button,
                             const daisyhost::PanelControlSlotSpec& slot);
    void ConfigureControl(ControlUi& control,
                          const juce::String& labelText,
                          const std::string& controlId);
    void ConfigureMouseFriendlySlider(juce::Slider& slider, double defaultValue);
    void RegisterKeyboardSource(juce::Component& component);
    void UpdateLearnButton(ControlUi& control);
    void UpdateRackUi();
    void UpdateCvGeneratorEditorUi();
    ControlUi& GetTopControlUi(std::size_t slotIndex);
    const ControlUi& GetTopControlUi(std::size_t slotIndex) const;
    void UpdateTopControlUi();
    void LayoutFieldControls();
    void UpdateFieldControlUi();
    void UpdateFieldDrawerPageUi();
    void UpdateModulationLaneUi();
    bool IsInteractiveExtendedSurfaceControl(const std::string& surfaceId) const;
    void ReleaseComputerKeyboardNotes();
    void UpdateComputerKeyboardUi();
    void ApplyWindowIconIfNeeded();
    void HideStandaloneMuteBannerIfNeeded();

    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;
    void buttonStateChanged(juce::Button* button) override;
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
    bool keyStateChanged(bool isKeyDown, juce::Component* originatingComponent) override;
    void timerCallback() override;

    DaisyHostPatchAudioProcessor& processor_;
    std::array<ControlUi, 4>      knobs_;
    ControlUi                     dryWet_;
    std::array<ControlUi, daisyhost::kDaisyFieldKnobCount> fieldKnobs_;
    std::array<juce::TextButton, daisyhost::kDaisyFieldKeyCount> fieldKeyButtons_;
    std::array<juce::Label, 5>     fieldKeyMappingLabels_;
    std::array<juce::TextButton, daisyhost::kDaisyFieldSwitchCount> fieldSwitchButtons_;
    std::array<juce::TextButton, 3> fieldDrawerPageButtons_;
    std::array<juce::TextButton, 16> fieldParameterButtons_;
    std::array<juce::Label, daisyhost::kHostModulationLaneCount>
        modulationLaneLabels_;
    std::array<juce::Label, daisyhost::kHostModulationLaneCount>
        modulationLaneDetailLabels_;
    std::array<juce::ComboBox, daisyhost::kHostModulationLaneCount>
        modulationLaneSourceBoxes_;
    std::array<juce::Slider, daisyhost::kHostModulationLaneCount>
        modulationLaneAmountSliders_;
    std::array<juce::ToggleButton, daisyhost::kHostModulationLaneCount>
        modulationLaneEnableButtons_;
    std::array<juce::TextButton, daisyhost::kHostModulationLaneCount>
        modulationLaneClearButtons_;
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
    juce::Label                   rackHeaderLabel_;
    juce::Label                   rackTopologyLabel_;
    juce::Label                   rackSelectedNodeLabel_;
    juce::Label                   rackTargetHintLabel_;
    std::array<RackNodeUi, 2>     rackNodes_;
    juce::ComboBox                rackTopologyBox_;
    std::array<juce::Rectangle<int>, 2> rackNodeCardBounds_{};
    juce::ComboBox                testInputModeBox_;
    juce::Label                   testInputModeLabel_;
    juce::Slider                  testInputLevelSlider_;
    juce::Label                   testInputLevelLabel_;
    juce::Slider                  testInputFrequencySlider_;
    juce::Label                   testInputFrequencyLabel_;
    juce::TextButton              triggerImpulseButton_;
    std::unordered_map<int, int>  heldComputerKeyboardNotes_;
    std::vector<std::string>      availableAppIds_;
    juce::Label                   cvGeneratorLabel_;
    std::array<juce::Label, 4>    cvGeneratorTitles_;
    std::array<juce::ComboBox, 4> cvGeneratorModeBoxes_;
    std::array<juce::ComboBox, 4> cvGeneratorTargetBoxes_;
    std::array<juce::ComboBox, 4> cvGeneratorWaveformBoxes_;
    std::array<juce::Slider, 4>   cvGeneratorFrequencySliders_;
    std::array<juce::Slider, 4>   cvGeneratorAmplitudeSliders_;
    std::array<juce::Slider, 4>   cvGeneratorBiasSliders_;
    std::array<std::array<juce::Label, 3>, 4> cvGeneratorSliderLabels_;
    std::array<bool, daisyhost::kDaisyFieldSwitchCount> fieldSwitchKeyHeld_{};
    int                           selectedFieldParameterIndex_ = -1;
    bool                          octaveDownHeld_ = false;
    bool                          octaveUpHeld_   = false;
    bool                          windowIconApplied_ = false;
    bool                          standaloneMuteBannerHidden_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DaisyHostPatchAudioProcessorEditor)
};
