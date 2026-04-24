#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "daisyhost/HostedAppCore.h"
#include "daisyhost/TestInputSignal.h"

namespace daisyhost
{
struct RenderConfig
{
    double      sampleRate         = 48000.0;
    std::size_t blockSize          = 48;
    double      durationSeconds    = 1.0;
    int         outputChannelCount = 2;
};

struct RenderAudioInputConfig
{
    int   mode        = static_cast<int>(TestInputSignalMode::kHostInput);
    float level       = 0.0f;
    float frequencyHz = 220.0f;
};

struct RenderNodeConfig
{
    std::string   nodeId;
    std::string   appId;
    std::uint32_t seed = 0;
};

struct RenderRoute
{
    std::string sourcePortId;
    std::string destPortId;
};

enum class RenderTimelineEventType
{
    kParameterSet,
    kCvSet,
    kGateSet,
    kMidi,
    kAudioInputConfig,
    kImpulse,
    kMenuRotate,
    kMenuPress,
    kMenuSetItem,
};

struct RenderTimelineEvent
{
    double                  timeSeconds      = 0.0;
    RenderTimelineEventType type             = RenderTimelineEventType::kParameterSet;
    std::string             targetNodeId;
    std::string             parameterId;
    std::string             portId;
    std::string             menuItemId;
    float                   normalizedValue  = 0.0f;
    bool                    gateValue        = false;
    MidiMessageEvent        midiMessage{};
    int                     menuDelta        = 0;
    bool                    hasAudioMode     = false;
    int                     audioMode        = static_cast<int>(TestInputSignalMode::kHostInput);
    bool                    hasAudioLevel    = false;
    float                   audioLevel       = 0.0f;
    bool                    hasAudioFrequency = false;
    float                   audioFrequencyHz = 220.0f;
};

struct RenderScenario
{
    std::string                  appId;
    std::string                  boardId        = "daisy_patch";
    std::string                  selectedNodeId = "node0";
    std::string                  entryNodeId    = "node0";
    std::string                  outputNodeId   = "node0";
    RenderConfig                 renderConfig;
    std::uint32_t                seed = 0;
    std::map<std::string, float> initialParameterValues;
    RenderAudioInputConfig       audioInput;
    std::vector<RenderTimelineEvent> timeline;
    std::vector<RenderNodeConfig>     nodes;
    std::vector<RenderRoute>          routes;
};

struct RenderChannelSummary
{
    float peak = 0.0f;
    float rms  = 0.0f;
};

struct RenderNodeResultSummary
{
    std::string                  nodeId;
    std::string                  appId;
    std::string                  appDisplayName;
    std::uint32_t                seed = 0;
    std::map<std::string, float> initialParameterValues;
    std::map<std::string, float> finalParameterValues;
    std::map<std::string, float> finalEffectiveParameterValues;
    std::map<std::string, float> finalCvInputs;
    std::map<std::string, bool>  finalGateInputs;
};

struct RenderResultManifest
{
    std::string                  appId;
    std::string                  boardId;
    std::string                  selectedNodeId;
    std::string                  entryNodeId;
    std::string                  outputNodeId;
    std::string                  appDisplayName;
    RenderConfig                 renderConfig;
    std::uint64_t                frameCount = 0;
    std::uint32_t                seed       = 0;
    std::map<std::string, float> initialParameterValues;
    std::map<std::string, float> finalParameterValues;
    std::map<std::string, float> finalEffectiveParameterValues;
    std::map<std::string, float> finalCvInputs;
    std::map<std::string, bool>  finalGateInputs;
    RenderAudioInputConfig       initialAudioInput;
    RenderAudioInputConfig       finalAudioInput;
    std::vector<RenderTimelineEvent> executedTimeline;
    std::vector<RenderChannelSummary> channelSummaries;
    std::string                  audioChecksum;
    std::string                  audioPath;
    std::string                  manifestPath;
    std::vector<RenderNodeResultSummary> nodes;
    std::vector<RenderRoute>             routes;
};

struct RenderResult
{
    std::vector<std::vector<float>> audioChannels;
    RenderResultManifest            manifest;
};

const char* GetRenderTimelineEventTypeName(RenderTimelineEventType type);
bool        TryParseRenderTimelineEventType(const std::string& text,
                                            RenderTimelineEventType* type);
bool        TryParseRenderAudioInputMode(const std::string& text, int* mode);
std::string GetRenderAudioInputModeName(int mode);
} // namespace daisyhost
