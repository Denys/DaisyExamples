#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <juce_core/juce_core.h>

#include "daisyhost/CliPayloads.h"
#include "daisyhost/GateDiagnostics.h"
#include "daisyhost/RenderRuntime.h"

namespace
{
enum ExitCode
{
    kSuccess = 0,
    kUsageError = 1,
    kValidationFailure = 2,
    kRuntimeFailure = 3,
};

void PrintUsage()
{
    std::cerr
        << "Usage: DaisyHostCLI <command> [options]\n"
        << "\n"
        << "Commands:\n"
        << "  list-apps [--json]\n"
        << "  describe-app <appId> [--json]\n"
        << "  list-boards [--json]\n"
        << "  describe-board <boardId> [--json]\n"
        << "  list-inputs [--json]\n"
        << "  validate-scenario <scenario.json> [--json]\n"
        << "  render <scenario.json> --output-dir <directory> [--json]\n"
        << "  snapshot --app <appId> --board <boardId> [--selected-node node0] [--json]\n"
        << "  smoke --mode render|standalone|all --build-dir <dir> --source-dir <dir> [--config Release] [--json]\n"
        << "  gate --source-dir <dir> --build-dir <dir> [--config Release] [--skip-configure] [--skip-build] [--skip-tests] [--json]\n"
        << "  doctor --build-dir <dir> [--source-dir <dir>] [--config Release] [--json]\n";
}

bool HasFlag(const std::vector<std::string>& args, const std::string& flag)
{
    for(const auto& arg : args)
    {
        if(arg == flag)
        {
            return true;
        }
    }
    return false;
}

bool ReadOption(const std::vector<std::string>& args,
                const std::string&              option,
                std::string*                    value)
{
    for(std::size_t index = 0; index < args.size(); ++index)
    {
        if(args[index] == option)
        {
            if(index + 1 >= args.size())
            {
                return false;
            }
            *value = args[index + 1];
            return true;
        }
    }
    return false;
}

std::string QuoteArgument(const std::string& argument)
{
    if(argument.find_first_of(" \t\"") == std::string::npos)
    {
        return argument;
    }

    std::string quoted = "\"";
    for(const char ch : argument)
    {
        if(ch == '"')
        {
            quoted += '\\';
        }
        quoted += ch;
    }
    quoted += '"';
    return quoted;
}

std::string StatusJson(bool ok,
                       const std::string& command,
                       const std::string& message,
                       const std::string& extraKey = {},
                       const std::string& extraValue = {})
{
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("ok", ok);
    root->setProperty("command", juce::String(command));
    root->setProperty("message", juce::String(message));
    if(!extraKey.empty())
    {
        root->setProperty(juce::String(extraKey), juce::String(extraValue));
    }
    return juce::JSON::toString(juce::var(root.release()), true).toStdString();
}

int PrintPayload(const std::string& json, bool jsonOutput, const std::string& text)
{
    if(jsonOutput)
    {
        std::cout << json << '\n';
    }
    else
    {
        std::cout << text << '\n';
    }
    return kSuccess;
}

std::string BuildCommandLine(const std::vector<std::string>& command)
{
    std::string result;
    for(std::size_t index = 0; index < command.size(); ++index)
    {
        if(index > 0)
        {
            result += ' ';
        }
        result += QuoteArgument(command[index]);
    }
    return result;
}

struct ExternalCommandResult
{
    int         exitCode = -1;
    std::string commandLine;
    std::string output;
    bool        started = false;
};

ExternalCommandResult RunExternalCommand(
    const std::vector<std::string>& command)
{
    ExternalCommandResult result;
    result.commandLine = BuildCommandLine(command);
    const auto capturedCommandLine = result.commandLine + " 2>&1";

#if defined(_WIN32)
    FILE* pipe = _popen(capturedCommandLine.c_str(), "r");
#else
    FILE* pipe = popen(capturedCommandLine.c_str(), "r");
#endif
    if(pipe == nullptr)
    {
        return result;
    }

    result.started = true;
    char buffer[4096];
    while(fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result.output += buffer;
    }

#if defined(_WIN32)
    result.exitCode = _pclose(pipe);
#else
    result.exitCode = pclose(pipe);
#endif
    return result;
}

int RunSmokeCommand(const std::vector<std::string>& command,
                    bool                            jsonOutput)
{
    const auto result = RunExternalCommand(command);
    if(!result.started)
    {
        std::cerr << "Failed to start smoke command\n";
        return kRuntimeFailure;
    }

    if(jsonOutput)
    {
        auto root = std::make_unique<juce::DynamicObject>();
        root->setProperty("ok", result.exitCode == 0);
        root->setProperty("exitCode", result.exitCode);
        root->setProperty("command", juce::String(result.commandLine));
        root->setProperty("output", juce::String(result.output));
        std::cout << juce::JSON::toString(juce::var(root.release()), true)
                  << '\n';
    }
    else
    {
        std::cout << result.output;
    }

    return result.exitCode == 0 ? kSuccess : kRuntimeFailure;
}

bool IsSupportedConfig(const std::string& config)
{
    return config == "Debug" || config == "Release"
           || config == "RelWithDebInfo" || config == "MinSizeRel";
}

void PrintGateTextSummary(const daisyhost::GateDiagnosticsResult& result)
{
    std::cout << result.outputTail;
    if(!result.outputTail.empty() && result.outputTail.back() != '\n')
    {
        std::cout << '\n';
    }
    std::cout << (result.ok ? "gate passed" : "gate failed") << '\n';
    for(const auto& blocker : result.blockers)
    {
        std::cout << blocker.kind << ": " << blocker.hint << '\n';
    }
}

int RunGate(const std::vector<std::string>& args)
{
    const bool jsonOutput = HasFlag(args, "--json");
    std::string sourceDirText;
    std::string buildDirText;
    std::string config = "Release";

    if(!ReadOption(args, "--source-dir", &sourceDirText)
       || sourceDirText.empty())
    {
        std::cerr << "gate requires --source-dir <dir>\n";
        return kUsageError;
    }
    if(!ReadOption(args, "--build-dir", &buildDirText)
       || buildDirText.empty())
    {
        std::cerr << "gate requires --build-dir <dir>\n";
        return kUsageError;
    }
    ReadOption(args, "--config", &config);
    if(!IsSupportedConfig(config))
    {
        std::cerr << "gate requires --config Debug|Release|RelWithDebInfo|MinSizeRel\n";
        return kUsageError;
    }

    const auto sourceDir = std::filesystem::path(sourceDirText);
    const auto scriptPath = sourceDir / "build_host.cmd";
    daisyhost::GateDiagnosticsOptions options;
    options.sourceDir = sourceDirText;
    options.buildDir = buildDirText;
    options.config = config;

    std::vector<std::string> gateCommand{
#if defined(_WIN32)
        "cmd",
        "/c",
        "call",
        scriptPath.string(),
#else
        scriptPath.string(),
#endif
        "-Configuration",
        config,
        "-BuildDir",
        buildDirText,
    };
    if(HasFlag(args, "--skip-configure"))
    {
        gateCommand.emplace_back("-SkipConfigure");
    }
    if(HasFlag(args, "--skip-build"))
    {
        gateCommand.emplace_back("-SkipBuild");
    }
    if(HasFlag(args, "--skip-tests"))
    {
        gateCommand.emplace_back("-SkipTests");
    }

    const auto commandLine = BuildCommandLine(gateCommand);
    if(!std::filesystem::exists(scriptPath))
    {
        const auto diagnostics = daisyhost::BuildGateDiagnostics(
            options,
            commandLine,
            1,
            "Missing build_host.cmd at " + scriptPath.string());
        if(jsonOutput)
        {
            std::cout << daisyhost::SerializeGateDiagnosticsPayloadJson(
                             diagnostics)
                      << '\n';
        }
        else
        {
            PrintGateTextSummary(diagnostics);
        }
        return kRuntimeFailure;
    }

    const auto result = RunExternalCommand(gateCommand);
    if(!result.started)
    {
        const auto diagnostics = daisyhost::BuildGateDiagnostics(
            options, commandLine, 1, "Failed to start gate command");
        if(jsonOutput)
        {
            std::cout << daisyhost::SerializeGateDiagnosticsPayloadJson(
                             diagnostics)
                      << '\n';
        }
        else
        {
            PrintGateTextSummary(diagnostics);
        }
        return kRuntimeFailure;
    }

    const auto diagnostics = daisyhost::BuildGateDiagnostics(
        options, result.commandLine, result.exitCode, result.output);
    if(jsonOutput)
    {
        std::cout << daisyhost::SerializeGateDiagnosticsPayloadJson(diagnostics)
                  << '\n';
    }
    else
    {
        PrintGateTextSummary(diagnostics);
    }

    return diagnostics.ok ? kSuccess : kRuntimeFailure;
}

juce::var DoctorItem(const std::string& description,
                     const std::filesystem::path& path,
                     bool required)
{
    auto object = std::make_unique<juce::DynamicObject>();
    const bool exists = std::filesystem::exists(path);
    object->setProperty("description", juce::String(description));
    object->setProperty("path", juce::String(path.generic_string()));
    object->setProperty("required", required);
    object->setProperty("exists", exists);
    object->setProperty("ok", exists || !required);
    return juce::var(object.release());
}

int RunDoctor(const std::vector<std::string>& args)
{
    const bool jsonOutput = HasFlag(args, "--json");
    std::string buildDirText;
    if(!ReadOption(args, "--build-dir", &buildDirText) || buildDirText.empty())
    {
        std::cerr << "doctor requires --build-dir <dir>\n";
        return kUsageError;
    }
    std::string sourceDirText = ".";
    ReadOption(args, "--source-dir", &sourceDirText);
    std::string config = "Release";
    ReadOption(args, "--config", &config);

    const auto buildDir  = std::filesystem::path(buildDirText);
    const auto sourceDir = std::filesystem::path(sourceDirText);

    juce::Array<juce::var> checks;
    checks.add(DoctorItem("build directory", buildDir, true));
    checks.add(DoctorItem("DaisyHostCLI executable",
                          buildDir / config / "DaisyHostCLI.exe",
                          true));
    checks.add(DoctorItem("DaisyHostRender executable",
                          buildDir / config / "DaisyHostRender.exe",
                          true));
    checks.add(DoctorItem("DaisyHost Patch standalone executable",
                          buildDir / "DaisyHostPatch_artefacts" / config
                              / "Standalone" / "DaisyHost Patch.exe",
                          true));
    checks.add(DoctorItem("smoke script",
                          sourceDir / "tests" / "run_smoke.py",
                          true));
    checks.add(DoctorItem("training examples directory",
                          sourceDir / "training" / "examples",
                          true));

    bool ok = true;
    for(const auto& check : checks)
    {
        if(auto* object = check.getDynamicObject())
        {
            ok = ok && static_cast<bool>(object->getProperty("ok"));
        }
    }

    if(jsonOutput)
    {
        auto root = std::make_unique<juce::DynamicObject>();
        root->setProperty("ok", ok);
        root->setProperty("buildDir", juce::String(buildDir.generic_string()));
        root->setProperty("sourceDir", juce::String(sourceDir.generic_string()));
        root->setProperty("config", juce::String(config));
        root->setProperty("checks", juce::var(checks));
        std::cout << juce::JSON::toString(juce::var(root.release()), true)
                  << '\n';
    }
    else
    {
        std::cout << (ok ? "doctor passed" : "doctor failed") << '\n';
    }

    return ok ? kSuccess : kRuntimeFailure;
}
} // namespace

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        PrintUsage();
        return kUsageError;
    }

    std::vector<std::string> args;
    for(int index = 2; index < argc; ++index)
    {
        args.emplace_back(argv[index]);
    }

    const std::string command = argv[1];
    const bool        jsonOutput = HasFlag(args, "--json");

    if(command == "--help" || command == "help")
    {
        PrintUsage();
        return kSuccess;
    }

    if(command == "list-apps")
    {
        return PrintPayload(daisyhost::cli::SerializeAppsPayloadJson(),
                            jsonOutput,
                            "listed DaisyHost apps");
    }

    if(command == "describe-app")
    {
        if(args.empty() || args.front().rfind("--", 0) == 0)
        {
            std::cerr << "describe-app requires <appId>\n";
            return kUsageError;
        }
        std::string json;
        std::string error;
        if(!daisyhost::cli::SerializeAppDescriptionPayloadJson(
               args.front(), &json, &error))
        {
            std::cerr << error << '\n';
            return kValidationFailure;
        }
        return PrintPayload(json, jsonOutput, "described app " + args.front());
    }

    if(command == "list-boards")
    {
        return PrintPayload(daisyhost::cli::SerializeBoardsPayloadJson(),
                            jsonOutput,
                            "listed DaisyHost boards");
    }

    if(command == "describe-board")
    {
        if(args.empty() || args.front().rfind("--", 0) == 0)
        {
            std::cerr << "describe-board requires <boardId>\n";
            return kUsageError;
        }
        std::string json;
        std::string error;
        if(!daisyhost::cli::SerializeBoardDescriptionPayloadJson(
               args.front(), &json, &error))
        {
            std::cerr << error << '\n';
            return kValidationFailure;
        }
        return PrintPayload(json, jsonOutput, "described board " + args.front());
    }

    if(command == "list-inputs")
    {
        return PrintPayload(daisyhost::cli::SerializeInputsPayloadJson(),
                            jsonOutput,
                            "listed DaisyHost input modes");
    }

    if(command == "validate-scenario")
    {
        if(args.empty() || args.front().rfind("--", 0) == 0)
        {
            std::cerr << "validate-scenario requires <scenario.json>\n";
            return kUsageError;
        }
        daisyhost::RenderScenario scenario;
        daisyhost::RenderResult   result;
        std::string               error;
        if(!daisyhost::LoadRenderScenarioFromFile(args.front(), &scenario, &error)
           || !daisyhost::RunRenderScenario(scenario, &result, &error))
        {
            std::cerr << error << '\n';
            if(jsonOutput)
            {
                std::cout << StatusJson(false,
                                        "validate-scenario",
                                        error,
                                        "scenarioPath",
                                        args.front())
                          << '\n';
            }
            return kValidationFailure;
        }
        if(jsonOutput)
        {
            std::cout << StatusJson(true,
                                    "validate-scenario",
                                    "valid",
                                    "scenarioPath",
                                    args.front())
                      << '\n';
        }
        else
        {
            std::cout << "valid scenario: " << args.front() << '\n';
        }
        return kSuccess;
    }

    if(command == "render")
    {
        if(args.empty() || args.front().rfind("--", 0) == 0)
        {
            std::cerr << "render requires <scenario.json>\n";
            return kUsageError;
        }
        std::string outputDir;
        if(!ReadOption(args, "--output-dir", &outputDir) || outputDir.empty())
        {
            std::cerr << "render requires --output-dir <directory>\n";
            return kUsageError;
        }
        daisyhost::RenderScenario scenario;
        daisyhost::RenderResult   result;
        std::string               error;
        if(!daisyhost::LoadRenderScenarioFromFile(args.front(), &scenario, &error)
           || !daisyhost::RunRenderScenario(scenario, &result, &error)
           || !daisyhost::WriteRenderOutputs(&result, outputDir, &error))
        {
            std::cerr << error << '\n';
            return kRuntimeFailure;
        }
        if(jsonOutput)
        {
            std::cout
                << daisyhost::cli::SerializeRenderResultPayloadJson(result.manifest)
                << '\n';
        }
        else
        {
            std::cout << "Rendered " << result.manifest.appId << " to "
                      << result.manifest.audioPath << '\n';
        }
        return kSuccess;
    }

    if(command == "snapshot")
    {
        std::string appId;
        std::string boardId;
        std::string nodeId = "node0";
        if(!ReadOption(args, "--app", &appId) || appId.empty())
        {
            std::cerr << "snapshot requires --app <appId>\n";
            return kUsageError;
        }
        if(!ReadOption(args, "--board", &boardId) || boardId.empty())
        {
            std::cerr << "snapshot requires --board <boardId>\n";
            return kUsageError;
        }
        ReadOption(args, "--selected-node", &nodeId);

        daisyhost::EffectiveHostStateSnapshot snapshot;
        std::string                           error;
        if(!daisyhost::cli::BuildDefaultSnapshot(
               appId, boardId, nodeId, &snapshot, &error))
        {
            std::cerr << error << '\n';
            return kValidationFailure;
        }
        return PrintPayload(daisyhost::cli::SerializeSnapshotPayloadJson(snapshot),
                            jsonOutput,
                            "snapshotted " + boardId + "/" + appId);
    }

    if(command == "smoke")
    {
        std::string mode;
        std::string buildDir;
        std::string sourceDir;
        std::string config = "Release";
        if(!ReadOption(args, "--mode", &mode)
           || (mode != "render" && mode != "standalone" && mode != "all"))
        {
            std::cerr << "smoke requires --mode render|standalone|all\n";
            return kUsageError;
        }
        if(!ReadOption(args, "--build-dir", &buildDir) || buildDir.empty())
        {
            std::cerr << "smoke requires --build-dir <dir>\n";
            return kUsageError;
        }
        if(!ReadOption(args, "--source-dir", &sourceDir) || sourceDir.empty())
        {
            std::cerr << "smoke requires --source-dir <dir>\n";
            return kUsageError;
        }
        ReadOption(args, "--config", &config);
        return RunSmokeCommand({"py",
                                "-3",
                                (std::filesystem::path(sourceDir) / "tests"
                                 / "run_smoke.py")
                                    .string(),
                                "--mode",
                                mode,
                                "--build-dir",
                                buildDir,
                                "--source-dir",
                                sourceDir,
                                "--config",
                                config},
                               jsonOutput);
    }

    if(command == "doctor")
    {
        return RunDoctor(args);
    }

    if(command == "gate")
    {
        return RunGate(args);
    }

    std::cerr << "Unknown command: " << command << '\n';
    PrintUsage();
    return kUsageError;
}
