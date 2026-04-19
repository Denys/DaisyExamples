#pragma once

#include <filesystem>
#include <string>

#include "daisyhost/RenderTypes.h"

namespace daisyhost
{
bool LoadRenderScenarioFromFile(const std::filesystem::path& scenarioPath,
                                RenderScenario*              scenario,
                                std::string*                 errorMessage = nullptr);
bool ParseRenderScenarioJson(const std::string& jsonText,
                             RenderScenario*    scenario,
                             std::string*       errorMessage = nullptr);
bool RunRenderScenario(const RenderScenario& scenario,
                       RenderResult*         result,
                       std::string*          errorMessage = nullptr);
std::string SerializeRenderManifestJson(const RenderResultManifest& manifest);
bool WriteRenderOutputs(RenderResult*                result,
                        const std::filesystem::path& outputDirectory,
                        std::string*                 errorMessage = nullptr);
} // namespace daisyhost
