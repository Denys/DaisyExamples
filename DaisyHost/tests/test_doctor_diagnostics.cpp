#include <gtest/gtest.h>
#include <juce_core/juce_core.h>

#include "daisyhost/DoctorDiagnostics.h"

#include <filesystem>
#include <fstream>

namespace
{
juce::var ParseJson(const std::string& json)
{
    return juce::JSON::parse(juce::String(json));
}

class TempTree
{
public:
    TempTree()
        : root_(std::filesystem::temp_directory_path()
                / std::filesystem::path(
                    "daisyhost_doctor_test_"
                    + std::to_string(
                        std::chrono::steady_clock::now()
                            .time_since_epoch()
                            .count())))
    {
        std::filesystem::create_directories(root_);
    }

    ~TempTree()
    {
        std::error_code error;
        std::filesystem::remove_all(root_, error);
    }

    std::filesystem::path root() const { return root_; }

    std::filesystem::path source() const { return root_ / "source"; }

    std::filesystem::path build() const { return root_ / "build"; }

    void file(const std::filesystem::path& path,
              const std::string&           contents = {})
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream stream(path, std::ios::binary);
        stream << contents;
    }

    void directory(const std::filesystem::path& path)
    {
        std::filesystem::create_directories(path);
    }

private:
    std::filesystem::path root_;
};

void PopulateReadyTree(TempTree& tree)
{
    tree.file(tree.source() / "CMakeLists.txt");
    tree.file(tree.source() / "build_host.cmd");
    tree.file(tree.source() / "build_host.ps1");
    tree.file(tree.source() / "tests" / "run_smoke.py");
    tree.directory(tree.source() / "training" / "examples");

    tree.file(tree.build() / "CMakeCache.txt");
    tree.file(tree.build() / "CTestTestfile.cmake",
              "add_test([=[DaisyHostNextWpSuggester]=] \"py\")\n"
              "add_test([=[DaisyHostStandaloneSmoke]=] \"py\")\n"
              "add_test([=[DaisyHostRenderSmoke]=] \"py\")\n"
              "add_test([=[DaisyHostCliDoctor]=] \"DaisyHostCLI\")\n"
              "add_test([=[DaisyHostCliRender]=] \"DaisyHostCLI\")\n");
    tree.directory(tree.build() / "Release");
    tree.file(tree.build() / "Release" / "DaisyHostCLI.exe");
    tree.file(tree.build() / "Release" / "DaisyHostRender.exe");
    tree.file(tree.build() / "DaisyHostHub_artefacts" / "Release"
              / "DaisyHost Hub.exe");
    tree.file(tree.build() / "DaisyHostPatch_artefacts" / "Release"
              / "Standalone" / "DaisyHost Patch.exe");
    tree.directory(tree.build() / "DaisyHostPatch_artefacts" / "Release"
                   / "VST3" / "DaisyHost Patch.vst3");
}

daisyhost::DoctorDiagnosticsOptions OptionsFor(const TempTree& tree)
{
    daisyhost::DoctorDiagnosticsOptions options;
    options.sourceDir = tree.source().string();
    options.buildDir  = tree.build().string();
    options.config    = "Release";
    return options;
}

daisyhost::DoctorEnvironment CleanEnvironment()
{
    daisyhost::DoctorEnvironment environment;
    environment.emplace_back("Path", "C:\\tools");
    return environment;
}

bool HasBlocker(const daisyhost::DoctorDiagnosticsResult& result,
                const std::string&                        kind)
{
    for(const auto& blocker : result.blockers)
    {
        if(blocker.kind == kind)
        {
            return true;
        }
    }
    return false;
}
} // namespace

TEST(DoctorDiagnosticsTest, SerializesBackwardCompatibleReadyPayload)
{
    TempTree tree;
    PopulateReadyTree(tree);

    const auto result
        = daisyhost::BuildDoctorDiagnostics(OptionsFor(tree), CleanEnvironment());

    ASSERT_TRUE(result.ok);
    EXPECT_TRUE(result.blockers.empty());
    ASSERT_FALSE(result.checks.empty());

    const auto parsed
        = ParseJson(daisyhost::SerializeDoctorDiagnosticsPayloadJson(result));
    auto* root = parsed.getDynamicObject();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(static_cast<bool>(root->getProperty("ok")), true);
    EXPECT_EQ(root->getProperty("buildDir").toString(),
              juce::String(tree.build().generic_string()));
    EXPECT_EQ(root->getProperty("sourceDir").toString(),
              juce::String(tree.source().generic_string()));
    EXPECT_EQ(root->getProperty("config").toString(), "Release");

    auto* checks = root->getProperty("checks").getArray();
    ASSERT_NE(checks, nullptr);
    ASSERT_FALSE(checks->isEmpty());
    auto* first = checks->getReference(0).getDynamicObject();
    ASSERT_NE(first, nullptr);
    EXPECT_TRUE(first->hasProperty("description"));
    EXPECT_TRUE(first->hasProperty("path"));
    EXPECT_TRUE(first->hasProperty("required"));
    EXPECT_TRUE(first->hasProperty("exists"));
    EXPECT_TRUE(first->hasProperty("ok"));
    EXPECT_TRUE(first->hasProperty("category"));
    EXPECT_TRUE(first->hasProperty("kind"));
    EXPECT_TRUE(first->hasProperty("severity"));
    EXPECT_TRUE(first->hasProperty("hint"));
    EXPECT_TRUE(root->hasProperty("source"));
    EXPECT_TRUE(root->hasProperty("build"));
    EXPECT_TRUE(root->hasProperty("ctest"));
    EXPECT_TRUE(root->hasProperty("environment"));
    EXPECT_TRUE(root->hasProperty("blockers"));
}

TEST(DoctorDiagnosticsTest, MissingCMakeListsReportsSourceFailure)
{
    TempTree tree;
    PopulateReadyTree(tree);
    std::filesystem::remove(tree.source() / "CMakeLists.txt");

    const auto result
        = daisyhost::BuildDoctorDiagnostics(OptionsFor(tree), CleanEnvironment());

    EXPECT_FALSE(result.ok);
    EXPECT_TRUE(HasBlocker(result, "missing-source"));
}

TEST(DoctorDiagnosticsTest, MissingBuildHostScriptsReportSourceFailure)
{
    TempTree tree;
    PopulateReadyTree(tree);
    std::filesystem::remove(tree.source() / "build_host.cmd");
    std::filesystem::remove(tree.source() / "build_host.ps1");

    const auto result
        = daisyhost::BuildDoctorDiagnostics(OptionsFor(tree), CleanEnvironment());

    EXPECT_FALSE(result.ok);
    EXPECT_TRUE(HasBlocker(result, "missing-source"));
}

TEST(DoctorDiagnosticsTest, MissingBuildTreeReportsBuildFailure)
{
    TempTree tree;
    PopulateReadyTree(tree);
    std::filesystem::remove(tree.build() / "CMakeCache.txt");
    std::filesystem::remove(tree.build() / "CTestTestfile.cmake");

    const auto result
        = daisyhost::BuildDoctorDiagnostics(OptionsFor(tree), CleanEnvironment());

    EXPECT_FALSE(result.ok);
    EXPECT_TRUE(HasBlocker(result, "missing-build-tree"));
}

TEST(DoctorDiagnosticsTest, MissingHubAndVst3ArtifactsReportArtifactFailure)
{
    TempTree tree;
    PopulateReadyTree(tree);
    std::filesystem::remove(tree.build() / "DaisyHostHub_artefacts" / "Release"
                            / "DaisyHost Hub.exe");
    std::filesystem::remove_all(tree.build() / "DaisyHostPatch_artefacts"
                                / "Release" / "VST3"
                                / "DaisyHost Patch.vst3");

    const auto result
        = daisyhost::BuildDoctorDiagnostics(OptionsFor(tree), CleanEnvironment());

    EXPECT_FALSE(result.ok);
    EXPECT_TRUE(HasBlocker(result, "missing-artifact"));
}

TEST(DoctorDiagnosticsTest, MissingExpectedCTestRegistrationReportsFailure)
{
    TempTree tree;
    PopulateReadyTree(tree);
    tree.file(tree.build() / "CTestTestfile.cmake",
              "add_test([=[DaisyHostCliRender]=] \"DaisyHostCLI\")\n");

    const auto result
        = daisyhost::BuildDoctorDiagnostics(OptionsFor(tree), CleanEnvironment());

    EXPECT_FALSE(result.ok);
    EXPECT_TRUE(HasBlocker(result, "missing-ctest-registration"));
    EXPECT_FALSE(result.ctest.missingTests.empty());
}

TEST(DoctorDiagnosticsTest, DuplicatePathEnvironmentReportsHazard)
{
    TempTree tree;
    PopulateReadyTree(tree);
    daisyhost::DoctorEnvironment environment;
    environment.emplace_back("Path", "C:\\tools");
    environment.emplace_back("PATH", "C:\\other");

    const auto result
        = daisyhost::BuildDoctorDiagnostics(OptionsFor(tree), environment);

    EXPECT_FALSE(result.ok);
    EXPECT_TRUE(HasBlocker(result, "duplicate-path-env"));
    EXPECT_TRUE(result.environment.duplicatePathKeys);
}

TEST(DoctorDiagnosticsTest, UnsupportedConfigReportsReadinessFailure)
{
    TempTree tree;
    PopulateReadyTree(tree);
    auto options = OptionsFor(tree);
    options.config = "Profile";

    const auto result
        = daisyhost::BuildDoctorDiagnostics(options, CleanEnvironment());

    EXPECT_FALSE(result.ok);
    EXPECT_TRUE(HasBlocker(result, "unsupported-config"));
}
