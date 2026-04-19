#include <filesystem>
#include <iostream>
#include <string>

#include "daisyhost/RenderRuntime.h"

namespace
{
void PrintUsage()
{
    std::cerr << "Usage: DaisyHostRender <scenario.json> [--output-dir <directory>]\n";
}
} // namespace

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        PrintUsage();
        return 1;
    }

    std::filesystem::path scenarioPath;
    std::filesystem::path outputDirectory;

    for(int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];
        if(argument == "--output-dir")
        {
            if(index + 1 >= argc)
            {
                std::cerr << "Missing value for --output-dir\n";
                return 1;
            }
            outputDirectory = argv[++index];
            continue;
        }

        if(scenarioPath.empty())
        {
            scenarioPath = argument;
            continue;
        }

        std::cerr << "Unknown argument: " << argument << '\n';
        PrintUsage();
        return 1;
    }

    if(scenarioPath.empty())
    {
        PrintUsage();
        return 1;
    }

    if(outputDirectory.empty())
    {
        outputDirectory = scenarioPath.parent_path() / scenarioPath.stem();
    }

    daisyhost::RenderScenario scenario;
    std::string               errorMessage;
    if(!daisyhost::LoadRenderScenarioFromFile(
           scenarioPath, &scenario, &errorMessage))
    {
        std::cerr << errorMessage << '\n';
        return 1;
    }

    daisyhost::RenderResult result;
    if(!daisyhost::RunRenderScenario(scenario, &result, &errorMessage))
    {
        std::cerr << errorMessage << '\n';
        return 1;
    }

    if(!daisyhost::WriteRenderOutputs(&result, outputDirectory, &errorMessage))
    {
        std::cerr << errorMessage << '\n';
        return 1;
    }

    std::cout << "Rendered " << result.manifest.appId << " to "
              << result.manifest.audioPath << '\n'
              << "Manifest: " << result.manifest.manifestPath << '\n';
    return 0;
}
