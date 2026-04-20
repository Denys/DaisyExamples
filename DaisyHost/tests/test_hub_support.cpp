#include <gtest/gtest.h>

#include <juce_core/juce_core.h>

#include <algorithm>

#include "daisyhost/AppRegistry.h"
#include "daisyhost/HubSupport.h"

namespace
{
struct ScopedTempDir
{
    ScopedTempDir()
        : directory(juce::File::getSpecialLocation(juce::File::tempDirectory)
                        .getChildFile("daisyhost_hub_test_"
                                      + juce::Uuid().toString()))
    {
        directory.createDirectory();
    }

    ~ScopedTempDir()
    {
        directory.deleteRecursively();
    }

    juce::File directory;
};

TEST(HubSupportTest, RegistersPatchBoardAndCoreActivities)
{
    const auto& boards = daisyhost::GetBoardRegistrations();
    ASSERT_FALSE(boards.empty());

    bool sawPatch = false;
    for(const auto& board : boards)
    {
        if(board.boardId == "daisy_patch")
        {
            sawPatch = true;
            EXPECT_TRUE(board.available);
            EXPECT_EQ(board.displayName, "Daisy Patch");
        }
    }
    EXPECT_TRUE(sawPatch);

    const auto& activities = daisyhost::GetActivityRegistrations();
    ASSERT_GE(activities.size(), 3u);

    bool sawPlay   = false;
    bool sawRender = false;
    bool sawTrain  = false;
    for(const auto& activity : activities)
    {
        if(activity.activityId == "play_test")
        {
            sawPlay = true;
        }
        if(activity.activityId == "render")
        {
            sawRender = true;
        }
        if(activity.activityId == "train")
        {
            sawTrain = true;
        }
    }

    EXPECT_TRUE(sawPlay);
    EXPECT_TRUE(sawRender);
    EXPECT_TRUE(sawTrain);
}

TEST(HubSupportTest, RejectsUnknownOrUnavailableBoardSelection)
{
    ScopedTempDir tempDir;

    daisyhost::HubToolPaths paths;
    paths.supportDirectory   = tempDir.directory;
    paths.renderExecutable   = tempDir.directory.getChildFile("DaisyHostRender.exe");
    paths.standaloneExecutable
        = tempDir.directory.getChildFile("DaisyHost Patch.exe");
    paths.trainingScript = tempDir.directory.getChildFile("render_dataset.py");

    daisyhost::HubLaunchSelection selection;
    selection.boardId    = "daisy_field";
    selection.appId      = daisyhost::GetDefaultHostedAppId();
    selection.activityId = "play_test";

    daisyhost::HubLaunchPlan plan;
    std::string              errorMessage;
    EXPECT_FALSE(
        daisyhost::BuildHubLaunchPlan(selection, paths, &plan, &errorMessage));
    EXPECT_NE(errorMessage.find("board"), std::string::npos);
}

TEST(HubSupportTest, BuildsPlayLaunchPlanWithCommandLineArgs)
{
    ScopedTempDir tempDir;

    daisyhost::HubToolPaths paths;
    paths.supportDirectory   = tempDir.directory;
    paths.renderExecutable   = tempDir.directory.getChildFile("DaisyHostRender.exe");
    paths.standaloneExecutable
        = tempDir.directory.getChildFile("DaisyHost Patch.exe");
    paths.trainingScript = tempDir.directory.getChildFile("render_dataset.py");

    daisyhost::HubLaunchSelection selection;
    selection.boardId    = "daisy_patch";
    selection.appId      = "torus";
    selection.activityId = "play_test";

    daisyhost::HubLaunchPlan plan;
    std::string              errorMessage;
    ASSERT_TRUE(
        daisyhost::BuildHubLaunchPlan(selection, paths, &plan, &errorMessage))
        << errorMessage;

    EXPECT_EQ(plan.executable,
              paths.standaloneExecutable.getFullPathName().toStdString());
    ASSERT_EQ(plan.arguments.size(), 4u);
    EXPECT_EQ(plan.arguments[0], "--board");
    EXPECT_EQ(plan.arguments[1], "daisy_patch");
    EXPECT_EQ(plan.arguments[2], "--app");
    EXPECT_EQ(plan.arguments[3], "torus");
    ASSERT_EQ(plan.cleanupFiles.size(), 1u);
    EXPECT_EQ(plan.cleanupFiles.front().getFileName().toStdString(),
              "hub_launch_request.json");
    EXPECT_TRUE(plan.generatedFiles.empty());
}

TEST(HubSupportTest, LoadAndConsumeStartupRequestClearsRequestFile)
{
    ScopedTempDir tempDir;
    const auto    requestFile = tempDir.directory.getChildFile("hub_launch_request.json");

    std::string errorMessage;
    ASSERT_TRUE(daisyhost::SaveHubStartupRequest(
        requestFile, {"daisy_patch", "torus"}, &errorMessage))
        << errorMessage;
    ASSERT_TRUE(requestFile.existsAsFile());

    const auto request
        = daisyhost::LoadAndConsumeHubStartupRequest(requestFile, &errorMessage);
    ASSERT_TRUE(request.has_value()) << errorMessage;
    EXPECT_EQ(request->boardId, "daisy_patch");
    EXPECT_EQ(request->appId, "torus");
    EXPECT_FALSE(requestFile.existsAsFile());
}

TEST(HubSupportTest, BuildsRenderLaunchPlanUsingExistingScenario)
{
    ScopedTempDir tempDir;
    const auto    scenarioFile = tempDir.directory.getChildFile("scenario.json");
    EXPECT_TRUE(scenarioFile.replaceWithText("{}"));

    daisyhost::HubToolPaths paths;
    paths.supportDirectory   = tempDir.directory;
    paths.renderExecutable   = tempDir.directory.getChildFile("DaisyHostRender.exe");
    paths.standaloneExecutable
        = tempDir.directory.getChildFile("DaisyHost Patch.exe");
    paths.trainingScript = tempDir.directory.getChildFile("render_dataset.py");

    daisyhost::HubLaunchSelection selection;
    selection.boardId        = "daisy_patch";
    selection.appId          = "multidelay";
    selection.activityId     = "render";
    selection.renderScenario = scenarioFile;
    selection.outputDirectory
        = tempDir.directory.getChildFile("render_out");

    daisyhost::HubLaunchPlan plan;
    std::string              errorMessage;
    ASSERT_TRUE(
        daisyhost::BuildHubLaunchPlan(selection, paths, &plan, &errorMessage))
        << errorMessage;

    EXPECT_EQ(plan.executable,
              paths.renderExecutable.getFullPathName().toStdString());
    ASSERT_EQ(plan.arguments.size(), 3);
    EXPECT_EQ(plan.arguments[0], scenarioFile.getFullPathName().toStdString());
    EXPECT_EQ(plan.arguments[1], "--output-dir");
    EXPECT_EQ(plan.arguments[2],
              selection.outputDirectory.getFullPathName().toStdString());
    EXPECT_TRUE(plan.generatedFiles.empty());
}

TEST(HubSupportTest, BuildsTrainLaunchPlanUsingGeneratedDatasetJob)
{
    ScopedTempDir tempDir;

    daisyhost::HubToolPaths paths;
    paths.supportDirectory   = tempDir.directory;
    paths.renderExecutable   = tempDir.directory.getChildFile("DaisyHostRender.exe");
    paths.standaloneExecutable
        = tempDir.directory.getChildFile("DaisyHost Patch.exe");
    paths.trainingScript = tempDir.directory.getChildFile("render_dataset.py");

    daisyhost::HubLaunchSelection selection;
    selection.boardId    = "daisy_patch";
    selection.appId      = "torus";
    selection.activityId = "train";
    selection.seed       = 77u;
    selection.outputDirectory
        = tempDir.directory.getChildFile("dataset_out");

    daisyhost::HubLaunchPlan plan;
    std::string              errorMessage;
    ASSERT_TRUE(
        daisyhost::BuildHubLaunchPlan(selection, paths, &plan, &errorMessage))
        << errorMessage;

    EXPECT_EQ(plan.executable, "py");
    ASSERT_EQ(plan.arguments.size(), 3);
    EXPECT_EQ(plan.arguments[0], "-3");
    EXPECT_EQ(plan.arguments[1],
              paths.trainingScript.getFullPathName().toStdString());
    EXPECT_EQ(plan.arguments[2],
              plan.generatedFiles.front().file.getFullPathName().toStdString());
    ASSERT_EQ(plan.generatedFiles.size(), 1u);
    EXPECT_NE(plan.generatedFiles.front().contents.find("\"jobName\":\"torus_dataset\""),
              std::string::npos);
    EXPECT_NE(plan.generatedFiles.front().contents.find("\"baseScenario\""),
              std::string::npos);
}

TEST(HubSupportTest, PersistsProfileAndFallsBackToDefaultsForInvalidSelections)
{
    ScopedTempDir tempDir;
    const auto    profileFile = tempDir.directory.getChildFile("hub_profile.json");

    daisyhost::HubProfile profile;
    profile.boardId            = "invalid_board";
    profile.appId              = "invalid_app";
    profile.activityId         = "invalid_activity";
    profile.renderScenarioPath = "C:/tmp/example.json";
    profile.datasetJobPath     = "C:/tmp/job.json";
    profile.outputDirectoryPath = "C:/tmp/out";
    profile.seed               = 123u;

    std::string errorMessage;
    ASSERT_TRUE(daisyhost::SaveHubProfile(profileFile, profile, &errorMessage))
        << errorMessage;

    auto loaded = daisyhost::LoadHubProfile(profileFile, &errorMessage);
    ASSERT_TRUE(loaded.has_value()) << errorMessage;
    EXPECT_EQ(loaded->renderScenarioPath, profile.renderScenarioPath);
    EXPECT_EQ(loaded->datasetJobPath, profile.datasetJobPath);
    EXPECT_EQ(loaded->outputDirectoryPath, profile.outputDirectoryPath);
    EXPECT_EQ(loaded->seed, 123u);

    const auto normalized = daisyhost::NormalizeHubProfile(*loaded);
    EXPECT_EQ(normalized.boardId, "daisy_patch");
    EXPECT_EQ(normalized.appId, daisyhost::GetDefaultHostedAppId());
    EXPECT_EQ(normalized.activityId, "play_test");
}

TEST(HubSupportTest, DiscoversStandaloneAndRenderToolsFromHubExecutablePath)
{
    const auto currentExecutable = juce::File(
        "C:/repo/DaisyHost/build/DaisyHostHub_artefacts/Release/DaisyHost Hub.exe");

    const auto paths = daisyhost::DiscoverDefaultHubToolPaths(currentExecutable);

    const auto normalizedRender
        = paths.renderExecutable.getFullPathName().replaceCharacter('\\', '/');
    const auto normalizedStandalone
        = paths.standaloneExecutable.getFullPathName().replaceCharacter('\\', '/');

    EXPECT_EQ(normalizedRender.toStdString(),
              "C:/repo/DaisyHost/build/Release/DaisyHostRender.exe");
    EXPECT_EQ(normalizedStandalone.toStdString(),
              "C:/repo/DaisyHost/build/DaisyHostPatch_artefacts/Release/Standalone/DaisyHost Patch.exe");
}
} // namespace
