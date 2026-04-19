#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class DaisyHostHubWindow : public juce::DocumentWindow
{
  public:
    DaisyHostHubWindow();
    void closeButtonPressed() override;
};
