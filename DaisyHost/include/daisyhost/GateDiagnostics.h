#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace daisyhost
{
struct GateDiagnosticsOptions
{
    std::string sourceDir;
    std::string buildDir;
    std::string config = "Release";
};

struct GatePhaseDiagnostic
{
    std::string name;
    std::string status;
};

struct GateBlockerDiagnostic
{
    std::string kind;
    std::string severity;
    std::string evidence;
    std::string hint;
};

struct GateCtestDiagnostic
{
    int                      passed = 0;
    int                      failed = 0;
    int                      total  = 0;
    std::vector<std::string> failedTests;
};

struct GateDiagnosticsResult
{
    bool                              ok       = false;
    std::string                       command;
    int                               exitCode = 0;
    std::string                       sourceDir;
    std::string                       buildDir;
    std::string                       config;
    std::vector<GatePhaseDiagnostic>  phases;
    std::vector<std::string>          targets;
    GateCtestDiagnostic               ctest;
    std::vector<GateBlockerDiagnostic> blockers;
    std::string                       outputTail;
};

GateDiagnosticsResult BuildGateDiagnostics(
    const GateDiagnosticsOptions& options,
    const std::string&            command,
    int                           exitCode,
    const std::string&            output,
    std::size_t                   outputTailLimit = 16000);

std::string SerializeGateDiagnosticsPayloadJson(
    const GateDiagnosticsResult& result);
} // namespace daisyhost
