#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <juce_core/juce_core.h>

namespace daisyhost
{
enum class HubLaunchMode
{
    kExecutable,
    kPython,
};

struct BoardRegistration
{
    std::string boardId;
    std::string displayName;
    bool        available = true;
};

struct ActivityRegistration
{
    std::string  activityId;
    std::string  displayName;
    HubLaunchMode launchMode = HubLaunchMode::kExecutable;
    bool         requiresOutputDirectory = false;
    bool         acceptsConfigPath       = false;
    bool         supportsGeneratedConfig = false;
};

struct HubToolPaths
{
    juce::File supportDirectory;
    juce::File sourceRoot;
    juce::File standaloneExecutable;
    juce::File renderExecutable;
    juce::File trainingScript;
};

struct HubLaunchSelection
{
    std::string boardId;
    std::string appId;
    std::string activityId;
    juce::File  renderScenario;
    juce::File  datasetJob;
    juce::File  outputDirectory;
    std::uint32_t seed = 0;
};

struct HubGeneratedFile
{
    juce::File  file;
    std::string contents;
};

struct HubLaunchPlan
{
    HubLaunchMode               launchMode = HubLaunchMode::kExecutable;
    std::string                 executable;
    std::vector<std::string>    arguments;
    std::vector<juce::File>     cleanupFiles;
    std::vector<HubGeneratedFile> generatedFiles;
};

struct HubProfile
{
    std::string boardId;
    std::string appId;
    std::string activityId;
    std::string renderScenarioPath;
    std::string datasetJobPath;
    std::string outputDirectoryPath;
    std::uint32_t seed = 0;
};

struct HubStartupRequest
{
    std::string boardId;
    std::string appId;
};

const std::vector<BoardRegistration>& GetBoardRegistrations();
const std::vector<ActivityRegistration>& GetActivityRegistrations();
std::string GetDefaultBoardId();
std::string GetDefaultActivityId();

HubProfile NormalizeHubProfile(const HubProfile& profile);
bool SaveHubProfile(const juce::File& file,
                    const HubProfile& profile,
                    std::string*      errorMessage = nullptr);
std::optional<HubProfile> LoadHubProfile(const juce::File& file,
                                         std::string*      errorMessage = nullptr);

juce::File GetDefaultHubSupportDirectory();
juce::File GetDefaultHubProfileFile();
juce::File GetDefaultHubLaunchRequestFile();

std::string SerializeHubStartupRequest(const HubStartupRequest& request);
std::optional<HubStartupRequest> ParseHubStartupRequest(
    const std::string& text,
    std::string*       errorMessage = nullptr);
bool SaveHubStartupRequest(const juce::File&    file,
                           const HubStartupRequest& request,
                           std::string*         errorMessage = nullptr);
std::optional<HubStartupRequest> LoadHubStartupRequest(
    const juce::File& file,
    std::string*      errorMessage = nullptr);
std::optional<HubStartupRequest> LoadAndConsumeHubStartupRequest(
    const juce::File& file,
    std::string*      errorMessage = nullptr);
bool ClearHubStartupRequest(const juce::File& file,
                            std::string*      errorMessage = nullptr);

HubToolPaths DiscoverDefaultHubToolPaths(const juce::File& currentExecutableFile);

bool BuildHubLaunchPlan(const HubLaunchSelection& selection,
                        const HubToolPaths&       paths,
                        HubLaunchPlan*            plan,
                        std::string*              errorMessage = nullptr);

bool ExecuteHubLaunchPlan(const HubLaunchPlan& plan,
                          std::string*         errorMessage = nullptr);
} // namespace daisyhost
