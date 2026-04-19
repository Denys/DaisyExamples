#include "DaisyHostHubWindow.h"

#include <vector>

#include "daisyhost/AppRegistry.h"
#include "daisyhost/HubSupport.h"

namespace
{
class DaisyHostHubContent final : public juce::Component
{
  public:
    DaisyHostHubContent()
        : toolPaths_(daisyhost::DiscoverDefaultHubToolPaths(
              juce::File::getSpecialLocation(juce::File::currentExecutableFile))),
          profileFile_(daisyhost::GetDefaultHubProfileFile())
    {
        titleLabel_.setText("DaisyHost Hub", juce::dontSendNotification);
        titleLabel_.setFont(juce::FontOptions(24.0f, juce::Font::bold));
        addAndMakeVisible(titleLabel_);

        configureLabel(boardLabel_, "Board");
        configureLabel(appLabel_, "App");
        configureLabel(activityLabel_, "Activity");
        configureLabel(renderLabel_, "Render Scenario");
        configureLabel(datasetLabel_, "Dataset Job");
        configureLabel(outputLabel_, "Output Folder");
        configureLabel(seedLabel_, "Seed");
        addAndMakeVisible(statusLabel_);
        statusLabel_.setColour(juce::Label::textColourId,
                               juce::Colours::lightgrey);

        addAndMakeVisible(boardCombo_);
        addAndMakeVisible(appCombo_);
        addAndMakeVisible(activityCombo_);

        boardCombo_.onChange    = [this] { saveProfile(); };
        appCombo_.onChange      = [this] { saveProfile(); };
        activityCombo_.onChange = [this] {
            refreshVisibility();
            saveProfile();
        };

        configurePathEditor(renderEditor_,
                            "Optional: blank uses an example or generated render scenario");
        configurePathEditor(datasetEditor_,
                            "Optional: blank generates a dataset job");
        configurePathEditor(outputEditor_,
                            "Optional: blank uses the hub support folder");

        configureButton(browseRenderButton_, "Browse...");
        configureButton(browseDatasetButton_, "Browse...");
        configureButton(browseOutputButton_, "Browse...");
        configureButton(launchButton_, "Launch");

        browseRenderButton_.onClick = [this] {
            browseForFile(renderEditor_, "*.json");
        };
        browseDatasetButton_.onClick = [this] {
            browseForFile(datasetEditor_, "*.json");
        };
        browseOutputButton_.onClick = [this] { browseForDirectory(outputEditor_); };
        launchButton_.onClick       = [this] { launchSelected(); };

        seedEditor_.setInputRestrictions(10, "0123456789");
        seedEditor_.onTextChange = [this] { saveProfile(); };
        renderEditor_.onTextChange = [this] { saveProfile(); };
        datasetEditor_.onTextChange = [this] { saveProfile(); };
        outputEditor_.onTextChange = [this] { saveProfile(); };

        populateBoardChoices();
        populateAppChoices();
        populateActivityChoices();
        loadProfile();
        refreshVisibility();
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(16);
        titleLabel_.setBounds(area.removeFromTop(32));
        area.removeFromTop(12);

        auto row = [&area](int height) {
            auto r = area.removeFromTop(height);
            area.removeFromTop(10);
            return r;
        };

        layoutRow(row(24), boardLabel_, boardCombo_);
        layoutRow(row(24), appLabel_, appCombo_);
        layoutRow(row(24), activityLabel_, activityCombo_);
        layoutPathRow(row(24), renderLabel_, renderEditor_, browseRenderButton_);
        layoutPathRow(row(24), datasetLabel_, datasetEditor_, browseDatasetButton_);
        layoutPathRow(row(24), outputLabel_, outputEditor_, browseOutputButton_);
        layoutRow(row(24), seedLabel_, seedEditor_);

        auto footer = area.removeFromBottom(56);
        launchButton_.setBounds(footer.removeFromRight(140).reduced(0, 8));
        statusLabel_.setBounds(footer.reduced(0, 8));
    }

  private:
    void configureLabel(juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        addAndMakeVisible(label);
    }

    void configureButton(juce::TextButton& button, const juce::String& text)
    {
        button.setButtonText(text);
        addAndMakeVisible(button);
    }

    void configurePathEditor(juce::TextEditor& editor, const juce::String& hint)
    {
        editor.setTextToShowWhenEmpty(hint, juce::Colours::grey);
        editor.onTextChange = [this] { saveProfile(); };
        addAndMakeVisible(editor);
    }

    void populateBoardChoices()
    {
        boardIds_.clear();
        boardCombo_.clear();
        int itemId = 1;
        for(const auto& registration : daisyhost::GetBoardRegistrations())
        {
            if(!registration.available)
            {
                continue;
            }
            boardIds_.push_back(registration.boardId);
            boardCombo_.addItem(registration.displayName, itemId++);
        }
    }

    void populateAppChoices()
    {
        appIds_.clear();
        appCombo_.clear();
        int itemId = 1;
        for(const auto& registration : daisyhost::GetHostedAppRegistrations())
        {
            appIds_.push_back(registration.appId);
            appCombo_.addItem(registration.displayName, itemId++);
        }
    }

    void populateActivityChoices()
    {
        activityIds_.clear();
        activityCombo_.clear();
        int itemId = 1;
        for(const auto& registration : daisyhost::GetActivityRegistrations())
        {
            activityIds_.push_back(registration.activityId);
            activityCombo_.addItem(registration.displayName, itemId++);
        }
    }

    static void layoutRow(juce::Rectangle<int> bounds,
                          juce::Component&     label,
                          juce::Component&     field)
    {
        auto labelArea = bounds.removeFromLeft(120);
        label.setBounds(labelArea);
        field.setBounds(bounds);
    }

    static void layoutPathRow(juce::Rectangle<int> bounds,
                              juce::Component&     label,
                              juce::Component&     editor,
                              juce::Component&     button)
    {
        auto labelArea  = bounds.removeFromLeft(120);
        auto buttonArea = bounds.removeFromRight(100);
        label.setBounds(labelArea);
        button.setBounds(buttonArea);
        editor.setBounds(bounds.reduced(0, 0));
    }

    std::string selectedBoardId() const
    {
        const auto index = boardCombo_.getSelectedItemIndex();
        return index >= 0 && index < static_cast<int>(boardIds_.size())
                   ? boardIds_[static_cast<std::size_t>(index)]
                   : daisyhost::GetDefaultBoardId();
    }

    std::string selectedAppId() const
    {
        const auto index = appCombo_.getSelectedItemIndex();
        return index >= 0 && index < static_cast<int>(appIds_.size())
                   ? appIds_[static_cast<std::size_t>(index)]
                   : daisyhost::GetDefaultHostedAppId();
    }

    std::string selectedActivityId() const
    {
        const auto index = activityCombo_.getSelectedItemIndex();
        return index >= 0 && index < static_cast<int>(activityIds_.size())
                   ? activityIds_[static_cast<std::size_t>(index)]
                   : daisyhost::GetDefaultActivityId();
    }

    void loadProfile()
    {
        std::string errorMessage;
        const auto  loaded = daisyhost::LoadHubProfile(profileFile_, &errorMessage);
        auto profile = loaded.value_or(daisyhost::HubProfile{
            daisyhost::GetDefaultBoardId(),
            daisyhost::GetDefaultHostedAppId(),
            daisyhost::GetDefaultActivityId(),
            {},
            {},
            {},
            0});
        profile = daisyhost::NormalizeHubProfile(profile);

        selectComboValue(boardIds_, boardCombo_, profile.boardId);
        selectComboValue(appIds_, appCombo_, profile.appId);
        selectComboValue(activityIds_, activityCombo_, profile.activityId);
        renderEditor_.setText(profile.renderScenarioPath, juce::dontSendNotification);
        datasetEditor_.setText(profile.datasetJobPath, juce::dontSendNotification);
        outputEditor_.setText(profile.outputDirectoryPath, juce::dontSendNotification);
        seedEditor_.setText(juce::String(profile.seed), juce::dontSendNotification);

        if(!errorMessage.empty())
        {
            statusLabel_.setText(errorMessage, juce::dontSendNotification);
        }
    }

    void saveProfile()
    {
        daisyhost::HubProfile profile;
        profile.boardId             = selectedBoardId();
        profile.appId               = selectedAppId();
        profile.activityId          = selectedActivityId();
        profile.renderScenarioPath  = renderEditor_.getText().toStdString();
        profile.datasetJobPath      = datasetEditor_.getText().toStdString();
        profile.outputDirectoryPath = outputEditor_.getText().toStdString();
        profile.seed = static_cast<std::uint32_t>(seedEditor_.getText().getIntValue());
        profile = daisyhost::NormalizeHubProfile(profile);

        std::string errorMessage;
        if(!daisyhost::SaveHubProfile(profileFile_, profile, &errorMessage))
        {
            statusLabel_.setText(errorMessage, juce::dontSendNotification);
        }
    }

    void refreshVisibility()
    {
        const auto activityId = selectedActivityId();
        const bool showRender = activityId == "render";
        const bool showTrain  = activityId == "train";
        const bool showOutput = showRender || showTrain;
        const bool showSeed   = showRender || showTrain;

        renderLabel_.setVisible(showRender);
        renderEditor_.setVisible(showRender);
        browseRenderButton_.setVisible(showRender);

        datasetLabel_.setVisible(showTrain);
        datasetEditor_.setVisible(showTrain);
        browseDatasetButton_.setVisible(showTrain);

        outputLabel_.setVisible(showOutput);
        outputEditor_.setVisible(showOutput);
        browseOutputButton_.setVisible(showOutput);

        seedLabel_.setVisible(showSeed);
        seedEditor_.setVisible(showSeed);
    }

    void browseForFile(juce::TextEditor& editor, const juce::String& wildcard)
    {
        activeChooser_ = std::make_unique<juce::FileChooser>("Select JSON file",
                                                             juce::File(),
                                                             wildcard);
        auto* targetEditor = &editor;
        activeChooser_->launchAsync(
            juce::FileBrowserComponent::openMode
                | juce::FileBrowserComponent::canSelectFiles,
            [this, targetEditor](const juce::FileChooser& chooser) {
                const auto result = chooser.getResult();
                if(!result.getFullPathName().isEmpty())
                {
                    targetEditor->setText(result.getFullPathName());
                }
                activeChooser_.reset();
            });
    }

    void browseForDirectory(juce::TextEditor& editor)
    {
        activeChooser_ = std::make_unique<juce::FileChooser>("Select output folder");
        auto* targetEditor = &editor;
        activeChooser_->launchAsync(
            juce::FileBrowserComponent::openMode
                | juce::FileBrowserComponent::canSelectDirectories,
            [this, targetEditor](const juce::FileChooser& chooser) {
                const auto result = chooser.getResult();
                if(!result.getFullPathName().isEmpty())
                {
                    targetEditor->setText(result.getFullPathName());
                }
                activeChooser_.reset();
            });
    }

    void launchSelected()
    {
        saveProfile();

        daisyhost::HubLaunchSelection selection;
        selection.boardId    = selectedBoardId();
        selection.appId      = selectedAppId();
        selection.activityId = selectedActivityId();
        selection.seed
            = static_cast<std::uint32_t>(seedEditor_.getText().getIntValue());

        if(!renderEditor_.getText().isEmpty())
        {
            selection.renderScenario = juce::File(renderEditor_.getText());
        }
        if(!datasetEditor_.getText().isEmpty())
        {
            selection.datasetJob = juce::File(datasetEditor_.getText());
        }
        if(!outputEditor_.getText().isEmpty())
        {
            selection.outputDirectory = juce::File(outputEditor_.getText());
        }

        daisyhost::HubLaunchPlan plan;
        std::string              errorMessage;
        if(!daisyhost::BuildHubLaunchPlan(selection, toolPaths_, &plan, &errorMessage))
        {
            statusLabel_.setText(errorMessage, juce::dontSendNotification);
            return;
        }

        if(!daisyhost::ExecuteHubLaunchPlan(plan, &errorMessage))
        {
            statusLabel_.setText(errorMessage, juce::dontSendNotification);
            return;
        }

        statusLabel_.setText("Launched " + juce::String(selectedActivityId()),
                             juce::dontSendNotification);
    }

    static void selectComboValue(const std::vector<std::string>& ids,
                                 juce::ComboBox&                combo,
                                 const std::string&             selectedId)
    {
        const auto it = std::find(ids.begin(), ids.end(), selectedId);
        combo.setSelectedItemIndex(it != ids.end()
                                       ? static_cast<int>(std::distance(ids.begin(), it))
                                       : 0,
                                   juce::dontSendNotification);
    }

    daisyhost::HubToolPaths toolPaths_;
    juce::File              profileFile_;

    juce::Label titleLabel_;
    juce::Label boardLabel_;
    juce::Label appLabel_;
    juce::Label activityLabel_;
    juce::Label renderLabel_;
    juce::Label datasetLabel_;
    juce::Label outputLabel_;
    juce::Label seedLabel_;
    juce::Label statusLabel_;

    juce::ComboBox boardCombo_;
    juce::ComboBox appCombo_;
    juce::ComboBox activityCombo_;

    juce::TextEditor renderEditor_;
    juce::TextEditor datasetEditor_;
    juce::TextEditor outputEditor_;
    juce::TextEditor seedEditor_;

    juce::TextButton browseRenderButton_;
    juce::TextButton browseDatasetButton_;
    juce::TextButton browseOutputButton_;
    juce::TextButton launchButton_;

    std::vector<std::string> boardIds_;
    std::vector<std::string> appIds_;
    std::vector<std::string> activityIds_;
    std::unique_ptr<juce::FileChooser> activeChooser_;
};
} // namespace

DaisyHostHubWindow::DaisyHostHubWindow()
    : juce::DocumentWindow("DaisyHost Hub",
                           juce::Colours::darkgrey,
                           juce::DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    setResizable(true, false);
    setContentOwned(new DaisyHostHubContent(), true);
    centreWithSize(720, 360);
    setVisible(true);
}

void DaisyHostHubWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
