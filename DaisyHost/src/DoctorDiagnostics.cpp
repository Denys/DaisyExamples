#include "daisyhost/DoctorDiagnostics.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>

#include <juce_core/juce_core.h>

#if !defined(_WIN32)
extern char** environ;
#endif

namespace daisyhost
{
namespace
{
bool IsSupportedConfig(const std::string& config)
{
    return config == "Debug" || config == "Release"
           || config == "RelWithDebInfo" || config == "MinSizeRel";
}

std::string Lowercase(std::string value)
{
    std::transform(value.begin(),
                   value.end(),
                   value.begin(),
                   [](unsigned char c) {
                       return static_cast<char>(std::tolower(c));
                   });
    return value;
}

std::string HintForKind(const std::string& kind)
{
    if(kind == "duplicate-path-env")
    {
        return "Use build_host.cmd or normalize the process to a single Path environment variable before raw MSBuild commands.";
    }
    if(kind == "missing-source")
    {
        return "Check --source-dir and make sure it points at the DaisyHost source root.";
    }
    if(kind == "missing-build-tree")
    {
        return "Run CMake configure for this source/build pair before running smoke or gate checks.";
    }
    if(kind == "missing-artifact")
    {
        return "Build the expected DaisyHost targets for this configuration.";
    }
    if(kind == "missing-ctest-registration")
    {
        return "Reconfigure CMake so the expected DaisyHost CTest entries are registered.";
    }
    if(kind == "unsupported-config")
    {
        return "Use Debug, Release, RelWithDebInfo, or MinSizeRel.";
    }
    return "Inspect the reported readiness check.";
}

void AddBlocker(std::vector<DoctorBlockerDiagnostic>* blockers,
                const std::string&                   kind,
                const std::string&                   evidence)
{
    if(blockers == nullptr)
    {
        return;
    }
    const auto existing = std::find_if(
        blockers->begin(),
        blockers->end(),
        [&](const DoctorBlockerDiagnostic& blocker) {
            return blocker.kind == kind;
        });
    if(existing != blockers->end())
    {
        return;
    }
    blockers->push_back({kind, "error", evidence, HintForKind(kind)});
}

DoctorCheckDiagnostic MakeCheck(const std::string&           description,
                                const std::filesystem::path& path,
                                bool                         required,
                                const std::string&           category,
                                const std::string&           kind)
{
    const bool exists = std::filesystem::exists(path);
    return {description,
            path.generic_string(),
            required,
            exists,
            exists || !required,
            category,
            kind,
            (exists || !required) ? "info" : "error",
            (exists || !required) ? "" : HintForKind(kind)};
}

DoctorCheckDiagnostic MakeVirtualCheck(const std::string& description,
                                       const std::string& value,
                                       bool              ok,
                                       const std::string& category,
                                       const std::string& kind)
{
    return {description,
            value,
            true,
            true,
            ok,
            category,
            kind,
            ok ? "info" : "error",
            ok ? "" : HintForKind(kind)};
}

void AddCheck(DoctorDiagnosticsResult*      result,
              const DoctorCheckDiagnostic& check)
{
    if(result == nullptr)
    {
        return;
    }
    result->checks.push_back(check);
    if(!check.ok)
    {
        AddBlocker(&result->blockers, check.kind, check.description);
    }
}

DoctorReadinessSummary SummarizeChecks(
    const std::vector<DoctorCheckDiagnostic>& checks,
    const std::string&                        category)
{
    DoctorReadinessSummary summary;
    summary.ok = true;
    for(const auto& check : checks)
    {
        if(check.category != category)
        {
            continue;
        }
        ++summary.checked;
        if(!check.ok)
        {
            ++summary.failed;
            summary.ok = false;
        }
    }
    if(summary.checked == 0)
    {
        summary.ok = true;
    }
    return summary;
}

std::vector<std::string> ExpectedCTestNames()
{
    return {"DaisyHostNextWpSuggester",
            "DaisyHostStandaloneSmoke",
            "DaisyHostRenderSmoke",
            "DaisyHostCliDoctor",
            "DaisyHostCliRender",
            "DaisyHostCliRenderAssertions",
            "DaisyHostCliRenderAssertionsPass"};
}

std::string ReadFileIfPresent(const std::filesystem::path& path)
{
    std::ifstream stream(path, std::ios::binary);
    if(!stream)
    {
        return {};
    }
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

DoctorCtestDiagnostics BuildCtestDiagnostics(
    const std::filesystem::path& ctestFile)
{
    DoctorCtestDiagnostics ctest;
    ctest.expectedTests = ExpectedCTestNames();
    const auto contents = ReadFileIfPresent(ctestFile);
    for(const auto& name : ctest.expectedTests)
    {
        if(contents.find(name) == std::string::npos)
        {
            ctest.missingTests.push_back(name);
        }
        else
        {
            ctest.registeredTests.push_back(name);
        }
    }
    ctest.ok = ctest.missingTests.empty();
    return ctest;
}

DoctorEnvironmentDiagnostics BuildEnvironmentDiagnostics(
    const DoctorEnvironment& environment)
{
    DoctorEnvironmentDiagnostics diagnostics;
    std::set<std::string>        uniqueSpellings;
    for(const auto& entry : environment)
    {
        if(Lowercase(entry.first) == "path")
        {
            uniqueSpellings.insert(entry.first);
        }
    }
    diagnostics.pathKeys.assign(uniqueSpellings.begin(), uniqueSpellings.end());
    diagnostics.duplicatePathKeys = diagnostics.pathKeys.size() > 1;
    diagnostics.ok = !diagnostics.duplicatePathKeys;
    return diagnostics;
}

juce::var StringArrayVar(const std::vector<std::string>& values)
{
    juce::Array<juce::var> array;
    for(const auto& value : values)
    {
        array.add(juce::String(value));
    }
    return juce::var(array);
}

juce::var CheckVar(const DoctorCheckDiagnostic& check)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("description", juce::String(check.description));
    object->setProperty("path", juce::String(check.path));
    object->setProperty("required", check.required);
    object->setProperty("exists", check.exists);
    object->setProperty("ok", check.ok);
    object->setProperty("category", juce::String(check.category));
    object->setProperty("kind", juce::String(check.kind));
    object->setProperty("severity", juce::String(check.severity));
    object->setProperty("hint", juce::String(check.hint));
    return juce::var(object.release());
}

juce::var ChecksVar(const std::vector<DoctorCheckDiagnostic>& checks)
{
    juce::Array<juce::var> array;
    for(const auto& check : checks)
    {
        array.add(CheckVar(check));
    }
    return juce::var(array);
}

juce::var SummaryVar(const DoctorReadinessSummary& summary)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("ok", summary.ok);
    object->setProperty("checked", summary.checked);
    object->setProperty("failed", summary.failed);
    return juce::var(object.release());
}

juce::var CtestVar(const DoctorCtestDiagnostics& ctest)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("ok", ctest.ok);
    object->setProperty("expectedTests", StringArrayVar(ctest.expectedTests));
    object->setProperty("registeredTests",
                        StringArrayVar(ctest.registeredTests));
    object->setProperty("missingTests", StringArrayVar(ctest.missingTests));
    return juce::var(object.release());
}

juce::var EnvironmentVar(const DoctorEnvironmentDiagnostics& environment)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("ok", environment.ok);
    object->setProperty("duplicatePathKeys", environment.duplicatePathKeys);
    object->setProperty("pathKeys", StringArrayVar(environment.pathKeys));
    return juce::var(object.release());
}

juce::var BlockersVar(const std::vector<DoctorBlockerDiagnostic>& blockers)
{
    juce::Array<juce::var> array;
    for(const auto& blocker : blockers)
    {
        auto object = std::make_unique<juce::DynamicObject>();
        object->setProperty("kind", juce::String(blocker.kind));
        object->setProperty("severity", juce::String(blocker.severity));
        object->setProperty("evidence", juce::String(blocker.evidence));
        object->setProperty("hint", juce::String(blocker.hint));
        array.add(juce::var(object.release()));
    }
    return juce::var(array);
}
} // namespace

DoctorEnvironment CurrentDoctorEnvironment()
{
    DoctorEnvironment environment;
#if defined(_WIN32)
    char** entries = _environ;
#else
    char** entries = environ;
#endif
    if(entries == nullptr)
    {
        return environment;
    }
    for(char** entry = entries; *entry != nullptr; ++entry)
    {
        std::string text(*entry);
        const auto  separator = text.find('=');
        if(separator == std::string::npos)
        {
            continue;
        }
        environment.emplace_back(text.substr(0, separator),
                                 text.substr(separator + 1));
    }
    return environment;
}

DoctorDiagnosticsResult BuildDoctorDiagnostics(
    const DoctorDiagnosticsOptions& options,
    const DoctorEnvironment&        environment)
{
    DoctorDiagnosticsResult result;
    result.buildDir  = std::filesystem::path(options.buildDir).generic_string();
    result.sourceDir = std::filesystem::path(options.sourceDir).generic_string();
    result.config    = options.config;

    const auto sourceDir = std::filesystem::path(options.sourceDir);
    const auto buildDir  = std::filesystem::path(options.buildDir);

    AddCheck(&result,
             MakeVirtualCheck("configuration",
                              options.config,
                              IsSupportedConfig(options.config),
                              "build",
                              "unsupported-config"));

    AddCheck(&result,
             MakeCheck("source directory",
                       sourceDir,
                       true,
                       "source",
                       "missing-source"));
    AddCheck(&result,
             MakeCheck("CMakeLists.txt",
                       sourceDir / "CMakeLists.txt",
                       true,
                       "source",
                       "missing-source"));
    AddCheck(&result,
             MakeCheck("build_host.cmd",
                       sourceDir / "build_host.cmd",
                       true,
                       "source",
                       "missing-source"));
    AddCheck(&result,
             MakeCheck("build_host.ps1",
                       sourceDir / "build_host.ps1",
                       true,
                       "source",
                       "missing-source"));
    AddCheck(&result,
             MakeCheck("smoke script",
                       sourceDir / "tests" / "run_smoke.py",
                       true,
                       "source",
                       "missing-source"));
    AddCheck(&result,
             MakeCheck("training examples directory",
                       sourceDir / "training" / "examples",
                       true,
                       "source",
                       "missing-source"));

    AddCheck(&result,
             MakeCheck("build directory",
                       buildDir,
                       true,
                       "build",
                       "missing-build-tree"));
    AddCheck(&result,
             MakeCheck("CMake cache",
                       buildDir / "CMakeCache.txt",
                       true,
                       "build",
                       "missing-build-tree"));
    AddCheck(&result,
             MakeCheck("CTest registry",
                       buildDir / "CTestTestfile.cmake",
                       true,
                       "build",
                       "missing-build-tree"));
    AddCheck(&result,
             MakeCheck("configuration output directory",
                       buildDir / options.config,
                       true,
                       "build",
                       "missing-build-tree"));

    AddCheck(&result,
             MakeCheck("DaisyHostCLI executable",
                       buildDir / options.config / "DaisyHostCLI.exe",
                       true,
                       "artifact",
                       "missing-artifact"));
    AddCheck(&result,
             MakeCheck("DaisyHostRender executable",
                       buildDir / options.config / "DaisyHostRender.exe",
                       true,
                       "artifact",
                       "missing-artifact"));
    AddCheck(&result,
             MakeCheck("DaisyHost Hub executable",
                       buildDir / "DaisyHostHub_artefacts" / options.config
                           / "DaisyHost Hub.exe",
                       true,
                       "artifact",
                       "missing-artifact"));
    AddCheck(&result,
             MakeCheck("DaisyHost Patch standalone executable",
                       buildDir / "DaisyHostPatch_artefacts" / options.config
                           / "Standalone" / "DaisyHost Patch.exe",
                       true,
                       "artifact",
                       "missing-artifact"));
    AddCheck(&result,
             MakeCheck("DaisyHost Patch VST3 bundle",
                       buildDir / "DaisyHostPatch_artefacts" / options.config
                           / "VST3" / "DaisyHost Patch.vst3",
                       true,
                       "artifact",
                       "missing-artifact"));

    result.ctest
        = BuildCtestDiagnostics(buildDir / "CTestTestfile.cmake");
    if(!result.ctest.ok)
    {
        for(const auto& missing : result.ctest.missingTests)
        {
            AddBlocker(&result.blockers,
                       "missing-ctest-registration",
                       missing);
        }
    }

    result.environment = BuildEnvironmentDiagnostics(environment);
    if(!result.environment.ok)
    {
        AddBlocker(&result.blockers,
                   "duplicate-path-env",
                   "Both Path and PATH are present in the process environment.");
    }

    result.source = SummarizeChecks(result.checks, "source");
    result.build  = SummarizeChecks(result.checks, "build");

    result.ok = result.source.ok && result.build.ok && result.ctest.ok
                && result.environment.ok && result.blockers.empty();
    for(const auto& check : result.checks)
    {
        result.ok = result.ok && check.ok;
    }

    return result;
}

std::string SerializeDoctorDiagnosticsPayloadJson(
    const DoctorDiagnosticsResult& result)
{
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("ok", result.ok);
    root->setProperty("buildDir", juce::String(result.buildDir));
    root->setProperty("sourceDir", juce::String(result.sourceDir));
    root->setProperty("config", juce::String(result.config));
    root->setProperty("checks", ChecksVar(result.checks));
    root->setProperty("source", SummaryVar(result.source));
    root->setProperty("build", SummaryVar(result.build));
    root->setProperty("ctest", CtestVar(result.ctest));
    root->setProperty("environment", EnvironmentVar(result.environment));
    root->setProperty("blockers", BlockersVar(result.blockers));
    return juce::JSON::toString(juce::var(root.release()), true).toStdString();
}
} // namespace daisyhost
