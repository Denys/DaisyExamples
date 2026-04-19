#include <juce_gui_basics/juce_gui_basics.h>

#include "DaisyHostHubWindow.h"

class DaisyHostHubApplication : public juce::JUCEApplication
{
  public:
    const juce::String getApplicationName() override
    {
        return "DaisyHost Hub";
    }

    const juce::String getApplicationVersion() override
    {
        return DAISYHOST_VERSION;
    }

    bool moreThanOneInstanceAllowed() override
    {
        return true;
    }

    void initialise(const juce::String&) override
    {
        mainWindow_ = std::make_unique<DaisyHostHubWindow>();
    }

    void shutdown() override
    {
        mainWindow_.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

  private:
    std::unique_ptr<DaisyHostHubWindow> mainWindow_;
};

START_JUCE_APPLICATION(DaisyHostHubApplication)
