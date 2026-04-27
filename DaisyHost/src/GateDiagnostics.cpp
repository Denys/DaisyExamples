#include "daisyhost/GateDiagnostics.h"

#include <algorithm>
#include <regex>
#include <sstream>

#include <juce_core/juce_core.h>

namespace daisyhost
{
namespace
{
std::vector<std::string> DefaultGateTargets()
{
    return {"unit_tests",
            "DaisyHostCLI",
            "DaisyHostHub",
            "DaisyHostRender",
            "DaisyHostPatch_VST3",
            "DaisyHostPatch_Standalone"};
}

std::string Tail(const std::string& output, std::size_t limit)
{
    if(output.size() <= limit)
    {
        return output;
    }
    return output.substr(output.size() - limit);
}

std::vector<std::string> Lines(const std::string& output)
{
    std::vector<std::string> lines;
    std::istringstream       stream(output);
    std::string              line;
    while(std::getline(stream, line))
    {
        if(!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        lines.push_back(line);
    }
    return lines;
}

bool ContainsCaseSensitive(const std::string& text, const std::string& needle)
{
    return text.find(needle) != std::string::npos;
}

std::string HintForKind(const std::string& kind)
{
    if(kind == "duplicate-path-env")
    {
        return "Use build_host.cmd or normalize the process to a single Path environment variable.";
    }
    if(kind == "locked-artifact")
    {
        return "Close the running DaisyHost executable and rerun the gate.";
    }
    if(kind == "missing-include")
    {
        return "Check whether the referenced header is missing, untracked, or absent from the include path.";
    }
    if(kind == "missing-member")
    {
        return "Check whether tests and implementation disagree on the current API.";
    }
    if(kind == "ctest-failure")
    {
        return "Inspect the failed CTest names and rerun the smallest matching test filter.";
    }
    if(kind == "cmake-configure-failure")
    {
        return "Inspect CMake configuration output before rebuilding.";
    }
    if(kind == "build-failure")
    {
        return "Inspect the first compiler or linker error above the build failure.";
    }
    if(kind == "timeout")
    {
        return "Increase the timeout only after checking for a stuck process or locked artifact.";
    }
    return "Inspect outputTail for the underlying command failure.";
}

void AddBlocker(std::vector<GateBlockerDiagnostic>* blockers,
                const std::string&                 kind,
                const std::string&                 evidence)
{
    if(blockers == nullptr)
    {
        return;
    }
    const auto duplicate = std::find_if(
        blockers->begin(),
        blockers->end(),
        [&](const GateBlockerDiagnostic& blocker) {
            return blocker.kind == kind;
        });
    if(duplicate != blockers->end())
    {
        return;
    }
    blockers->push_back({kind, "error", evidence, HintForKind(kind)});
}

GateCtestDiagnostic ParseCtest(const std::vector<std::string>& lines)
{
    GateCtestDiagnostic diagnostic;
    const std::regex summaryRegex(
        R"((\d+)% tests passed,\s*(\d+) tests failed out of\s*(\d+))");
    const std::regex failedTestRegex(R"(^\s*\d+\s*-\s*([^\(]+)\s*\(Failed\))");

    for(const auto& line : lines)
    {
        std::smatch match;
        if(std::regex_search(line, match, summaryRegex) && match.size() == 4)
        {
            diagnostic.failed = std::stoi(match[2].str());
            diagnostic.total  = std::stoi(match[3].str());
            diagnostic.passed = diagnostic.total - diagnostic.failed;
            if(diagnostic.passed < 0)
            {
                diagnostic.passed = 0;
            }
            continue;
        }

        if(std::regex_search(line, match, failedTestRegex) && match.size() == 2)
        {
            auto name = match[1].str();
            while(!name.empty()
                  && (name.back() == ' ' || name.back() == '\t'))
            {
                name.pop_back();
            }
            diagnostic.failedTests.push_back(name);
        }
    }
    return diagnostic;
}

std::vector<GatePhaseDiagnostic> BuildPhases(const std::vector<std::string>& lines,
                                             int exitCode)
{
    std::vector<GatePhaseDiagnostic> phases{{"configure", "not-run"},
                                            {"build", "not-run"},
                                            {"ctest", "not-run"}};
    std::vector<bool> started(phases.size(), false);
    int               lastStarted = -1;
    for(const auto& line : lines)
    {
        if(line.rfind("+ cmake -S", 0) == 0)
        {
            started[0] = true;
            lastStarted = 0;
        }
        else if(line.rfind("+ cmake --build", 0) == 0)
        {
            started[1] = true;
            lastStarted = 1;
        }
        else if(line.rfind("+ ctest", 0) == 0)
        {
            started[2] = true;
            lastStarted = 2;
        }
    }

    if(lastStarted < 0)
    {
        return phases;
    }

    if(exitCode == 0)
    {
        for(std::size_t index = 0; index < started.size(); ++index)
        {
            if(started[index])
            {
                phases[index].status = "passed";
            }
        }
        return phases;
    }

    for(int index = 0; index <= lastStarted; ++index)
    {
        if(!started[static_cast<std::size_t>(index)])
        {
            continue;
        }
        phases[static_cast<std::size_t>(index)].status
            = index == lastStarted ? "failed" : "passed";
    }
    return phases;
}

std::string FailedPhaseKind(const std::vector<GatePhaseDiagnostic>& phases)
{
    for(const auto& phase : phases)
    {
        if(phase.status != "failed")
        {
            continue;
        }
        if(phase.name == "configure")
        {
            return "cmake-configure-failure";
        }
        if(phase.name == "build")
        {
            return "build-failure";
        }
        if(phase.name == "ctest")
        {
            return "ctest-failure";
        }
    }
    return {};
}

std::vector<GateBlockerDiagnostic> ClassifyBlockers(
    const std::vector<std::string>&       lines,
    const GateCtestDiagnostic&            ctest,
    const std::vector<GatePhaseDiagnostic>& phases,
    int                                  exitCode)
{
    std::vector<GateBlockerDiagnostic> blockers;
    for(const auto& line : lines)
    {
        if(ContainsCaseSensitive(line, "Path")
           && ContainsCaseSensitive(line, "PATH")
           && (ContainsCaseSensitive(line, "duplicate")
               || ContainsCaseSensitive(line, "Env:")
               || ContainsCaseSensitive(line, "environment")))
        {
            AddBlocker(&blockers, "duplicate-path-env", line);
        }
        if(ContainsCaseSensitive(line, "LNK1104")
           || (ContainsCaseSensitive(line, "cannot open file")
               && (ContainsCaseSensitive(line, ".exe")
                   || ContainsCaseSensitive(line, ".dll"))))
        {
            AddBlocker(&blockers, "locked-artifact", line);
        }
        if(ContainsCaseSensitive(line, "fatal error C1083")
           && ContainsCaseSensitive(line, "Cannot open include file"))
        {
            AddBlocker(&blockers, "missing-include", line);
        }
        if(ContainsCaseSensitive(line, "error C2039")
           && ContainsCaseSensitive(line, "is not a member"))
        {
            AddBlocker(&blockers, "missing-member", line);
        }
        if(ContainsCaseSensitive(line, "timed out")
           || ContainsCaseSensitive(line, "timeout"))
        {
            AddBlocker(&blockers, "timeout", line);
        }
    }

    if(ctest.failed > 0)
    {
        AddBlocker(&blockers,
                   "ctest-failure",
                   std::to_string(ctest.failed)
                       + " tests failed out of " + std::to_string(ctest.total));
    }

    if(exitCode != 0 && blockers.empty())
    {
        const auto phaseKind = FailedPhaseKind(phases);
        if(!phaseKind.empty())
        {
            AddBlocker(&blockers, phaseKind, "Gate phase failed: " + phaseKind);
        }
    }
    if(exitCode != 0 && blockers.empty())
    {
        AddBlocker(&blockers,
                   "unknown-runtime-failure",
                   lines.empty() ? "Gate command failed" : lines.back());
    }
    return blockers;
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

juce::var PhasesVar(const std::vector<GatePhaseDiagnostic>& phases)
{
    juce::Array<juce::var> array;
    for(const auto& phase : phases)
    {
        auto object = std::make_unique<juce::DynamicObject>();
        object->setProperty("name", juce::String(phase.name));
        object->setProperty("status", juce::String(phase.status));
        array.add(juce::var(object.release()));
    }
    return juce::var(array);
}

juce::var BlockersVar(const std::vector<GateBlockerDiagnostic>& blockers)
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

juce::var CtestVar(const GateCtestDiagnostic& ctest)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("passed", ctest.passed);
    object->setProperty("failed", ctest.failed);
    object->setProperty("total", ctest.total);
    object->setProperty("failedTests", StringArrayVar(ctest.failedTests));
    return juce::var(object.release());
}
} // namespace

GateDiagnosticsResult BuildGateDiagnostics(
    const GateDiagnosticsOptions& options,
    const std::string&            command,
    int                           exitCode,
    const std::string&            output,
    std::size_t                   outputTailLimit)
{
    const auto lines = Lines(output);

    GateDiagnosticsResult result;
    result.ok         = exitCode == 0;
    result.command    = command;
    result.exitCode   = exitCode;
    result.sourceDir  = options.sourceDir;
    result.buildDir   = options.buildDir;
    result.config     = options.config;
    result.targets    = DefaultGateTargets();
    result.phases     = BuildPhases(lines, exitCode);
    result.ctest      = ParseCtest(lines);
    result.blockers   = ClassifyBlockers(lines, result.ctest, result.phases, exitCode);
    result.outputTail = Tail(output, outputTailLimit);
    return result;
}

std::string SerializeGateDiagnosticsPayloadJson(
    const GateDiagnosticsResult& result)
{
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("ok", result.ok);
    root->setProperty("command", juce::String(result.command));
    root->setProperty("exitCode", result.exitCode);
    root->setProperty("sourceDir", juce::String(result.sourceDir));
    root->setProperty("buildDir", juce::String(result.buildDir));
    root->setProperty("config", juce::String(result.config));
    root->setProperty("phases", PhasesVar(result.phases));
    root->setProperty("targets", StringArrayVar(result.targets));
    root->setProperty("ctest", CtestVar(result.ctest));
    root->setProperty("blockers", BlockersVar(result.blockers));
    root->setProperty("outputTail", juce::String(result.outputTail));
    return juce::JSON::toString(juce::var(root.release()), true).toStdString();
}
} // namespace daisyhost
