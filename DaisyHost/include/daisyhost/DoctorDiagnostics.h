#pragma once

#include <string>
#include <utility>
#include <vector>

namespace daisyhost
{
struct DoctorDiagnosticsOptions
{
    std::string sourceDir = ".";
    std::string buildDir;
    std::string config = "Release";
};

using DoctorEnvironment = std::vector<std::pair<std::string, std::string>>;

struct DoctorCheckDiagnostic
{
    std::string description;
    std::string path;
    bool        required = true;
    bool        exists   = false;
    bool        ok       = false;
    std::string category;
    std::string kind;
    std::string severity;
    std::string hint;
};

struct DoctorBlockerDiagnostic
{
    std::string kind;
    std::string severity;
    std::string evidence;
    std::string hint;
};

struct DoctorReadinessSummary
{
    bool ok = false;
    int  checked = 0;
    int  failed = 0;
};

struct DoctorCtestDiagnostics
{
    bool                     ok = false;
    std::vector<std::string> expectedTests;
    std::vector<std::string> registeredTests;
    std::vector<std::string> missingTests;
};

struct DoctorEnvironmentDiagnostics
{
    bool ok = true;
    bool duplicatePathKeys = false;
    std::vector<std::string> pathKeys;
};

struct DoctorDiagnosticsResult
{
    bool                         ok = false;
    std::string                  buildDir;
    std::string                  sourceDir;
    std::string                  config;
    std::vector<DoctorCheckDiagnostic> checks;
    DoctorReadinessSummary       source;
    DoctorReadinessSummary       build;
    DoctorCtestDiagnostics       ctest;
    DoctorEnvironmentDiagnostics environment;
    std::vector<DoctorBlockerDiagnostic> blockers;
};

DoctorEnvironment CurrentDoctorEnvironment();

DoctorDiagnosticsResult BuildDoctorDiagnostics(
    const DoctorDiagnosticsOptions& options,
    const DoctorEnvironment&        environment = CurrentDoctorEnvironment());

std::string SerializeDoctorDiagnosticsPayloadJson(
    const DoctorDiagnosticsResult& result);
} // namespace daisyhost
