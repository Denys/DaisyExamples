#include "daisyhost/HubSupport.h"

#include <algorithm>

#include <juce_core/juce_core.h>

#include "daisyhost/AppRegistry.h"

#if JUCE_WINDOWS
#include <windows.h>
#include <shellapi.h>
#endif

namespace daisyhost
{
namespace
{
juce::var ToVar(const std::string& text)
{
    return juce::var(juce::String(text));
}

std::string ToCompactJsonString(const juce::var& value)
{
    auto text = juce::JSON::toString(value, false).toStdString();
    for(std::string::size_type position = 0;
        (position = text.find(": ", position)) != std::string::npos;)
    {
        text.erase(position + 1, 1);
    }
    for(std::string::size_type position = 0;
        (position = text.find(", ", position)) != std::string::npos;)
    {
        text.erase(position + 1, 1);
    }
    return text;
}

const std::vector<BoardRegistration> kBoardRegistrations = {
    {"daisy_patch", "Daisy Patch", true},
};

const std::vector<ActivityRegistration> kActivityRegistrations = {
    {"play_test", "Play / Test", HubLaunchMode::kExecutable, false, false, false},
    {"render", "Render", HubLaunchMode::kExecutable, true, true, true},
    {"train", "Train", HubLaunchMode::kPython, true, true, true},
};

const BoardRegistration* FindBoardRegistration(const std::string& boardId)
{
    const auto& registrations = GetBoardRegistrations();
    const auto  it = std::find_if(registrations.begin(),
                                 registrations.end(),
                                 [&boardId](const BoardRegistration& registration) {
                                     return registration.boardId == boardId;
                                 });
    return it != registrations.end() ? &(*it) : nullptr;
}

const ActivityRegistration* FindActivityRegistration(const std::string& activityId)
{
    const auto& registrations = GetActivityRegistrations();
    const auto  it
        = std::find_if(registrations.begin(),
                       registrations.end(),
                       [&activityId](const ActivityRegistration& registration) {
                           return registration.activityId == activityId;
                       });
    return it != registrations.end() ? &(*it) : nullptr;
}

bool IsKnownAppId(const std::string& appId)
{
    const auto& registrations = GetHostedAppRegistrations();
    return std::any_of(registrations.begin(),
                       registrations.end(),
                       [&appId](const HostedAppRegistration& registration) {
                           return registration.appId == appId;
                       });
}

juce::var HubProfileToVar(const HubProfile& profile)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("boardId", ToVar(profile.boardId));
    object->setProperty("appId", ToVar(profile.appId));
    object->setProperty("activityId", ToVar(profile.activityId));
    object->setProperty("renderScenarioPath", ToVar(profile.renderScenarioPath));
    object->setProperty("datasetJobPath", ToVar(profile.datasetJobPath));
    object->setProperty("outputDirectoryPath", ToVar(profile.outputDirectoryPath));
    object->setProperty("seed", static_cast<juce::int64>(profile.seed));
    return juce::var(object.release());
}

std::optional<HubProfile> HubProfileFromVar(const juce::var& value,
                                            std::string*     errorMessage)
{
    if(auto* object = value.getDynamicObject())
    {
        HubProfile profile;
        profile.boardId = object->getProperty("boardId").toString().toStdString();
        profile.appId = object->getProperty("appId").toString().toStdString();
        profile.activityId
            = object->getProperty("activityId").toString().toStdString();
        profile.renderScenarioPath
            = object->getProperty("renderScenarioPath").toString().toStdString();
        profile.datasetJobPath
            = object->getProperty("datasetJobPath").toString().toStdString();
        profile.outputDirectoryPath
            = object->getProperty("outputDirectoryPath").toString().toStdString();
        profile.seed = static_cast<std::uint32_t>(
            static_cast<juce::int64>(object->getProperty("seed")));
        return profile;
    }

    if(errorMessage != nullptr)
    {
        *errorMessage = "Hub profile JSON must be an object";
    }
    return std::nullopt;
}

std::string BuildRenderScenarioJson(const HubLaunchSelection& selection)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("boardId", ToVar(selection.boardId));
    object->setProperty("appId", ToVar(selection.appId));

    auto renderConfig = std::make_unique<juce::DynamicObject>();
    renderConfig->setProperty("sampleRate", 48000.0);
    renderConfig->setProperty("blockSize", 48);
    renderConfig->setProperty("durationSeconds", 1.0);
    renderConfig->setProperty("outputChannelCount", 2);
    object->setProperty("renderConfig", juce::var(renderConfig.release()));
    object->setProperty("seed", static_cast<juce::int64>(selection.seed));

    auto initialParameters = std::make_unique<juce::DynamicObject>();
    object->setProperty("initialParameterValues",
                        juce::var(initialParameters.release()));

    auto audioInput = std::make_unique<juce::DynamicObject>();
    audioInput->setProperty("mode", juce::var("sine"));
    audioInput->setProperty("level", 4.0);
    audioInput->setProperty("frequencyHz", 220.0);
    object->setProperty("audioInput", juce::var(audioInput.release()));

    object->setProperty("timeline", juce::Array<juce::var>());
    return ToCompactJsonString(juce::var(object.release()));
}

std::string BuildDatasetJobJson(const HubLaunchSelection& selection)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("jobName", ToVar(selection.appId + "_dataset"));
    object->setProperty("boardId", ToVar(selection.boardId));
    object->setProperty(
        "outputDir",
        ToVar(selection.outputDirectory.getFullPathName().toStdString()));

    auto baseScenario = std::make_unique<juce::DynamicObject>();
    baseScenario->setProperty("boardId", ToVar(selection.boardId));
    baseScenario->setProperty("appId", ToVar(selection.appId));

    auto renderConfig = std::make_unique<juce::DynamicObject>();
    renderConfig->setProperty("sampleRate", 48000.0);
    renderConfig->setProperty("blockSize", 48);
    renderConfig->setProperty("durationSeconds", 1.0);
    renderConfig->setProperty("outputChannelCount", 2);
    baseScenario->setProperty("renderConfig", juce::var(renderConfig.release()));
    baseScenario->setProperty("seed", static_cast<juce::int64>(selection.seed));

    auto initialParameters = std::make_unique<juce::DynamicObject>();
    baseScenario->setProperty("initialParameterValues",
                              juce::var(initialParameters.release()));

    auto audioInput = std::make_unique<juce::DynamicObject>();
    audioInput->setProperty("mode", juce::var("sine"));
    audioInput->setProperty("level", 4.0);
    audioInput->setProperty("frequencyHz", 220.0);
    baseScenario->setProperty("audioInput", juce::var(audioInput.release()));
    baseScenario->setProperty("timeline", juce::Array<juce::var>());

    object->setProperty("baseScenario", juce::var(baseScenario.release()));

    juce::Array<juce::var> sweeps;
    auto                   seedSweep = std::make_unique<juce::DynamicObject>();
    seedSweep->setProperty("kind", juce::var("seed"));
    juce::Array<juce::var> seedValues;
    seedValues.add(static_cast<juce::int64>(selection.seed));
    sweeps.add(juce::var(seedSweep.release()));
    sweeps.getReference(0).getDynamicObject()->setProperty("values", seedValues);
    object->setProperty("sweeps", sweeps);

    return ToCompactJsonString(juce::var(object.release()));
}

std::string QuoteArgument(const std::string& argument)
{
    if(argument.find_first_of(" \t\"") == std::string::npos)
    {
        return argument;
    }

    std::string escaped = "\"";
    for(const char character : argument)
    {
        if(character == '"')
        {
            escaped += '\\';
        }
        escaped += character;
    }
    escaped += '"';
    return escaped;
}

bool WriteGeneratedFiles(const HubLaunchPlan& plan, std::string* errorMessage)
{
    for(const auto& generatedFile : plan.generatedFiles)
    {
        if(!generatedFile.file.getParentDirectory().createDirectory())
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Failed to create " + generatedFile.file.getParentDirectory()
                                                         .getFullPathName()
                                                         .toStdString();
            }
            return false;
        }

        if(!generatedFile.file.replaceWithText(generatedFile.contents))
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Failed to write "
                                + generatedFile.file.getFullPathName().toStdString();
            }
            return false;
        }
    }

    return true;
}
} // namespace

const std::vector<BoardRegistration>& GetBoardRegistrations()
{
    return kBoardRegistrations;
}

const std::vector<ActivityRegistration>& GetActivityRegistrations()
{
    return kActivityRegistrations;
}

std::string GetDefaultBoardId()
{
    return "daisy_patch";
}

std::string GetDefaultActivityId()
{
    return "play_test";
}

HubProfile NormalizeHubProfile(const HubProfile& profile)
{
    HubProfile normalized = profile;
    if(const auto* board = FindBoardRegistration(normalized.boardId);
       board == nullptr || !board->available)
    {
        normalized.boardId = GetDefaultBoardId();
    }

    if(!IsKnownAppId(normalized.appId))
    {
        normalized.appId = GetDefaultHostedAppId();
    }

    if(FindActivityRegistration(normalized.activityId) == nullptr)
    {
        normalized.activityId = GetDefaultActivityId();
    }

    return normalized;
}

bool SaveHubProfile(const juce::File& file,
                    const HubProfile& profile,
                    std::string*      errorMessage)
{
    if(!file.getParentDirectory().createDirectory())
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Failed to create profile directory";
        }
        return false;
    }

    if(!file.replaceWithText(
           juce::JSON::toString(HubProfileToVar(profile), true)))
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Failed to write hub profile";
        }
        return false;
    }

    return true;
}

std::optional<HubProfile> LoadHubProfile(const juce::File& file,
                                         std::string*      errorMessage)
{
    if(!file.existsAsFile())
    {
        return HubProfile{GetDefaultBoardId(),
                          GetDefaultHostedAppId(),
                          GetDefaultActivityId(),
                          {},
                          {},
                          {},
                          0};
    }

    const auto parsed = juce::JSON::parse(file);
    if(parsed.isVoid())
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Failed to parse hub profile JSON";
        }
        return std::nullopt;
    }

    return HubProfileFromVar(parsed, errorMessage);
}

juce::File GetDefaultHubSupportDirectory()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("DaisyHost");
}

juce::File GetDefaultHubProfileFile()
{
    return GetDefaultHubSupportDirectory().getChildFile("hub_profile.json");
}

juce::File GetDefaultHubLaunchRequestFile()
{
    return GetDefaultHubSupportDirectory().getChildFile("hub_launch_request.json");
}

std::string SerializeHubStartupRequest(const HubStartupRequest& request)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("boardId", ToVar(request.boardId));
    object->setProperty("appId", ToVar(request.appId));
    return ToCompactJsonString(juce::var(object.release()));
}

std::optional<HubStartupRequest> ParseHubStartupRequest(
    const std::string& text,
    std::string*       errorMessage)
{
    const auto parsed = juce::JSON::parse(text);
    if(parsed.isVoid())
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Failed to parse hub launch request";
        }
        return std::nullopt;
    }

    if(auto* object = parsed.getDynamicObject())
    {
        HubStartupRequest request;
        request.boardId = object->getProperty("boardId").toString().toStdString();
        request.appId   = object->getProperty("appId").toString().toStdString();
        return request;
    }

    if(errorMessage != nullptr)
    {
        *errorMessage = "Hub launch request JSON must be an object";
    }
    return std::nullopt;
}

bool SaveHubStartupRequest(const juce::File&       file,
                           const HubStartupRequest& request,
                           std::string*            errorMessage)
{
    if(!file.getParentDirectory().createDirectory())
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Failed to create launch request directory";
        }
        return false;
    }

    if(!file.replaceWithText(SerializeHubStartupRequest(request)))
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Failed to write hub launch request";
        }
        return false;
    }

    return true;
}

std::optional<HubStartupRequest> LoadHubStartupRequest(
    const juce::File& file,
    std::string*      errorMessage)
{
    if(!file.existsAsFile())
    {
        return std::nullopt;
    }

    return ParseHubStartupRequest(file.loadFileAsString().toStdString(),
                                  errorMessage);
}

std::optional<HubStartupRequest> LoadAndConsumeHubStartupRequest(
    const juce::File& file,
    std::string*      errorMessage)
{
    if(!file.existsAsFile())
    {
        return std::nullopt;
    }

    const auto text = file.loadFileAsString().toStdString();
    auto       request = ParseHubStartupRequest(text, errorMessage);

    std::string clearError;
    if(!ClearHubStartupRequest(file, &clearError) && errorMessage != nullptr
       && errorMessage->empty())
    {
        *errorMessage = clearError;
    }

    return request;
}

bool ClearHubStartupRequest(const juce::File& file, std::string* errorMessage)
{
    if(!file.exists())
    {
        return true;
    }

    if(file.deleteFile())
    {
        return true;
    }

    if(errorMessage != nullptr)
    {
        *errorMessage = "Failed to delete hub launch request";
    }
    return false;
}

HubToolPaths DiscoverDefaultHubToolPaths(const juce::File& currentExecutableFile)
{
    HubToolPaths paths;
    paths.supportDirectory = GetDefaultHubSupportDirectory();
    const auto executableDir = currentExecutableFile.getParentDirectory();
    const auto artefactsDir  = executableDir.getParentDirectory();
    const auto buildDir      = artefactsDir.getParentDirectory();
    paths.renderExecutable   = buildDir.getChildFile("Release/DaisyHostRender.exe");
    paths.standaloneExecutable = buildDir.getChildFile(
        "DaisyHostPatch_artefacts/Release/Standalone/DaisyHost Patch.exe");
#ifdef DAISYHOST_SOURCE_DIR
    paths.sourceRoot = juce::File(DAISYHOST_SOURCE_DIR);
    paths.trainingScript
        = paths.sourceRoot.getChildFile("training/render_dataset.py");
#endif
    return paths;
}

bool BuildHubLaunchPlan(const HubLaunchSelection& selection,
                        const HubToolPaths&       paths,
                        HubLaunchPlan*            plan,
                        std::string*              errorMessage)
{
    if(plan == nullptr)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Hub launch plan output is null";
        }
        return false;
    }

    const std::string boardId
        = selection.boardId.empty() ? GetDefaultBoardId() : selection.boardId;
    const auto* board = FindBoardRegistration(boardId);
    if(board == nullptr || !board->available)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Unknown or unavailable board: " + boardId;
        }
        return false;
    }

    const std::string appId
        = selection.appId.empty() ? GetDefaultHostedAppId() : selection.appId;
    if(!IsKnownAppId(appId))
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Unknown app: " + appId;
        }
        return false;
    }

    const std::string activityId = selection.activityId.empty()
                                       ? GetDefaultActivityId()
                                       : selection.activityId;
    const auto* activity = FindActivityRegistration(activityId);
    if(activity == nullptr)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Unknown activity: " + activityId;
        }
        return false;
    }

    HubLaunchPlan builtPlan;
    builtPlan.launchMode = activity->launchMode;

    if(activityId == "play_test")
    {
        builtPlan.executable
            = paths.standaloneExecutable.getFullPathName().toStdString();
        builtPlan.arguments.push_back("--board");
        builtPlan.arguments.push_back(boardId);
        builtPlan.arguments.push_back("--app");
        builtPlan.arguments.push_back(appId);
        builtPlan.cleanupFiles.push_back(
            paths.supportDirectory.getChildFile("hub_launch_request.json"));
    }
    else if(activityId == "render")
    {
        builtPlan.executable
            = paths.renderExecutable.getFullPathName().toStdString();
        const auto outputDirectory = !selection.outputDirectory.getFullPathName().isEmpty()
                                         ? selection.outputDirectory
                                         : paths.supportDirectory.getChildFile(
                                               "render_out/" + appId);

        juce::File scenarioFile = selection.renderScenario;
        if(!scenarioFile.existsAsFile() && paths.sourceRoot.isDirectory())
        {
            const auto exampleScenario = paths.sourceRoot.getChildFile(
                "training/examples/" + appId + "_smoke.json");
            if(exampleScenario.existsAsFile())
            {
                scenarioFile = exampleScenario;
            }
        }

        if(!scenarioFile.existsAsFile())
        {
            scenarioFile = paths.supportDirectory.getChildFile(
                "generated_" + appId + "_render.json");
            builtPlan.generatedFiles.push_back(
                {scenarioFile, BuildRenderScenarioJson({boardId,
                                                        appId,
                                                        activityId,
                                                        {},
                                                        {},
                                                        outputDirectory,
                                                        selection.seed})});
        }

        builtPlan.arguments.push_back(
            scenarioFile.getFullPathName().toStdString());
        builtPlan.arguments.push_back("--output-dir");
        builtPlan.arguments.push_back(
            outputDirectory.getFullPathName().toStdString());
    }
    else if(activityId == "train")
    {
        builtPlan.executable = "py";
        builtPlan.arguments.push_back("-3");
        builtPlan.arguments.push_back(
            paths.trainingScript.getFullPathName().toStdString());

        const auto outputDirectory = !selection.outputDirectory.getFullPathName().isEmpty()
                                         ? selection.outputDirectory
                                         : paths.supportDirectory.getChildFile(
                                               "dataset_out/" + appId);
        juce::File datasetJobFile = selection.datasetJob;
        if(!datasetJobFile.existsAsFile())
        {
            datasetJobFile = paths.supportDirectory.getChildFile(
                "generated_" + appId + "_dataset.json");
            builtPlan.generatedFiles.push_back(
                {datasetJobFile, BuildDatasetJobJson({boardId,
                                                      appId,
                                                      activityId,
                                                      {},
                                                      {},
                                                      outputDirectory,
                                                      selection.seed})});
        }

        builtPlan.arguments.push_back(
            datasetJobFile.getFullPathName().toStdString());
    }

    *plan = std::move(builtPlan);
    return true;
}

bool ExecuteHubLaunchPlan(const HubLaunchPlan& plan, std::string* errorMessage)
{
    if(plan.executable.empty())
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Hub launch plan executable is empty";
        }
        return false;
    }

    if(!WriteGeneratedFiles(plan, errorMessage))
    {
        return false;
    }

    for(const auto& file : plan.cleanupFiles)
    {
        if(file.exists())
        {
            file.deleteFile();
        }
    }

    std::string parameterString;
    for(std::size_t index = 0; index < plan.arguments.size(); ++index)
    {
        if(index > 0)
        {
            parameterString += ' ';
        }
        parameterString += QuoteArgument(plan.arguments[index]);
    }

#if JUCE_WINDOWS
    const auto executable = juce::String(plan.executable).toWideCharPointer();
    const auto parameters = juce::String(parameterString).toWideCharPointer();
    const auto result = reinterpret_cast<std::uintptr_t>(ShellExecuteW(
        nullptr,
        L"open",
        executable,
        parameterString.empty() ? nullptr : parameters,
        nullptr,
        SW_SHOWNORMAL));
    if(result <= 32u)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Failed to launch " + plan.executable;
        }
        return false;
    }

    return true;
#else
    juce::ignoreUnused(parameterString);
    if(errorMessage != nullptr)
    {
        *errorMessage = "Hub launching is only implemented on Windows";
    }
    return false;
#endif
}
} // namespace daisyhost
