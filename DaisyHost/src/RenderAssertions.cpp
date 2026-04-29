#include "daisyhost/RenderAssertions.h"

#include <algorithm>
#include <sstream>

namespace daisyhost
{
namespace cli
{
namespace
{
void AddResult(RenderAssertionReport* report,
               const std::string&    id,
               const std::string&    expected,
               const std::string&    actual,
               bool                  passed,
               const std::string&    failureMessage)
{
    RenderAssertionResult result;
    result.id       = id;
    result.expected = expected;
    result.actual   = actual;
    result.passed   = passed;
    result.message  = passed ? "passed" : failureMessage;
    report->results.push_back(std::move(result));
    report->passed = report->passed && passed;
}

std::string ToString(std::size_t value)
{
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

bool ContainsNodeId(const RenderResultManifest& manifest, const std::string& nodeId)
{
    return std::any_of(manifest.nodes.begin(),
                       manifest.nodes.end(),
                       [&nodeId](const RenderNodeResultSummary& node) {
                           return node.nodeId == nodeId;
                       });
}

bool ContainsTimelineTargetNodeId(const RenderResultManifest& manifest,
                                  const std::string&          nodeId)
{
    return std::any_of(manifest.executedTimeline.begin(),
                       manifest.executedTimeline.end(),
                       [&nodeId](const RenderTimelineEvent& event) {
                           return event.targetNodeId == nodeId;
                       });
}

float MaxPeak(const RenderResultManifest& manifest)
{
    float peak = 0.0f;
    for(const auto& summary : manifest.channelSummaries)
    {
        peak = std::max(peak, summary.peak);
    }
    return peak;
}
} // namespace

bool HasRenderAssertions(const RenderAssertionOptions& options)
{
    return !options.expectedChecksum.empty() || options.expectNonSilent
           || options.expectedRouteCount >= 0 || !options.expectedNodeIds.empty()
           || !options.expectedTimelineTargetNodeIds.empty();
}

RenderAssertionReport EvaluateRenderAssertions(
    const RenderResultManifest& manifest,
    const RenderAssertionOptions& options)
{
    RenderAssertionReport report;

    if(!options.expectedChecksum.empty())
    {
        const bool passed = manifest.audioChecksum == options.expectedChecksum;
        AddResult(&report,
                  "checksum",
                  options.expectedChecksum,
                  manifest.audioChecksum,
                  passed,
                  "audio checksum mismatch");
    }

    if(options.expectNonSilent)
    {
        const float peak   = MaxPeak(manifest);
        const bool  passed = peak > 0.0f;
        AddResult(&report,
                  "non_silent",
                  "peak > 0",
                  std::to_string(peak),
                  passed,
                  "render output is silent or has no channel summaries");
    }

    if(options.expectedRouteCount >= 0)
    {
        const auto actual = manifest.routes.size();
        const bool passed
            = actual == static_cast<std::size_t>(options.expectedRouteCount);
        AddResult(&report,
                  "route_count",
                  std::to_string(options.expectedRouteCount),
                  ToString(actual),
                  passed,
                  "route count mismatch");
    }

    for(const auto& nodeId : options.expectedNodeIds)
    {
        const bool passed = ContainsNodeId(manifest, nodeId);
        AddResult(&report,
                  "node_id",
                  nodeId,
                  passed ? nodeId : "missing",
                  passed,
                  "expected node id missing from render result");
    }

    for(const auto& nodeId : options.expectedTimelineTargetNodeIds)
    {
        const bool passed = ContainsTimelineTargetNodeId(manifest, nodeId);
        AddResult(&report,
                  "timeline_target_node",
                  nodeId,
                  passed ? nodeId : "missing",
                  passed,
                  "expected timeline target node id missing from render result");
    }

    return report;
}
} // namespace cli
} // namespace daisyhost
