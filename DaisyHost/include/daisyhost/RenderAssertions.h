#pragma once

#include <string>
#include <vector>

#include "daisyhost/RenderTypes.h"

namespace daisyhost
{
namespace cli
{
struct RenderAssertionOptions
{
    std::string              expectedChecksum;
    bool                     expectNonSilent    = false;
    int                      expectedRouteCount = -1;
    std::vector<std::string> expectedNodeIds;
    std::vector<std::string> expectedTimelineTargetNodeIds;
};

struct RenderAssertionResult
{
    std::string id;
    std::string expected;
    std::string actual;
    bool        passed = false;
    std::string message;
};

struct RenderAssertionReport
{
    bool                               passed = true;
    std::vector<RenderAssertionResult> results;
};

bool HasRenderAssertions(const RenderAssertionOptions& options);
RenderAssertionReport EvaluateRenderAssertions(
    const RenderResultManifest& manifest,
    const RenderAssertionOptions& options);
} // namespace cli
} // namespace daisyhost
