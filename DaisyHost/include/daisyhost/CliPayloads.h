#pragma once

#include <string>

#include "daisyhost/EffectiveHostStateSnapshot.h"
#include "daisyhost/RenderAssertions.h"
#include "daisyhost/RenderTypes.h"

namespace daisyhost
{
namespace cli
{
std::string SerializeAppsPayloadJson();
bool SerializeAppDescriptionPayloadJson(const std::string& appId,
                                        std::string*       outputJson,
                                        std::string*       errorMessage = nullptr);
std::string SerializeBoardsPayloadJson();
bool SerializeBoardDescriptionPayloadJson(const std::string& boardId,
                                          std::string*       outputJson,
                                          std::string*       errorMessage = nullptr);
std::string SerializeInputsPayloadJson();
std::string SerializeRenderResultPayloadJson(
    const RenderResultManifest& manifest,
    const RenderAssertionReport* assertions = nullptr);
bool BuildDefaultSnapshot(const std::string&        appId,
                          const std::string&        boardId,
                          const std::string&        selectedNodeId,
                          EffectiveHostStateSnapshot* snapshot,
                          std::string*             errorMessage = nullptr);
std::string SerializeSnapshotPayloadJson(
    const EffectiveHostStateSnapshot& snapshot);
} // namespace cli
} // namespace daisyhost
