#include <gtest/gtest.h>
#include <juce_core/juce_core.h>

#include "daisyhost/GateDiagnostics.h"

namespace
{
juce::var ParseJson(const std::string& json)
{
    return juce::JSON::parse(juce::String(json));
}

const daisyhost::GateDiagnosticsOptions kOptions{".", "build", "Release"};

} // namespace

TEST(GateDiagnosticsTest, ClassifiesLockedArtifactFailures)
{
    const std::string output
        = "+ cmake --build build --config Release --target unit_tests "
          "DaisyHostCLI DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 "
          "DaisyHostPatch_Standalone\n"
          "LINK : fatal error LNK1104: cannot open file "
          "'build\\DaisyHostHub_artefacts\\Release\\DaisyHost Hub.exe'\n";

    const auto result = daisyhost::BuildGateDiagnostics(
        kOptions, "cmd /c build_host.cmd", 1, output);

    ASSERT_FALSE(result.ok);
    ASSERT_EQ(result.blockers.size(), 1u);
    EXPECT_EQ(result.blockers[0].kind, "locked-artifact");
    EXPECT_NE(result.blockers[0].hint.find("Close the running DaisyHost"),
              std::string::npos);
}

TEST(GateDiagnosticsTest, ClassifiesMissingIncludeFailures)
{
    const std::string output
        = "+ cmake --build build --config Debug --target unit_tests\n"
          "test.cpp(3,10): fatal error C1083: Cannot open include file: "
          "'daisyhost/HostModulationUiText.h': No such file or directory\n";

    const auto result = daisyhost::BuildGateDiagnostics(
        kOptions, "cmd /c build_host.cmd", 1, output);

    ASSERT_FALSE(result.ok);
    ASSERT_EQ(result.blockers.size(), 1u);
    EXPECT_EQ(result.blockers[0].kind, "missing-include");
}

TEST(GateDiagnosticsTest, ClassifiesMissingMemberFailures)
{
    const std::string output
        = "+ cmake --build build --config Debug --target unit_tests\n"
          "test.cpp(42,13): error C2039: 'GetFieldKeyLedValues': is not a "
          "member of 'daisyhost::SubharmoniqCore'\n";

    const auto result = daisyhost::BuildGateDiagnostics(
        kOptions, "cmd /c build_host.cmd", 1, output);

    ASSERT_FALSE(result.ok);
    ASSERT_EQ(result.blockers.size(), 1u);
    EXPECT_EQ(result.blockers[0].kind, "missing-member");
}

TEST(GateDiagnosticsTest, ParsesCtestSummaryAndFailedTests)
{
    const std::string output
        = "+ ctest --test-dir build -C Release --output-on-failure\n"
          "95% tests passed, 2 tests failed out of 40\n"
          "The following tests FAILED:\n"
          "\t12 - RenderRuntimeTest.RendersRack (Failed)\n"
          "\t13 - DaisyHostCliRender (Failed)\n";

    const auto result = daisyhost::BuildGateDiagnostics(
        kOptions, "cmd /c build_host.cmd", 8, output);

    EXPECT_EQ(result.ctest.passed, 38);
    EXPECT_EQ(result.ctest.failed, 2);
    EXPECT_EQ(result.ctest.total, 40);
    ASSERT_EQ(result.ctest.failedTests.size(), 2u);
    EXPECT_EQ(result.ctest.failedTests[0], "RenderRuntimeTest.RendersRack");
    ASSERT_FALSE(result.blockers.empty());
    EXPECT_EQ(result.blockers[0].kind, "ctest-failure");
}

TEST(GateDiagnosticsTest, DetectsPhaseStatuses)
{
    const std::string output
        = "+ cmake -S C:/repo/DaisyHost -B C:/repo/DaisyHost/build\n"
          "+ cmake --build C:/repo/DaisyHost/build --config Release --target "
          "unit_tests DaisyHostCLI DaisyHostHub DaisyHostRender "
          "DaisyHostPatch_VST3 DaisyHostPatch_Standalone\n"
          "LINK : fatal error LNK1104: cannot open file 'DaisyHost Hub.exe'\n";

    const auto result = daisyhost::BuildGateDiagnostics(
        kOptions, "cmd /c build_host.cmd", 1, output);

    ASSERT_EQ(result.phases.size(), 3u);
    EXPECT_EQ(result.phases[0].name, "configure");
    EXPECT_EQ(result.phases[0].status, "passed");
    EXPECT_EQ(result.phases[1].name, "build");
    EXPECT_EQ(result.phases[1].status, "failed");
    EXPECT_EQ(result.phases[2].name, "ctest");
    EXPECT_EQ(result.phases[2].status, "not-run");
}

TEST(GateDiagnosticsTest, SkippedPhasesStayNotRun)
{
    const std::string output
        = "+ cmake --build C:/repo/DaisyHost/build --config Release --target "
          "unit_tests DaisyHostCLI\n"
          "LINK : fatal error LNK1104: cannot open file 'DaisyHost Patch.exe'\n";

    const auto result = daisyhost::BuildGateDiagnostics(
        kOptions, "cmd /c build_host.cmd", 1, output);

    ASSERT_EQ(result.phases.size(), 3u);
    EXPECT_EQ(result.phases[0].name, "configure");
    EXPECT_EQ(result.phases[0].status, "not-run");
    EXPECT_EQ(result.phases[1].name, "build");
    EXPECT_EQ(result.phases[1].status, "failed");
    EXPECT_EQ(result.phases[2].name, "ctest");
    EXPECT_EQ(result.phases[2].status, "not-run");
}

TEST(GateDiagnosticsTest, EmitsUnknownRuntimeFailureForUnclassifiedFailures)
{
    const auto result = daisyhost::BuildGateDiagnostics(
        kOptions, "cmd /c build_host.cmd", 1, "unexpected failure\n");

    ASSERT_FALSE(result.ok);
    ASSERT_EQ(result.blockers.size(), 1u);
    EXPECT_EQ(result.blockers[0].kind, "unknown-runtime-failure");
}

TEST(GateDiagnosticsTest, SerializesSuccessfulPayloadWithoutBlockers)
{
    const std::string output
        = "+ cmake -S . -B build\n"
          "+ cmake --build build --config Release --target unit_tests "
          "DaisyHostCLI DaisyHostHub DaisyHostRender DaisyHostPatch_VST3 "
          "DaisyHostPatch_Standalone\n"
          "+ ctest --test-dir build -C Release --output-on-failure\n"
          "100% tests passed, 0 tests failed out of 232\n";

    const auto result = daisyhost::BuildGateDiagnostics(
        kOptions, "cmd /c build_host.cmd", 0, output);

    EXPECT_TRUE(result.ok);
    EXPECT_TRUE(result.blockers.empty());
    EXPECT_EQ(result.ctest.passed, 232);
    EXPECT_EQ(result.ctest.failed, 0);
    EXPECT_EQ(result.ctest.total, 232);

    auto parsed = ParseJson(daisyhost::SerializeGateDiagnosticsPayloadJson(result));
    auto* root = parsed.getDynamicObject();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(static_cast<bool>(root->getProperty("ok")), true);
    EXPECT_EQ(root->getProperty("config").toString(), "Release");
    auto* blockers = root->getProperty("blockers").getArray();
    ASSERT_NE(blockers, nullptr);
    EXPECT_EQ(blockers->size(), 0);
}
