#include "daisyhost/RenderRuntime.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <unordered_map>
#include <utility>

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>

#include "daisyhost/AppRegistry.h"
#include "daisyhost/LiveRackTopology.h"

namespace daisyhost
{
namespace
{
constexpr std::size_t kMaxAudioChannels = 4;

struct PortDescriptor
{
    std::string     nodeId;
    VirtualPortType type      = VirtualPortType::kAudio;
    PortDirection   direction = PortDirection::kInput;
    int             channelIndex = -1;
};

struct ResolvedRenderNode
{
    RenderNodeConfig               config;
    std::unique_ptr<HostedAppCore> app;
    HostedAppPatchBindings         bindings;
    std::map<std::string, float>   currentCvInputs;
    std::map<std::string, bool>    currentGateInputs;
};

struct RenderNodeInputState
{
    RenderAudioInputConfig audioInput;
    float                  testPhase        = 0.0f;
    std::uint32_t          noiseState       = 1u;
    bool                   impulseRequested = false;
};

std::string ExtractQualifiedNodeId(const std::string& qualifiedId);

const char* GetTimelineTypeLabel(RenderTimelineEventType type)
{
    switch(type)
    {
        case RenderTimelineEventType::kParameterSet: return "parameter_set";
        case RenderTimelineEventType::kCvSet: return "cv_set";
        case RenderTimelineEventType::kGateSet: return "gate_set";
        case RenderTimelineEventType::kMidi: return "midi";
        case RenderTimelineEventType::kAudioInputConfig: return "audio_input_config";
        case RenderTimelineEventType::kImpulse: return "impulse";
        case RenderTimelineEventType::kMenuRotate: return "menu_rotate";
        case RenderTimelineEventType::kMenuPress: return "menu_press";
        case RenderTimelineEventType::kMenuSetItem: return "menu_set_item";
    }

    return "timeline_event";
}

int EventPriority(RenderTimelineEventType type)
{
    switch(type)
    {
        case RenderTimelineEventType::kParameterSet: return 0;
        case RenderTimelineEventType::kCvSet:
        case RenderTimelineEventType::kGateSet: return 1;
        case RenderTimelineEventType::kMidi: return 2;
        case RenderTimelineEventType::kAudioInputConfig: return 3;
        case RenderTimelineEventType::kImpulse:
        case RenderTimelineEventType::kMenuRotate:
        case RenderTimelineEventType::kMenuPress:
        case RenderTimelineEventType::kMenuSetItem: return 4;
    }

    return 99;
}

std::string ToLower(std::string text)
{
    std::transform(text.begin(),
                   text.end(),
                   text.begin(),
                   [](unsigned char character) {
                       return static_cast<char>(std::tolower(character));
                   });
    return text;
}

std::string NormalizeToken(std::string text)
{
    text = ToLower(std::move(text));
    std::replace(text.begin(), text.end(), ' ', '_');
    std::replace(text.begin(), text.end(), '-', '_');
    return text;
}

bool IsFinite(float value)
{
    return std::isfinite(value);
}

bool IsFinite(double value)
{
    return std::isfinite(value);
}

std::uint64_t TimeToFrame(double timeSeconds, double sampleRate)
{
    const double clampedTime = std::max(0.0, timeSeconds);
    return static_cast<std::uint64_t>(std::floor(clampedTime * sampleRate + 1.0e-9));
}

const juce::DynamicObject* ExpectObject(const juce::var& value,
                                        const char*      context,
                                        std::string*     errorMessage)
{
    if(auto* object = value.getDynamicObject())
    {
        return object;
    }

    if(errorMessage != nullptr)
    {
        *errorMessage = std::string(context) + " must be a JSON object";
    }
    return nullptr;
}

const juce::Array<juce::var>* ExpectArray(const juce::var& value,
                                          const char*      context,
                                          std::string*     errorMessage)
{
    if(auto* array = value.getArray())
    {
        return array;
    }

    if(errorMessage != nullptr)
    {
        *errorMessage = std::string(context) + " must be a JSON array";
    }
    return nullptr;
}

bool ReadRequiredString(const juce::DynamicObject& object,
                        const char*                key,
                        std::string*               value,
                        std::string*               errorMessage)
{
    const auto property = object.getProperty(key);
    if(property.isVoid() || !property.isString())
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = std::string("Missing or invalid string field: ") + key;
        }
        return false;
    }

    if(value != nullptr)
    {
        *value = property.toString().toStdString();
    }
    return true;
}

bool ReadOptionalString(const juce::DynamicObject& object,
                        const char*                key,
                        std::string*               value,
                        bool*                      present,
                        std::string*               errorMessage)
{
    const auto property = object.getProperty(key);
    if(property.isVoid())
    {
        if(present != nullptr)
        {
            *present = false;
        }
        return true;
    }
    if(!property.isString())
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = std::string("Invalid string field: ") + key;
        }
        return false;
    }

    if(value != nullptr)
    {
        *value = property.toString().toStdString();
    }
    if(present != nullptr)
    {
        *present = true;
    }
    return true;
}

bool ReadOptionalNumber(const juce::DynamicObject& object,
                        const char*                key,
                        double*                    value,
                        bool*                      present,
                        std::string*               errorMessage)
{
    const auto property = object.getProperty(key);
    if(property.isVoid())
    {
        if(present != nullptr)
        {
            *present = false;
        }
        return true;
    }
    if(!property.isDouble() && !property.isInt() && !property.isInt64())
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = std::string("Invalid numeric field: ") + key;
        }
        return false;
    }

    if(value != nullptr)
    {
        *value = static_cast<double>(property);
    }
    if(present != nullptr)
    {
        *present = true;
    }
    return true;
}

bool ReadOptionalBool(const juce::DynamicObject& object,
                      const char*                key,
                      bool*                      value,
                      bool*                      present,
                      std::string*               errorMessage)
{
    const auto property = object.getProperty(key);
    if(property.isVoid())
    {
        if(present != nullptr)
        {
            *present = false;
        }
        return true;
    }
    if(!property.isBool() && !property.isInt() && !property.isInt64())
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = std::string("Invalid boolean field: ") + key;
        }
        return false;
    }

    if(value != nullptr)
    {
        *value = static_cast<bool>(property);
    }
    if(present != nullptr)
    {
        *present = true;
    }
    return true;
}

bool ParseAudioInputConfigObject(const juce::DynamicObject& object,
                                 RenderAudioInputConfig*    config,
                                 std::string*               errorMessage)
{
    if(config == nullptr)
    {
        return false;
    }

    std::string modeText;
    bool        hasModeString = false;
    if(!ReadOptionalString(
           object, "mode", &modeText, &hasModeString, errorMessage))
    {
        return false;
    }
    if(!hasModeString
       && !ReadOptionalString(
              object, "audioMode", &modeText, &hasModeString, errorMessage))
    {
        return false;
    }

    double numericValue = 0.0;
    bool   hasModeNumber = false;
    if(!hasModeString
       && !ReadOptionalNumber(
              object, "mode", &numericValue, &hasModeNumber, errorMessage))
    {
        return false;
    }
    if(!hasModeString
       && !hasModeNumber
       && !ReadOptionalNumber(
              object, "audioMode", &numericValue, &hasModeNumber, errorMessage))
    {
        return false;
    }

    if(hasModeString)
    {
        int parsedMode = config->mode;
        if(!TryParseRenderAudioInputMode(modeText, &parsedMode))
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Unknown audio input mode: " + modeText;
            }
            return false;
        }
        config->mode = parsedMode;
    }
    else if(hasModeNumber)
    {
        config->mode = ClampTestInputSignalMode(static_cast<int>(std::round(numericValue)));
    }

    bool levelPresent = false;
    if(!ReadOptionalNumber(object,
                           "level",
                           &numericValue,
                           &levelPresent,
                           errorMessage))
    {
        return false;
    }
    if(!levelPresent
       && !ReadOptionalNumber(object,
                              "audioLevel",
                              &numericValue,
                              &levelPresent,
                              errorMessage))
    {
        return false;
    }
    if(levelPresent)
    {
        config->level = static_cast<float>(numericValue);
    }
    bool frequencyPresent = false;
    if(!ReadOptionalNumber(
           object, "frequencyHz", &numericValue, &frequencyPresent, errorMessage))
    {
        return false;
    }
    if(!frequencyPresent
       && !ReadOptionalNumber(object,
                              "audioFrequencyHz",
                              &numericValue,
                              &frequencyPresent,
                              errorMessage))
    {
        return false;
    }
    if(frequencyPresent)
    {
        config->frequencyHz = static_cast<float>(numericValue);
    }

    return true;
}

bool ParseRenderNodeObject(const juce::var& value,
                           std::uint32_t    fallbackSeed,
                           RenderNodeConfig* config,
                           std::string*     errorMessage)
{
    const auto* object = ExpectObject(value, "node", errorMessage);
    if(object == nullptr || config == nullptr)
    {
        return false;
    }

    if(!ReadRequiredString(*object, "nodeId", &config->nodeId, errorMessage)
       || !ReadRequiredString(*object, "appId", &config->appId, errorMessage))
    {
        return false;
    }

    config->seed = fallbackSeed;
    double seedValue = 0.0;
    bool   hasSeed   = false;
    if(!ReadOptionalNumber(*object, "seed", &seedValue, &hasSeed, errorMessage))
    {
        return false;
    }
    if(hasSeed)
    {
        config->seed = static_cast<std::uint32_t>(std::llround(seedValue));
    }

    return true;
}

bool ParseRenderRouteObject(const juce::var& value,
                            RenderRoute*     route,
                            std::string*     errorMessage)
{
    const auto* object = ExpectObject(value, "route", errorMessage);
    if(object == nullptr || route == nullptr)
    {
        return false;
    }

    return ReadRequiredString(*object, "sourcePortId", &route->sourcePortId, errorMessage)
           && ReadRequiredString(*object, "destPortId", &route->destPortId, errorMessage);
}

bool ParseTimelineEvent(const juce::var& value,
                        RenderTimelineEvent* event,
                        std::string*         errorMessage)
{
    const auto* object = ExpectObject(value, "timeline item", errorMessage);
    if(object == nullptr || event == nullptr)
    {
        return false;
    }

    double timeSeconds = 0.0;
    bool   hasTime     = false;
    if(!ReadOptionalNumber(
           *object, "timeSeconds", &timeSeconds, &hasTime, errorMessage)
       || !hasTime)
    {
        if(errorMessage != nullptr && errorMessage->empty())
        {
            *errorMessage = "Missing or invalid numeric field: timeSeconds";
        }
        return false;
    }

    std::string typeText;
    if(!ReadRequiredString(*object, "type", &typeText, errorMessage))
    {
        return false;
    }

    RenderTimelineEventType eventType = RenderTimelineEventType::kParameterSet;
    if(!TryParseRenderTimelineEventType(typeText, &eventType))
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Unknown timeline event type: " + typeText;
        }
        return false;
    }

    event->timeSeconds = timeSeconds;
    event->type        = eventType;
    if(!ReadOptionalString(*object,
                           "targetNodeId",
                           &event->targetNodeId,
                           nullptr,
                           errorMessage))
    {
        return false;
    }

    switch(eventType)
    {
        case RenderTimelineEventType::kParameterSet:
            if(!ReadRequiredString(
                   *object, "parameterId", &event->parameterId, errorMessage))
            {
                return false;
            }
            {
                double normalizedValue = 0.0;
                bool   hasValue        = false;
                if(!ReadOptionalNumber(*object,
                                       "normalizedValue",
                                       &normalizedValue,
                                       &hasValue,
                                       errorMessage)
                   || !hasValue)
                {
                    if(errorMessage != nullptr && errorMessage->empty())
                    {
                        *errorMessage = "Missing or invalid numeric field: normalizedValue";
                    }
                    return false;
                }
                event->normalizedValue = static_cast<float>(normalizedValue);
            }
            return true;

        case RenderTimelineEventType::kCvSet:
            if(!ReadRequiredString(*object, "portId", &event->portId, errorMessage))
            {
                return false;
            }
            {
                double normalizedValue = 0.0;
                bool   hasValue        = false;
                if(!ReadOptionalNumber(*object,
                                       "normalizedValue",
                                       &normalizedValue,
                                       &hasValue,
                                       errorMessage)
                   || !hasValue)
                {
                    if(errorMessage != nullptr && errorMessage->empty())
                    {
                        *errorMessage = "Missing or invalid numeric field: normalizedValue";
                    }
                    return false;
                }
                event->normalizedValue = static_cast<float>(normalizedValue);
            }
            return true;

        case RenderTimelineEventType::kGateSet:
        {
            bool hasGate = false;
            if(!ReadRequiredString(*object, "portId", &event->portId, errorMessage)
               || !ReadOptionalBool(
                      *object, "gate", &event->gateValue, &hasGate, errorMessage)
               || !hasGate)
            {
                if(errorMessage != nullptr && errorMessage->empty())
                {
                    *errorMessage = "Missing or invalid boolean field: gate";
                }
                return false;
            }
            return true;
        }

        case RenderTimelineEventType::kMidi:
        {
            double valueNumber = 0.0;
            bool   present     = false;
            if(!ReadOptionalNumber(*object, "status", &valueNumber, &present, errorMessage)
               || !present)
            {
                if(errorMessage != nullptr && errorMessage->empty())
                {
                    *errorMessage = "Missing or invalid numeric field: status";
                }
                return false;
            }
            event->midiMessage.status = static_cast<std::uint8_t>(std::round(valueNumber));

            if(!ReadOptionalNumber(*object, "data1", &valueNumber, &present, errorMessage)
               || !present)
            {
                if(errorMessage != nullptr && errorMessage->empty())
                {
                    *errorMessage = "Missing or invalid numeric field: data1";
                }
                return false;
            }
            event->midiMessage.data1 = static_cast<std::uint8_t>(std::round(valueNumber));

            if(!ReadOptionalNumber(*object, "data2", &valueNumber, &present, errorMessage)
               || !present)
            {
                if(errorMessage != nullptr && errorMessage->empty())
                {
                    *errorMessage = "Missing or invalid numeric field: data2";
                }
                return false;
            }
            event->midiMessage.data2 = static_cast<std::uint8_t>(std::round(valueNumber));
            return true;
        }

        case RenderTimelineEventType::kAudioInputConfig:
        {
            std::string modeText;
            bool        modePresent = false;
            if(!ReadOptionalString(*object, "mode", &modeText, &modePresent, errorMessage))
            {
                return false;
            }
            if(!modePresent
               && !ReadOptionalString(*object,
                                      "audioMode",
                                      &modeText,
                                      &modePresent,
                                      errorMessage))
            {
                return false;
            }
            if(modePresent)
            {
                int parsedMode = event->audioMode;
                if(!TryParseRenderAudioInputMode(modeText, &parsedMode))
                {
                    if(errorMessage != nullptr)
                    {
                        *errorMessage = "Unknown audio input mode: " + modeText;
                    }
                    return false;
                }
                event->hasAudioMode = true;
                event->audioMode    = parsedMode;
            }

            double numberValue = 0.0;
            bool   present     = false;
            if(!ReadOptionalNumber(
                   *object, "level", &numberValue, &present, errorMessage))
            {
                return false;
            }
            if(!present
               && !ReadOptionalNumber(
                      *object, "audioLevel", &numberValue, &present, errorMessage))
            {
                return false;
            }
            if(present)
            {
                event->hasAudioLevel = true;
                event->audioLevel    = static_cast<float>(numberValue);
            }

            if(!ReadOptionalNumber(
                   *object, "frequencyHz", &numberValue, &present, errorMessage))
            {
                return false;
            }
            if(!present
               && !ReadOptionalNumber(*object,
                                      "audioFrequencyHz",
                                      &numberValue,
                                      &present,
                                      errorMessage))
            {
                return false;
            }
            if(present)
            {
                event->hasAudioFrequency = true;
                event->audioFrequencyHz  = static_cast<float>(numberValue);
            }

            if(!event->hasAudioMode && !event->hasAudioLevel
               && !event->hasAudioFrequency)
            {
                if(errorMessage != nullptr)
                {
                    *errorMessage
                        = "audio_input_config event must set mode, level, or frequencyHz";
                }
                return false;
            }
            return true;
        }

        case RenderTimelineEventType::kImpulse: return true;

        case RenderTimelineEventType::kMenuRotate:
        {
            double delta = 0.0;
            bool   hasDelta = false;
            if(!ReadOptionalNumber(*object, "delta", &delta, &hasDelta, errorMessage)
               || !hasDelta)
            {
                if(errorMessage != nullptr && errorMessage->empty())
                {
                    *errorMessage = "Missing or invalid numeric field: delta";
                }
                return false;
            }
            event->menuDelta = static_cast<int>(std::round(delta));
            return true;
        }

        case RenderTimelineEventType::kMenuPress: return true;

        case RenderTimelineEventType::kMenuSetItem:
        {
            if(!ReadRequiredString(
                   *object, "itemId", &event->menuItemId, errorMessage))
            {
                return false;
            }
            double normalizedValue = 0.0;
            bool   hasValue        = false;
            if(!ReadOptionalNumber(*object,
                                   "normalizedValue",
                                   &normalizedValue,
                                   &hasValue,
                                   errorMessage)
               || !hasValue)
            {
                if(errorMessage != nullptr && errorMessage->empty())
                {
                    *errorMessage = "Missing or invalid numeric field: normalizedValue";
                }
                return false;
            }
            event->normalizedValue = static_cast<float>(normalizedValue);
            return true;
        }
    }

    return false;
}

bool ValidateNormalizedValue(float value,
                             const std::string& label,
                             std::string*       errorMessage)
{
    if(!IsFinite(value) || value < 0.0f || value > 1.0f)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = label + " must be a finite normalized value in [0, 1]";
        }
        return false;
    }
    return true;
}

bool ValidateAudioConfig(const RenderAudioInputConfig& config,
                         std::string*                  errorMessage)
{
    const int clampedMode = ClampTestInputSignalMode(config.mode);
    if(clampedMode != config.mode)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Invalid audio input mode";
        }
        return false;
    }

    if(!IsFinite(config.level) || config.level < 0.0f || config.level > 10.0f)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Audio input level must be finite and within [0, 10]";
        }
        return false;
    }

    if(!IsFinite(config.frequencyHz) || config.frequencyHz < 20.0f
       || config.frequencyHz > 5000.0f)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Audio input frequency must be finite and within [20, 5000]";
        }
        return false;
    }

    return true;
}

std::map<std::string, float> CaptureEffectiveParameterValues(
    const HostedAppCore& app)
{
    std::map<std::string, float> values;
    for(const auto& parameter : app.GetParameters())
    {
        const auto lookup = app.GetEffectiveParameterValue(parameter.id);
        if(lookup.hasValue)
        {
            values[parameter.id] = lookup.value;
        }
        else
        {
            values[parameter.id] = parameter.effectiveNormalizedValue;
        }
    }
    return values;
}

std::string HashAudioChannels(const std::vector<std::vector<float>>& audioChannels)
{
    constexpr std::uint64_t kOffset = 1469598103934665603ull;
    constexpr std::uint64_t kPrime  = 1099511628211ull;

    const std::size_t frameCount
        = audioChannels.empty() ? 0 : audioChannels.front().size();

    std::uint64_t hash = kOffset;
    for(std::size_t frame = 0; frame < frameCount; ++frame)
    {
        for(const auto& channel : audioChannels)
        {
            const float value = frame < channel.size() ? channel[frame] : 0.0f;
            std::uint32_t bits = 0;
            std::memcpy(&bits, &value, sizeof(bits));
            for(int byteIndex = 0; byteIndex < 4; ++byteIndex)
            {
                const auto byte = static_cast<std::uint8_t>((bits >> (byteIndex * 8)) & 0xFFu);
                hash ^= byte;
                hash *= kPrime;
            }
        }
    }

    std::ostringstream stream;
    stream << std::hex << hash;
    return stream.str();
}

RenderChannelSummary SummarizeChannel(const std::vector<float>& samples)
{
    RenderChannelSummary summary;
    if(samples.empty())
    {
        return summary;
    }

    double sumSquares = 0.0;
    for(float sample : samples)
    {
        summary.peak = std::max(summary.peak, std::abs(sample));
        sumSquares += static_cast<double>(sample) * static_cast<double>(sample);
    }

    summary.rms = static_cast<float>(
        std::sqrt(sumSquares / static_cast<double>(samples.size())));
    return summary;
}

juce::var MakeAudioInputVar(const RenderAudioInputConfig& config)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("mode", juce::String(GetRenderAudioInputModeName(config.mode)));
    object->setProperty("level", config.level);
    object->setProperty("frequencyHz", config.frequencyHz);
    return juce::var(object.release());
}

juce::var MakeMapVar(const std::map<std::string, float>& values)
{
    auto object = std::make_unique<juce::DynamicObject>();
    for(const auto& entry : values)
    {
        object->setProperty(juce::Identifier(entry.first), entry.second);
    }
    return juce::var(object.release());
}

juce::var MakeBoolMapVar(const std::map<std::string, bool>& values)
{
    auto object = std::make_unique<juce::DynamicObject>();
    for(const auto& entry : values)
    {
        object->setProperty(juce::Identifier(entry.first), entry.second);
    }
    return juce::var(object.release());
}

juce::var MakeTimelineEventVar(const RenderTimelineEvent& event)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("timeSeconds", event.timeSeconds);
    object->setProperty("type",
                        juce::String(GetRenderTimelineEventTypeName(event.type)));
    if(!event.targetNodeId.empty())
    {
        object->setProperty("targetNodeId", juce::String(event.targetNodeId));
    }

    switch(event.type)
    {
        case RenderTimelineEventType::kParameterSet:
            object->setProperty("parameterId", juce::String(event.parameterId));
            object->setProperty("normalizedValue", event.normalizedValue);
            break;
        case RenderTimelineEventType::kCvSet:
            object->setProperty("portId", juce::String(event.portId));
            object->setProperty("normalizedValue", event.normalizedValue);
            break;
        case RenderTimelineEventType::kGateSet:
            object->setProperty("portId", juce::String(event.portId));
            object->setProperty("gate", event.gateValue);
            break;
        case RenderTimelineEventType::kMidi:
            object->setProperty("status", static_cast<int>(event.midiMessage.status));
            object->setProperty("data1", static_cast<int>(event.midiMessage.data1));
            object->setProperty("data2", static_cast<int>(event.midiMessage.data2));
            break;
        case RenderTimelineEventType::kAudioInputConfig:
            if(event.hasAudioMode)
            {
                object->setProperty(
                    "mode",
                    juce::String(GetRenderAudioInputModeName(event.audioMode)));
            }
            if(event.hasAudioLevel)
            {
                object->setProperty("level", event.audioLevel);
            }
            if(event.hasAudioFrequency)
            {
                object->setProperty("frequencyHz", event.audioFrequencyHz);
            }
            break;
        case RenderTimelineEventType::kImpulse: break;
        case RenderTimelineEventType::kMenuRotate:
            object->setProperty("delta", event.menuDelta);
            break;
        case RenderTimelineEventType::kMenuPress: break;
        case RenderTimelineEventType::kMenuSetItem:
            object->setProperty("itemId", juce::String(event.menuItemId));
            object->setProperty("normalizedValue", event.normalizedValue);
            break;
    }

    return juce::var(object.release());
}

juce::var MakeRouteVar(const RenderRoute& route)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("sourcePortId", juce::String(route.sourcePortId));
    object->setProperty("destPortId", juce::String(route.destPortId));
    return juce::var(object.release());
}

juce::var MakeNodeSummaryVar(const RenderNodeResultSummary& node)
{
    auto object = std::make_unique<juce::DynamicObject>();
    object->setProperty("nodeId", juce::String(node.nodeId));
    object->setProperty("appId", juce::String(node.appId));
    object->setProperty("appDisplayName", juce::String(node.appDisplayName));
    object->setProperty("seed", static_cast<juce::int64>(node.seed));
    object->setProperty("initialParameterValues", MakeMapVar(node.initialParameterValues));
    object->setProperty("finalParameterValues", MakeMapVar(node.finalParameterValues));
    object->setProperty("finalEffectiveParameterValues",
                        MakeMapVar(node.finalEffectiveParameterValues));
    object->setProperty("finalCvInputs", MakeMapVar(node.finalCvInputs));
    object->setProperty("finalGateInputs", MakeBoolMapVar(node.finalGateInputs));
    return juce::var(object.release());
}

std::vector<RenderNodeConfig> ResolveScenarioNodes(const RenderScenario& scenario)
{
    if(!scenario.nodes.empty())
    {
        return scenario.nodes;
    }

    return {RenderNodeConfig{"node0", scenario.appId, scenario.seed}};
}

void AddPortDescriptor(std::unordered_map<std::string, PortDescriptor>* ports,
                       const std::string&                                portId,
                       const std::string&                                nodeId,
                       VirtualPortType                                   type,
                       PortDirection                                     direction,
                       int                                               channelIndex)
{
    if(ports == nullptr || portId.empty())
    {
        return;
    }

    (*ports)[portId] = PortDescriptor{nodeId, type, direction, channelIndex};
}

std::unordered_map<std::string, PortDescriptor>
BuildPortDescriptorMap(const HostedAppPatchBindings& bindings, const std::string& nodeId)
{
    std::unordered_map<std::string, PortDescriptor> ports;
    for(std::size_t index = 0; index < bindings.audioInputPortIds.size(); ++index)
    {
        AddPortDescriptor(&ports,
                          bindings.audioInputPortIds[index],
                          nodeId,
                          VirtualPortType::kAudio,
                          PortDirection::kInput,
                          static_cast<int>(index));
        AddPortDescriptor(&ports,
                          bindings.audioOutputPortIds[index],
                          nodeId,
                          VirtualPortType::kAudio,
                          PortDirection::kOutput,
                          static_cast<int>(index));
        AddPortDescriptor(&ports,
                          bindings.cvInputPortIds[index],
                          nodeId,
                          VirtualPortType::kCv,
                          PortDirection::kInput,
                          static_cast<int>(index));
    }
    for(std::size_t index = 0; index < bindings.gateInputPortIds.size(); ++index)
    {
        AddPortDescriptor(&ports,
                          bindings.gateInputPortIds[index],
                          nodeId,
                          VirtualPortType::kGate,
                          PortDirection::kInput,
                          static_cast<int>(index));
    }
    AddPortDescriptor(&ports,
                      bindings.gateOutputPortId,
                      nodeId,
                      VirtualPortType::kGate,
                      PortDirection::kOutput,
                      0);
    AddPortDescriptor(&ports,
                      bindings.midiInputPortId,
                      nodeId,
                      VirtualPortType::kMidi,
                      PortDirection::kInput,
                      0);
    AddPortDescriptor(&ports,
                      bindings.midiOutputPortId,
                      nodeId,
                      VirtualPortType::kMidi,
                      PortDirection::kOutput,
                      0);
    return ports;
}

LiveRackTopologyConfig MakeLiveRackTopologyConfig(const RenderScenario& scenario)
{
    LiveRackTopologyConfig config;
    config.entryNodeId  = scenario.entryNodeId;
    config.outputNodeId = scenario.outputNodeId;
    config.routes.reserve(scenario.routes.size());
    for(const auto& route : scenario.routes)
    {
        config.routes.push_back({route.sourcePortId, route.destPortId});
    }
    if(!config.routes.empty() && config.entryNodeId == config.outputNodeId)
    {
        const auto inferredEntryNodeId
            = ExtractQualifiedNodeId(config.routes.front().sourcePortId);
        const auto inferredOutputNodeId
            = ExtractQualifiedNodeId(config.routes.front().destPortId);
        if(!inferredEntryNodeId.empty() && !inferredOutputNodeId.empty()
           && inferredEntryNodeId != inferredOutputNodeId)
        {
            config.entryNodeId  = inferredEntryNodeId;
            config.outputNodeId = inferredOutputNodeId;
        }
    }
    return config;
}

bool RequiresTargetNodeId(RenderTimelineEventType type)
{
    switch(type)
    {
        case RenderTimelineEventType::kAudioInputConfig:
        case RenderTimelineEventType::kImpulse:
        case RenderTimelineEventType::kMenuRotate:
        case RenderTimelineEventType::kMenuPress: return true;

        case RenderTimelineEventType::kParameterSet:
        case RenderTimelineEventType::kCvSet:
        case RenderTimelineEventType::kGateSet:
        case RenderTimelineEventType::kMidi:
        case RenderTimelineEventType::kMenuSetItem: return false;
    }

    return false;
}

std::size_t FindNodeIndexById(const std::vector<ResolvedRenderNode>& nodes,
                              const std::string&                     nodeId)
{
    const auto nodeIt = std::find_if(nodes.begin(),
                                     nodes.end(),
                                     [&nodeId](const ResolvedRenderNode& candidate) {
                                         return candidate.config.nodeId == nodeId;
                                     });
    return nodeIt == nodes.end()
               ? nodes.size()
               : static_cast<std::size_t>(std::distance(nodes.begin(), nodeIt));
}

std::string ExtractQualifiedNodeId(const std::string& qualifiedId)
{
    const auto slashIndex = qualifiedId.find('/');
    if(slashIndex == std::string::npos)
    {
        return {};
    }
    return qualifiedId.substr(0, slashIndex);
}

std::size_t FindNodeIndexForQualifiedId(const std::vector<ResolvedRenderNode>& nodes,
                                        const std::string& qualifiedId)
{
    return FindNodeIndexById(nodes, ExtractQualifiedNodeId(qualifiedId));
}

std::size_t ResolveEventNodeIndex(const RenderTimelineEvent&            event,
                                  const RenderScenario&                 scenario,
                                  const std::vector<ResolvedRenderNode>& nodes,
                                  std::string*                          errorMessage)
{
    if(RequiresTargetNodeId(event.type))
    {
        if(event.targetNodeId.empty())
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Multi-node "
                                + std::string(GetTimelineTypeLabel(event.type))
                                + " events require targetNodeId";
            }
            return nodes.size();
        }

        const auto nodeIndex = FindNodeIndexById(nodes, event.targetNodeId);
        if(nodeIndex >= nodes.size())
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Unknown targetNodeId in timeline: " + event.targetNodeId;
            }
            return nodes.size();
        }
        return nodeIndex;
    }

    if(event.type == RenderTimelineEventType::kMidi)
    {
        const std::string targetNodeId
            = event.targetNodeId.empty() ? scenario.selectedNodeId : event.targetNodeId;
        const auto nodeIndex = FindNodeIndexById(nodes, targetNodeId);
        if(nodeIndex >= nodes.size())
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Unknown targetNodeId in timeline: " + targetNodeId;
            }
            return nodes.size();
        }
        return nodeIndex;
    }

    return nodes.size();
}

bool ValidateCommonRenderScenario(const RenderScenario& scenario,
                                  std::string*          errorMessage)
{
    if(scenario.appId.empty())
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Scenario appId must not be empty";
        }
        return false;
    }

    if(!IsFinite(scenario.renderConfig.sampleRate)
       || scenario.renderConfig.sampleRate <= 1.0)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Render sampleRate must be greater than 1";
        }
        return false;
    }
    if(scenario.renderConfig.blockSize == 0)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Render blockSize must be greater than 0";
        }
        return false;
    }
    if(!IsFinite(scenario.renderConfig.durationSeconds)
       || scenario.renderConfig.durationSeconds <= 0.0)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Render durationSeconds must be greater than 0";
        }
        return false;
    }
    if(scenario.renderConfig.outputChannelCount != 1
       && scenario.renderConfig.outputChannelCount != 2
       && scenario.renderConfig.outputChannelCount != 4)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Render outputChannelCount must be 1, 2, or 4";
        }
        return false;
    }
    return ValidateAudioConfig(scenario.audioInput, errorMessage);
}

bool ValidateRouteGraph(const std::vector<RenderRoute>&                 routes,
                        const std::unordered_map<std::string, PortDescriptor>& ports,
                        std::string*                                     errorMessage)
{
    std::set<std::string> routedDestinations;
    for(const auto& route : routes)
    {
        const auto sourceIt = ports.find(route.sourcePortId);
        const auto destIt   = ports.find(route.destPortId);
        if(sourceIt == ports.end() || destIt == ports.end())
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Unknown route port id";
            }
            return false;
        }

        if(sourceIt->second.type != VirtualPortType::kAudio
           || destIt->second.type != VirtualPortType::kAudio)
        {
            if(errorMessage != nullptr)
            {
                *errorMessage
                    = "Multi-node render currently supports audio routes only";
            }
            return false;
        }

        if(sourceIt->second.direction != PortDirection::kOutput
           || destIt->second.direction != PortDirection::kInput)
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Route directions must be output -> input";
            }
            return false;
        }

        if(sourceIt->second.nodeId == destIt->second.nodeId)
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Routes must connect different nodes";
            }
            return false;
        }

        if(!routedDestinations.insert(route.destPortId).second)
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Multiple inbound routes to the same destination are not supported";
            }
            return false;
        }
    }

    return true;
}

std::size_t FindPrimaryNodeIndex(const std::vector<ResolvedRenderNode>& nodes)
{
    for(std::size_t index = 0; index < nodes.size(); ++index)
    {
        if(nodes[index].config.nodeId == "node0")
        {
            return index;
        }
    }
    return nodes.size();
}

bool ResolveMultiNodeScenario(const RenderScenario& scenario,
                              std::vector<ResolvedRenderNode>* nodes,
                              std::unordered_map<std::string, PortDescriptor>* ports,
                              std::string* errorMessage)
{
    if(nodes == nullptr || ports == nullptr)
    {
        return false;
    }
    if(!ValidateCommonRenderScenario(scenario, errorMessage))
    {
        return false;
    }

    const auto nodeConfigs = ResolveScenarioNodes(scenario);
    if(nodeConfigs.empty())
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Scenario must define at least one render node";
        }
        return false;
    }
    if(nodeConfigs.size() > 2)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Multi-node render currently supports up to two nodes";
        }
        return false;
    }

    nodes->clear();
    ports->clear();
    nodes->reserve(nodeConfigs.size());

    std::set<std::string> seenNodeIds;
    for(const auto& config : nodeConfigs)
    {
        if(config.nodeId.empty() || config.appId.empty())
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Render nodes require non-empty nodeId and appId";
            }
            return false;
        }
        if(!seenNodeIds.insert(config.nodeId).second)
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Duplicate render node id: " + config.nodeId;
            }
            return false;
        }

        std::string resolvedAppId;
        auto app = CreateHostedAppCore(config.appId, config.nodeId, &resolvedAppId);
        if(app == nullptr || resolvedAppId != config.appId)
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Unknown appId: " + config.appId;
            }
            return false;
        }

        ResolvedRenderNode resolved;
        resolved.config   = config;
        resolved.bindings = app->GetPatchBindings();
        resolved.app      = std::move(app);

        for(const auto& portId : resolved.bindings.cvInputPortIds)
        {
            if(!portId.empty())
            {
                resolved.currentCvInputs[portId] = 0.5f;
            }
        }
        for(const auto& portId : resolved.bindings.gateInputPortIds)
        {
            if(!portId.empty())
            {
                resolved.currentGateInputs[portId] = false;
            }
        }

        const auto nodePorts
            = BuildPortDescriptorMap(resolved.bindings, resolved.config.nodeId);
        for(const auto& entry : nodePorts)
        {
            ports->insert(entry);
        }

        nodes->push_back(std::move(resolved));
    }

    const auto primaryIndex = FindPrimaryNodeIndex(*nodes);
    if(primaryIndex >= nodes->size())
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Multi-node render requires node0";
        }
        return false;
    }
    if(!scenario.appId.empty() && scenario.appId != (*nodes)[primaryIndex].config.appId)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Top-level scenario appId must match the primary node appId";
        }
        return false;
    }

    if(!ValidateRouteGraph(scenario.routes, *ports, errorMessage))
    {
        return false;
    }

    if(!ValidateLiveRackTopologyConfig(MakeLiveRackTopologyConfig(scenario), errorMessage))
    {
        return false;
    }

    return true;
}

bool FindMenuItemId(const MenuModel& menu, const std::string& itemId)
{
    for(const auto& section : menu.sections)
    {
        for(const auto& item : section.items)
        {
            if(item.id == itemId)
            {
                return true;
            }
        }
    }
    return false;
}

struct TimelineEventWithIndex
{
    RenderTimelineEvent event;
    std::size_t         originalIndex = 0;
};

bool ValidateScenario(const RenderScenario& scenario,
                      HostedAppCore&        app,
                      std::vector<TimelineEventWithIndex>* sortedEvents,
                      std::string*          errorMessage)
{
    if(!ValidateCommonRenderScenario(scenario, errorMessage))
    {
        return false;
    }

    std::set<std::string> validParameterIds;
    for(const auto& parameter : app.GetParameters())
    {
        validParameterIds.insert(parameter.id);
    }

    const auto bindings = app.GetPatchBindings();
    std::set<std::string> validCvPortIds;
    std::set<std::string> validGatePortIds;
    for(const auto& portId : bindings.cvInputPortIds)
    {
        if(!portId.empty())
        {
            validCvPortIds.insert(portId);
        }
    }
    for(const auto& portId : bindings.gateInputPortIds)
    {
        if(!portId.empty())
        {
            validGatePortIds.insert(portId);
        }
    }
    const bool acceptsMidi = app.GetCapabilities().acceptsMidiInput
                             && !bindings.midiInputPortId.empty();
    const MenuModel initialMenu = app.GetMenuModel();

    for(const auto& entry : scenario.initialParameterValues)
    {
        if(validParameterIds.find(entry.first) == validParameterIds.end())
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Unknown parameter id in initialParameterValues: "
                                + entry.first;
            }
            return false;
        }
        if(!ValidateNormalizedValue(entry.second,
                                    "Initial parameter value for " + entry.first,
                                    errorMessage))
        {
            return false;
        }
    }

    if(sortedEvents == nullptr)
    {
        return false;
    }

    sortedEvents->clear();
    sortedEvents->reserve(scenario.timeline.size());
    for(std::size_t index = 0; index < scenario.timeline.size(); ++index)
    {
        const auto& event = scenario.timeline[index];
        if(!IsFinite(event.timeSeconds) || event.timeSeconds < 0.0
           || event.timeSeconds > scenario.renderConfig.durationSeconds)
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Timeline event timeSeconds is out of range";
            }
            return false;
        }

        switch(event.type)
        {
            case RenderTimelineEventType::kParameterSet:
                if(validParameterIds.find(event.parameterId) == validParameterIds.end())
                {
                    if(errorMessage != nullptr)
                    {
                        *errorMessage
                            = "Unknown parameter id in timeline: " + event.parameterId;
                    }
                    return false;
                }
                if(!ValidateNormalizedValue(event.normalizedValue,
                                            "Timeline parameter value",
                                            errorMessage))
                {
                    return false;
                }
                break;

            case RenderTimelineEventType::kCvSet:
                if(validCvPortIds.find(event.portId) == validCvPortIds.end())
                {
                    if(errorMessage != nullptr)
                    {
                        *errorMessage = "Unknown CV port id in timeline: " + event.portId;
                    }
                    return false;
                }
                if(!ValidateNormalizedValue(event.normalizedValue,
                                            "Timeline CV value",
                                            errorMessage))
                {
                    return false;
                }
                break;

            case RenderTimelineEventType::kGateSet:
                if(validGatePortIds.find(event.portId) == validGatePortIds.end())
                {
                    if(errorMessage != nullptr)
                    {
                        *errorMessage = "Unknown gate port id in timeline: " + event.portId;
                    }
                    return false;
                }
                break;

            case RenderTimelineEventType::kMidi:
                if(!acceptsMidi)
                {
                    if(errorMessage != nullptr)
                    {
                        *errorMessage = "App does not accept MIDI timeline events";
                    }
                    return false;
                }
                break;

            case RenderTimelineEventType::kAudioInputConfig:
            {
                RenderAudioInputConfig updatedConfig = scenario.audioInput;
                if(event.hasAudioMode)
                {
                    updatedConfig.mode = event.audioMode;
                }
                if(event.hasAudioLevel)
                {
                    updatedConfig.level = event.audioLevel;
                }
                if(event.hasAudioFrequency)
                {
                    updatedConfig.frequencyHz = event.audioFrequencyHz;
                }
                if(!ValidateAudioConfig(updatedConfig, errorMessage))
                {
                    return false;
                }
                break;
            }

            case RenderTimelineEventType::kImpulse: break;

            case RenderTimelineEventType::kMenuRotate: break;

            case RenderTimelineEventType::kMenuPress: break;

            case RenderTimelineEventType::kMenuSetItem:
                if(!FindMenuItemId(initialMenu, event.menuItemId))
                {
                    if(errorMessage != nullptr)
                    {
                        *errorMessage = "Unknown menu item id in timeline: "
                                        + event.menuItemId;
                    }
                    return false;
                }
                if(!ValidateNormalizedValue(event.normalizedValue,
                                            "Timeline menu item value",
                                            errorMessage))
                {
                    return false;
                }
                break;
        }

        sortedEvents->push_back({event, index});
    }

    std::stable_sort(sortedEvents->begin(),
                     sortedEvents->end(),
                     [&scenario](const TimelineEventWithIndex& left,
                                 const TimelineEventWithIndex& right) {
                         const auto leftFrame
                             = TimeToFrame(left.event.timeSeconds,
                                           scenario.renderConfig.sampleRate);
                         const auto rightFrame
                             = TimeToFrame(right.event.timeSeconds,
                                           scenario.renderConfig.sampleRate);
                         if(leftFrame != rightFrame)
                         {
                             return leftFrame < rightFrame;
                         }
                         const int leftPriority  = EventPriority(left.event.type);
                         const int rightPriority = EventPriority(right.event.type);
                         if(leftPriority != rightPriority)
                         {
                             return leftPriority < rightPriority;
                         }
                         return left.originalIndex < right.originalIndex;
                     });

    return true;
}

bool ValidateMultiNodeScenario(const RenderScenario&             scenario,
                               const std::vector<ResolvedRenderNode>& nodes,
                               const std::unordered_map<std::string, PortDescriptor>& ports,
                               std::vector<TimelineEventWithIndex>* sortedEvents,
                               std::string*                       errorMessage)
{
    if(!ValidateCommonRenderScenario(scenario, errorMessage))
    {
        return false;
    }
    if(sortedEvents == nullptr)
    {
        return false;
    }

    std::set<std::string> validParameterIds;
    std::set<std::string> validMenuItemIds;
    std::unordered_map<std::string, bool> midiSupportByNodeId;
    for(const auto& node : nodes)
    {
        for(const auto& parameter : node.app->GetParameters())
        {
            validParameterIds.insert(parameter.id);
        }
        const auto menu = node.app->GetMenuModel();
        for(const auto& section : menu.sections)
        {
            for(const auto& item : section.items)
            {
                validMenuItemIds.insert(item.id);
            }
        }
        midiSupportByNodeId[node.config.nodeId]
            = node.app->GetCapabilities().acceptsMidiInput
              && !node.bindings.midiInputPortId.empty();
    }

    for(const auto& entry : scenario.initialParameterValues)
    {
        if(validParameterIds.find(entry.first) == validParameterIds.end())
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Unknown parameter id in initialParameterValues: "
                                + entry.first;
            }
            return false;
        }
        if(!ValidateNormalizedValue(entry.second,
                                    "Initial parameter value for " + entry.first,
                                    errorMessage))
        {
            return false;
        }
    }

    sortedEvents->clear();
    sortedEvents->reserve(scenario.timeline.size());
    for(std::size_t index = 0; index < scenario.timeline.size(); ++index)
    {
        const auto& event = scenario.timeline[index];
        if(!IsFinite(event.timeSeconds) || event.timeSeconds < 0.0
           || event.timeSeconds > scenario.renderConfig.durationSeconds)
        {
            if(errorMessage != nullptr)
            {
                *errorMessage = "Timeline event timeSeconds is out of range";
            }
            return false;
        }

        switch(event.type)
        {
            case RenderTimelineEventType::kParameterSet:
                if(validParameterIds.find(event.parameterId) == validParameterIds.end())
                {
                    if(errorMessage != nullptr)
                    {
                        *errorMessage
                            = "Unknown parameter id in timeline: " + event.parameterId;
                    }
                    return false;
                }
                if(!ValidateNormalizedValue(event.normalizedValue,
                                            "Timeline parameter value",
                                            errorMessage))
                {
                    return false;
                }
                break;

            case RenderTimelineEventType::kCvSet:
            case RenderTimelineEventType::kGateSet:
            {
                const auto portIt = ports.find(event.portId);
                if(portIt == ports.end())
                {
                    if(errorMessage != nullptr)
                    {
                        *errorMessage = std::string(event.type
                                                        == RenderTimelineEventType::kCvSet
                                                    ? "Unknown CV port id in timeline: "
                                                    : "Unknown gate port id in timeline: ")
                                        + event.portId;
                    }
                    return false;
                }
                const auto expectedType = event.type == RenderTimelineEventType::kCvSet
                                              ? VirtualPortType::kCv
                                              : VirtualPortType::kGate;
                if(portIt->second.type != expectedType
                   || portIt->second.direction != PortDirection::kInput)
                {
                    if(errorMessage != nullptr)
                    {
                        *errorMessage = "Timeline port targets the wrong port type: "
                                        + event.portId;
                    }
                    return false;
                }
                if(event.type == RenderTimelineEventType::kCvSet
                   && !ValidateNormalizedValue(event.normalizedValue,
                                              "Timeline CV value",
                                              errorMessage))
                {
                    return false;
                }
                break;
            }

            case RenderTimelineEventType::kMidi:
            {
                const auto nodeIndex
                    = ResolveEventNodeIndex(event, scenario, nodes, errorMessage);
                if(nodeIndex >= nodes.size())
                {
                    return false;
                }
                const auto midiSupportIt
                    = midiSupportByNodeId.find(nodes[nodeIndex].config.nodeId);
                if(midiSupportIt == midiSupportByNodeId.end() || !midiSupportIt->second)
                {
                    if(errorMessage != nullptr)
                    {
                        *errorMessage = "App does not accept MIDI timeline events";
                    }
                    return false;
                }
                break;
            }

            case RenderTimelineEventType::kAudioInputConfig:
            {
                if(ResolveEventNodeIndex(event, scenario, nodes, errorMessage) >= nodes.size())
                {
                    return false;
                }
                RenderAudioInputConfig updatedConfig = scenario.audioInput;
                if(event.hasAudioMode)
                {
                    updatedConfig.mode = event.audioMode;
                }
                if(event.hasAudioLevel)
                {
                    updatedConfig.level = event.audioLevel;
                }
                if(event.hasAudioFrequency)
                {
                    updatedConfig.frequencyHz = event.audioFrequencyHz;
                }
                if(!ValidateAudioConfig(updatedConfig, errorMessage))
                {
                    return false;
                }
                break;
            }

            case RenderTimelineEventType::kImpulse:
            case RenderTimelineEventType::kMenuRotate:
            case RenderTimelineEventType::kMenuPress:
                if(ResolveEventNodeIndex(event, scenario, nodes, errorMessage) >= nodes.size())
                {
                    return false;
                }
                break;

            case RenderTimelineEventType::kMenuSetItem:
                if(validMenuItemIds.find(event.menuItemId) == validMenuItemIds.end())
                {
                    if(errorMessage != nullptr)
                    {
                        *errorMessage = "Unknown menu item id in timeline: "
                                        + event.menuItemId;
                    }
                    return false;
                }
                if(!ValidateNormalizedValue(event.normalizedValue,
                                            "Timeline menu item value",
                                            errorMessage))
                {
                    return false;
                }
                break;
        }

        sortedEvents->push_back({event, index});
    }

    std::stable_sort(sortedEvents->begin(),
                     sortedEvents->end(),
                     [&scenario](const TimelineEventWithIndex& left,
                                 const TimelineEventWithIndex& right) {
                         const auto leftFrame
                             = TimeToFrame(left.event.timeSeconds,
                                           scenario.renderConfig.sampleRate);
                         const auto rightFrame
                             = TimeToFrame(right.event.timeSeconds,
                                           scenario.renderConfig.sampleRate);
                         if(leftFrame != rightFrame)
                         {
                             return leftFrame < rightFrame;
                         }
                         const int leftPriority  = EventPriority(left.event.type);
                         const int rightPriority = EventPriority(right.event.type);
                         if(leftPriority != rightPriority)
                         {
                             return leftPriority < rightPriority;
                         }
                         return left.originalIndex < right.originalIndex;
                     });

    return true;
}

void FillZero(std::vector<float>* buffer, std::size_t count)
{
    if(buffer == nullptr)
    {
        return;
    }
    if(buffer->size() < count)
    {
        buffer->assign(count, 0.0f);
    }
    else
    {
        std::fill(buffer->begin(), buffer->begin() + static_cast<std::ptrdiff_t>(count), 0.0f);
    }
}

bool RunMultiNodeRenderScenario(const RenderScenario& scenario,
                                RenderResult*         result,
                                std::string*          errorMessage)
{
    std::vector<ResolvedRenderNode>                 nodes;
    std::unordered_map<std::string, PortDescriptor> ports;
    if(!ResolveMultiNodeScenario(scenario, &nodes, &ports, errorMessage))
    {
        return false;
    }

    std::vector<TimelineEventWithIndex> sortedEvents;
    if(!ValidateMultiNodeScenario(scenario, nodes, ports, &sortedEvents, errorMessage))
    {
        return false;
    }

    const std::size_t primaryIndex    = FindPrimaryNodeIndex(nodes);
    const std::size_t selectedIndex   = FindNodeIndexById(nodes, scenario.selectedNodeId);
    const std::size_t entryNodeIndex  = FindNodeIndexById(nodes, scenario.entryNodeId);
    const std::size_t outputNodeIndex = FindNodeIndexById(nodes, scenario.outputNodeId);
    if(primaryIndex >= nodes.size() || selectedIndex >= nodes.size()
       || entryNodeIndex >= nodes.size() || outputNodeIndex >= nodes.size())
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Rack globals reference an unknown node";
        }
        return false;
    }

    LiveRackTopologyPreset topologyPreset = LiveRackTopologyPreset::kNode0Only;
    if(!TryInferLiveRackTopologyPreset(
           MakeLiveRackTopologyConfig(scenario), &topologyPreset, errorMessage))
    {
        return false;
    }

    for(auto& node : nodes)
    {
        node.app->Prepare(scenario.renderConfig.sampleRate, scenario.renderConfig.blockSize);
        node.app->ResetToDefaultState(node.config.seed);
    }

    if(!scenario.initialParameterValues.empty())
    {
        std::vector<std::unordered_map<std::string, float>> restoredParameters(nodes.size());
        for(const auto& entry : scenario.initialParameterValues)
        {
            const auto nodeIndex = FindNodeIndexForQualifiedId(nodes, entry.first);
            if(nodeIndex < nodes.size())
            {
                restoredParameters[nodeIndex][entry.first] = entry.second;
            }
        }
        for(std::size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
        {
            if(!restoredParameters[nodeIndex].empty())
            {
                nodes[nodeIndex].app->RestoreStatefulParameterValues(
                    restoredParameters[nodeIndex]);
            }
        }
    }

    const auto totalFrames = TimeToFrame(scenario.renderConfig.durationSeconds,
                                         scenario.renderConfig.sampleRate);
    const auto blockSize   = scenario.renderConfig.blockSize;
    const auto outputChannelCount
        = static_cast<std::size_t>(scenario.renderConfig.outputChannelCount);
    std::vector<RenderNodeInputState> inputStates(nodes.size());
    for(std::size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
    {
        inputStates[nodeIndex].audioInput = scenario.audioInput;
        inputStates[nodeIndex].noiseState
            = nodes[nodeIndex].config.seed == 0 ? 1u : nodes[nodeIndex].config.seed;
    }
    std::size_t eventIndex = 0;

    result->audioChannels.assign(outputChannelCount,
                                 std::vector<float>(totalFrames, 0.0f));
    result->manifest                   = {};
    result->manifest.appId             = nodes[primaryIndex].config.appId;
    result->manifest.boardId           = scenario.boardId;
    result->manifest.selectedNodeId    = scenario.selectedNodeId;
    result->manifest.entryNodeId       = scenario.entryNodeId;
    result->manifest.outputNodeId      = scenario.outputNodeId;
    result->manifest.appDisplayName    = nodes[primaryIndex].app->GetAppDisplayName();
    result->manifest.renderConfig      = scenario.renderConfig;
    result->manifest.frameCount        = totalFrames;
    result->manifest.seed              = nodes[primaryIndex].config.seed;
    result->manifest.initialAudioInput = scenario.audioInput;
    result->manifest.routes            = scenario.routes;

    result->manifest.nodes.reserve(nodes.size());
    for(const auto& node : nodes)
    {
        RenderNodeResultSummary summary;
        summary.nodeId                 = node.config.nodeId;
        summary.appId                  = node.config.appId;
        summary.appDisplayName         = node.app->GetAppDisplayName();
        summary.seed                   = node.config.seed;
        const auto initialParameters   = node.app->CaptureStatefulParameterValues();
        summary.initialParameterValues = std::map<std::string, float>(
            initialParameters.begin(), initialParameters.end());
        summary.finalCvInputs   = node.currentCvInputs;
        summary.finalGateInputs = node.currentGateInputs;
        result->manifest.nodes.push_back(std::move(summary));
    }
    result->manifest.initialParameterValues
        = result->manifest.nodes[primaryIndex].initialParameterValues;

    std::vector<float> inputBuffer(blockSize, 0.0f);
    std::vector<float> zeroBuffer(blockSize, 0.0f);
    std::array<std::array<std::vector<float>, kMaxAudioChannels>, 2> nodeOutputs;
    std::array<std::array<std::vector<float>, kMaxAudioChannels>, 2> routedInputs;
    for(auto& nodeBuffers : nodeOutputs)
    {
        for(auto& channel : nodeBuffers)
        {
            channel.assign(blockSize, 0.0f);
        }
    }
    for(auto& nodeBuffers : routedInputs)
    {
        for(auto& channel : nodeBuffers)
        {
            channel.assign(blockSize, 0.0f);
        }
    }

    std::uint64_t currentFrame = 0;
    while(currentFrame < totalFrames)
    {
        std::array<std::vector<MidiMessageEvent>, 2> midiEventsForSegment;

        while(eventIndex < sortedEvents.size()
              && TimeToFrame(sortedEvents[eventIndex].event.timeSeconds,
                             scenario.renderConfig.sampleRate)
                     == currentFrame)
        {
            const auto& event = sortedEvents[eventIndex].event;
            result->manifest.executedTimeline.push_back(event);

            switch(event.type)
            {
                case RenderTimelineEventType::kParameterSet:
                {
                    const auto nodeIndex = FindNodeIndexForQualifiedId(nodes, event.parameterId);
                    if(nodeIndex < nodes.size())
                    {
                        nodes[nodeIndex].app->SetParameterValue(event.parameterId,
                                                                event.normalizedValue);
                    }
                    break;
                }

                case RenderTimelineEventType::kCvSet:
                {
                    const auto nodeIndex = FindNodeIndexForQualifiedId(nodes, event.portId);
                    if(nodeIndex < nodes.size())
                    {
                        nodes[nodeIndex].currentCvInputs[event.portId]
                            = event.normalizedValue;
                    }
                    break;
                }

                case RenderTimelineEventType::kGateSet:
                {
                    const auto nodeIndex = FindNodeIndexForQualifiedId(nodes, event.portId);
                    if(nodeIndex < nodes.size())
                    {
                        nodes[nodeIndex].currentGateInputs[event.portId]
                            = event.gateValue;
                    }
                    break;
                }

                case RenderTimelineEventType::kMidi:
                {
                    const auto nodeIndex
                        = ResolveEventNodeIndex(event, scenario, nodes, nullptr);
                    if(nodeIndex < nodes.size())
                    {
                        midiEventsForSegment[nodeIndex].push_back(event.midiMessage);
                    }
                    break;
                }

                case RenderTimelineEventType::kAudioInputConfig:
                {
                    const auto nodeIndex
                        = ResolveEventNodeIndex(event, scenario, nodes, nullptr);
                    if(nodeIndex < nodes.size() && event.hasAudioMode)
                    {
                        inputStates[nodeIndex].audioInput.mode = event.audioMode;
                    }
                    if(nodeIndex < nodes.size() && event.hasAudioLevel)
                    {
                        inputStates[nodeIndex].audioInput.level = event.audioLevel;
                    }
                    if(nodeIndex < nodes.size() && event.hasAudioFrequency)
                    {
                        inputStates[nodeIndex].audioInput.frequencyHz
                            = event.audioFrequencyHz;
                    }
                    break;
                }

                case RenderTimelineEventType::kImpulse:
                {
                    const auto nodeIndex
                        = ResolveEventNodeIndex(event, scenario, nodes, nullptr);
                    if(nodeIndex < nodes.size())
                    {
                        inputStates[nodeIndex].impulseRequested = true;
                    }
                    break;
                }

                case RenderTimelineEventType::kMenuRotate:
                {
                    const auto nodeIndex
                        = ResolveEventNodeIndex(event, scenario, nodes, nullptr);
                    if(nodeIndex < nodes.size())
                    {
                        nodes[nodeIndex].app->MenuRotate(event.menuDelta);
                    }
                    break;
                }

                case RenderTimelineEventType::kMenuPress:
                {
                    const auto nodeIndex
                        = ResolveEventNodeIndex(event, scenario, nodes, nullptr);
                    if(nodeIndex < nodes.size())
                    {
                        nodes[nodeIndex].app->MenuPress();
                    }
                    break;
                }

                case RenderTimelineEventType::kMenuSetItem:
                {
                    const auto nodeIndex = FindNodeIndexForQualifiedId(nodes, event.menuItemId);
                    if(nodeIndex < nodes.size())
                    {
                        nodes[nodeIndex].app->SetMenuItemValue(event.menuItemId,
                                                               event.normalizedValue);
                    }
                    break;
                }
            }

            ++eventIndex;
        }

        std::uint64_t nextEventFrame = totalFrames;
        if(eventIndex < sortedEvents.size())
        {
            nextEventFrame = TimeToFrame(sortedEvents[eventIndex].event.timeSeconds,
                                         scenario.renderConfig.sampleRate);
        }

        const auto framesUntilNextEvent = nextEventFrame > currentFrame
                                              ? nextEventFrame - currentFrame
                                              : 0ull;
        const auto segmentFrames = static_cast<std::size_t>(
            std::min<std::uint64_t>(blockSize,
                                    framesUntilNextEvent == 0 ? 1 : framesUntilNextEvent));
        if(segmentFrames == 0)
        {
            break;
        }

        std::array<std::size_t, 2> processingOrder = {primaryIndex, outputNodeIndex};
        switch(topologyPreset)
        {
            case LiveRackTopologyPreset::kNode0Only:
                processingOrder = {FindNodeIndexById(nodes, "node0"), nodes.size()};
                break;
            case LiveRackTopologyPreset::kNode1Only:
                processingOrder = {FindNodeIndexById(nodes, "node1"), nodes.size()};
                break;
            case LiveRackTopologyPreset::kNode0ToNode1:
                processingOrder = {FindNodeIndexById(nodes, "node0"),
                                   FindNodeIndexById(nodes, "node1")};
                break;
            case LiveRackTopologyPreset::kNode1ToNode0:
                processingOrder = {FindNodeIndexById(nodes, "node1"),
                                   FindNodeIndexById(nodes, "node0")};
                break;
        }

        for(const auto nodeIndex : processingOrder)
        {
            if(nodeIndex >= nodes.size())
            {
                continue;
            }
            auto& node = nodes[nodeIndex];
            for(const auto& entry : node.currentCvInputs)
            {
                PortValue value;
                value.type   = VirtualPortType::kCv;
                value.scalar = entry.second;
                node.app->SetPortInput(entry.first, value);
            }

            for(const auto& entry : node.currentGateInputs)
            {
                PortValue value;
                value.type = VirtualPortType::kGate;
                value.gate = entry.second;
                node.app->SetPortInput(entry.first, value);
            }

            if(!node.bindings.midiInputPortId.empty())
            {
                PortValue midiValue;
                midiValue.type       = VirtualPortType::kMidi;
                midiValue.midiEvents = midiEventsForSegment[nodeIndex];
                node.app->SetPortInput(node.bindings.midiInputPortId, midiValue);
            }

            FillZero(&inputBuffer, segmentFrames);
            FillZero(&zeroBuffer, segmentFrames);
            for(auto& channel : nodeOutputs[nodeIndex])
            {
                FillZero(&channel, segmentFrames);
            }
            for(auto& channel : routedInputs[nodeIndex])
            {
                FillZero(&channel, segmentFrames);
            }

            std::array<const float*, kMaxAudioChannels> inputPointers = {
                {zeroBuffer.data(), zeroBuffer.data(), zeroBuffer.data(), zeroBuffer.data()}};
            if(nodeIndex == entryNodeIndex)
            {
                if(inputStates[nodeIndex].audioInput.mode
                   != static_cast<int>(TestInputSignalMode::kHostInput))
                {
                    GenerateSyntheticTestInput(
                        inputStates[nodeIndex].audioInput.mode,
                        std::clamp(inputStates[nodeIndex].audioInput.level / 10.0f,
                                   0.0f,
                                   1.0f),
                        inputStates[nodeIndex].audioInput.frequencyHz,
                        scenario.renderConfig.sampleRate,
                        &inputStates[nodeIndex].testPhase,
                        &inputStates[nodeIndex].noiseState,
                        &inputStates[nodeIndex].impulseRequested,
                        inputBuffer.data(),
                        static_cast<int>(segmentFrames));
                    inputPointers[0] = inputBuffer.data();
                }
            }
            else
            {
                for(const auto& route : scenario.routes)
                {
                    const auto sourcePort = ports.at(route.sourcePortId);
                    const auto destPort   = ports.at(route.destPortId);
                    if(destPort.nodeId != node.config.nodeId)
                    {
                        continue;
                    }

                    const auto sourceNodeIt = std::find_if(
                        nodes.begin(),
                        nodes.end(),
                        [&sourcePort](const ResolvedRenderNode& candidate) {
                            return candidate.config.nodeId == sourcePort.nodeId;
                        });
                    if(sourceNodeIt == nodes.end())
                    {
                        continue;
                    }

                    const auto sourceNodeIndex = static_cast<std::size_t>(
                        std::distance(nodes.begin(), sourceNodeIt));
                    std::copy_n(nodeOutputs[sourceNodeIndex][sourcePort.channelIndex].begin(),
                                static_cast<std::ptrdiff_t>(segmentFrames),
                                routedInputs[nodeIndex][destPort.channelIndex].begin());
                    inputPointers[destPort.channelIndex]
                        = routedInputs[nodeIndex][destPort.channelIndex].data();
                }
            }

            std::array<float*, kMaxAudioChannels> outputPointers = {
                {nodeOutputs[nodeIndex][0].data(),
                 nodeOutputs[nodeIndex][1].data(),
                 nodeOutputs[nodeIndex][2].data(),
                 nodeOutputs[nodeIndex][3].data()}};

            node.app->Process({inputPointers.data(), kMaxAudioChannels},
                              {outputPointers.data(), kMaxAudioChannels},
                              segmentFrames);
            node.app->TickUi((1000.0 * static_cast<double>(segmentFrames))
                             / scenario.renderConfig.sampleRate);
        }

        const auto&       outputBindings  = nodes[outputNodeIndex].bindings;
        for(std::size_t frame = 0; frame < segmentFrames; ++frame)
        {
            const auto destinationFrame = currentFrame + frame;
            switch(outputChannelCount)
            {
                case 1:
                    result->audioChannels[0][destinationFrame]
                        = nodeOutputs[outputNodeIndex][std::clamp(
                              outputBindings.mainOutputChannels[0], 0, 3)][frame];
                    break;

                case 2:
                    result->audioChannels[0][destinationFrame]
                        = nodeOutputs[outputNodeIndex][std::clamp(
                              outputBindings.mainOutputChannels[0], 0, 3)][frame];
                    result->audioChannels[1][destinationFrame]
                        = nodeOutputs[outputNodeIndex][std::clamp(
                              outputBindings.mainOutputChannels[1], 0, 3)][frame];
                    break;

                case 4:
                    for(std::size_t channel = 0; channel < 4; ++channel)
                    {
                        result->audioChannels[channel][destinationFrame]
                            = nodeOutputs[outputNodeIndex][channel][frame];
                    }
                    break;
            }
        }

        currentFrame += segmentFrames;
    }

    for(std::size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
    {
        const auto finalParameters = nodes[nodeIndex].app->CaptureStatefulParameterValues();
        result->manifest.nodes[nodeIndex].finalParameterValues
            = std::map<std::string, float>(finalParameters.begin(), finalParameters.end());
        result->manifest.nodes[nodeIndex].finalEffectiveParameterValues
            = CaptureEffectiveParameterValues(*nodes[nodeIndex].app);
        result->manifest.nodes[nodeIndex].finalCvInputs   = nodes[nodeIndex].currentCvInputs;
        result->manifest.nodes[nodeIndex].finalGateInputs = nodes[nodeIndex].currentGateInputs;
    }

    result->manifest.finalParameterValues
        = result->manifest.nodes[primaryIndex].finalParameterValues;
    result->manifest.finalEffectiveParameterValues
        = result->manifest.nodes[primaryIndex].finalEffectiveParameterValues;
    result->manifest.finalCvInputs   = result->manifest.nodes[primaryIndex].finalCvInputs;
    result->manifest.finalGateInputs = result->manifest.nodes[primaryIndex].finalGateInputs;
    result->manifest.finalAudioInput = inputStates[entryNodeIndex].audioInput;
    result->manifest.audioChecksum   = HashAudioChannels(result->audioChannels);
    result->manifest.channelSummaries.clear();
    result->manifest.channelSummaries.reserve(result->audioChannels.size());
    for(const auto& channel : result->audioChannels)
    {
        result->manifest.channelSummaries.push_back(SummarizeChannel(channel));
    }

    return true;
}

} // namespace

const char* GetRenderTimelineEventTypeName(RenderTimelineEventType type)
{
    switch(type)
    {
        case RenderTimelineEventType::kParameterSet: return "parameter_set";
        case RenderTimelineEventType::kCvSet: return "cv_set";
        case RenderTimelineEventType::kGateSet: return "gate_set";
        case RenderTimelineEventType::kMidi: return "midi";
        case RenderTimelineEventType::kAudioInputConfig: return "audio_input_config";
        case RenderTimelineEventType::kImpulse: return "impulse";
        case RenderTimelineEventType::kMenuRotate: return "menu_rotate";
        case RenderTimelineEventType::kMenuPress: return "menu_press";
        case RenderTimelineEventType::kMenuSetItem: return "menu_set_item";
    }

    return "parameter_set";
}

bool TryParseRenderTimelineEventType(const std::string& text,
                                     RenderTimelineEventType* type)
{
    const auto normalized = NormalizeToken(text);
    if(normalized == "parameter_set")
    {
        *type = RenderTimelineEventType::kParameterSet;
        return true;
    }
    if(normalized == "cv_set")
    {
        *type = RenderTimelineEventType::kCvSet;
        return true;
    }
    if(normalized == "gate_set")
    {
        *type = RenderTimelineEventType::kGateSet;
        return true;
    }
    if(normalized == "midi")
    {
        *type = RenderTimelineEventType::kMidi;
        return true;
    }
    if(normalized == "audio_input_config")
    {
        *type = RenderTimelineEventType::kAudioInputConfig;
        return true;
    }
    if(normalized == "impulse")
    {
        *type = RenderTimelineEventType::kImpulse;
        return true;
    }
    if(normalized == "menu_rotate")
    {
        *type = RenderTimelineEventType::kMenuRotate;
        return true;
    }
    if(normalized == "menu_press")
    {
        *type = RenderTimelineEventType::kMenuPress;
        return true;
    }
    if(normalized == "menu_set_item")
    {
        *type = RenderTimelineEventType::kMenuSetItem;
        return true;
    }
    return false;
}

bool TryParseRenderAudioInputMode(const std::string& text, int* mode)
{
    const auto normalized = NormalizeToken(text);
    if(normalized == "host_in" || normalized == "host_input" || normalized == "host")
    {
        *mode = static_cast<int>(TestInputSignalMode::kHostInput);
        return true;
    }
    if(normalized == "sine")
    {
        *mode = static_cast<int>(TestInputSignalMode::kSineInput);
        return true;
    }
    if(normalized == "saw")
    {
        *mode = static_cast<int>(TestInputSignalMode::kSawInput);
        return true;
    }
    if(normalized == "noise")
    {
        *mode = static_cast<int>(TestInputSignalMode::kNoiseInput);
        return true;
    }
    if(normalized == "impulse")
    {
        *mode = static_cast<int>(TestInputSignalMode::kImpulseInput);
        return true;
    }
    if(normalized == "triangle")
    {
        *mode = static_cast<int>(TestInputSignalMode::kTriangleInput);
        return true;
    }
    if(normalized == "square")
    {
        *mode = static_cast<int>(TestInputSignalMode::kSquareInput);
        return true;
    }
    return false;
}

std::string GetRenderAudioInputModeName(int mode)
{
    return NormalizeToken(GetTestInputSignalModeName(mode));
}

bool ParseRenderScenarioJson(const std::string& jsonText,
                             RenderScenario*    scenario,
                             std::string*       errorMessage)
{
    if(scenario == nullptr)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "RenderScenario output pointer must not be null";
        }
        return false;
    }

    const juce::var parsed = juce::JSON::parse(jsonText);
    const auto*     root   = ExpectObject(parsed, "scenario", errorMessage);
    if(root == nullptr)
    {
        if(errorMessage != nullptr && errorMessage->empty())
        {
            *errorMessage = "Failed to parse scenario JSON";
        }
        return false;
    }

    RenderScenario parsedScenario;
    if(!ReadRequiredString(*root, "appId", &parsedScenario.appId, errorMessage))
    {
        return false;
    }
    if(!ReadOptionalString(
           *root, "boardId", &parsedScenario.boardId, nullptr, errorMessage))
    {
        return false;
    }
    if(!ReadOptionalString(*root,
                           "selectedNodeId",
                           &parsedScenario.selectedNodeId,
                           nullptr,
                           errorMessage))
    {
        return false;
    }
    if(!ReadOptionalString(
           *root, "entryNodeId", &parsedScenario.entryNodeId, nullptr, errorMessage))
    {
        return false;
    }
    if(!ReadOptionalString(*root,
                           "outputNodeId",
                           &parsedScenario.outputNodeId,
                           nullptr,
                           errorMessage))
    {
        return false;
    }

    const auto renderConfigVar = root->getProperty("renderConfig");
    if(!renderConfigVar.isVoid())
    {
        const auto* renderConfigObject
            = ExpectObject(renderConfigVar, "renderConfig", errorMessage);
        if(renderConfigObject == nullptr)
        {
            return false;
        }

        double numberValue = 0.0;
        bool   present     = false;
        if(!ReadOptionalNumber(*renderConfigObject,
                               "sampleRate",
                               &numberValue,
                               &present,
                               errorMessage))
        {
            return false;
        }
        if(present)
        {
            parsedScenario.renderConfig.sampleRate = numberValue;
        }

        if(!ReadOptionalNumber(*renderConfigObject,
                               "blockSize",
                               &numberValue,
                               &present,
                               errorMessage))
        {
            return false;
        }
        if(present)
        {
            parsedScenario.renderConfig.blockSize
                = static_cast<std::size_t>(std::llround(numberValue));
        }

        if(!ReadOptionalNumber(*renderConfigObject,
                               "durationSeconds",
                               &numberValue,
                               &present,
                               errorMessage))
        {
            return false;
        }
        if(present)
        {
            parsedScenario.renderConfig.durationSeconds = numberValue;
        }

        if(!ReadOptionalNumber(*renderConfigObject,
                               "outputChannelCount",
                               &numberValue,
                               &present,
                               errorMessage))
        {
            return false;
        }
        if(present)
        {
            parsedScenario.renderConfig.outputChannelCount
                = static_cast<int>(std::llround(numberValue));
        }
    }

    double seedValue = 0.0;
    bool   hasSeed   = false;
    if(!ReadOptionalNumber(*root, "seed", &seedValue, &hasSeed, errorMessage))
    {
        return false;
    }
    if(hasSeed)
    {
        parsedScenario.seed = static_cast<std::uint32_t>(std::llround(seedValue));
    }

    const auto nodesVar = root->getProperty("nodes");
    if(!nodesVar.isVoid())
    {
        const auto* nodeArray = ExpectArray(nodesVar, "nodes", errorMessage);
        if(nodeArray == nullptr)
        {
            return false;
        }

        parsedScenario.nodes.reserve(static_cast<std::size_t>(nodeArray->size()));
        for(const auto& item : *nodeArray)
        {
            RenderNodeConfig node;
            if(!ParseRenderNodeObject(item, parsedScenario.seed, &node, errorMessage))
            {
                return false;
            }
            parsedScenario.nodes.push_back(node);
        }
    }

    const auto routesVar = root->getProperty("routes");
    if(!routesVar.isVoid())
    {
        const auto* routeArray = ExpectArray(routesVar, "routes", errorMessage);
        if(routeArray == nullptr)
        {
            return false;
        }

        parsedScenario.routes.reserve(static_cast<std::size_t>(routeArray->size()));
        for(const auto& item : *routeArray)
        {
            RenderRoute route;
            if(!ParseRenderRouteObject(item, &route, errorMessage))
            {
                return false;
            }
            parsedScenario.routes.push_back(route);
        }
    }

    const auto parameterValuesVar = root->getProperty("initialParameterValues");
    if(!parameterValuesVar.isVoid())
    {
        const auto* parameterObject
            = ExpectObject(parameterValuesVar, "initialParameterValues", errorMessage);
        if(parameterObject == nullptr)
        {
            return false;
        }

        for(const auto& property : parameterObject->getProperties())
        {
            const auto& propertyValue = property.value;
            if(!propertyValue.isDouble() && !propertyValue.isInt()
               && !propertyValue.isInt64())
            {
                if(errorMessage != nullptr)
                {
                    *errorMessage = "initialParameterValues entries must be numeric";
                }
                return false;
            }

            parsedScenario.initialParameterValues[property.name.toString().toStdString()]
                = static_cast<float>(propertyValue);
        }
    }

    const auto audioInputVar = root->getProperty("audioInput");
    if(!audioInputVar.isVoid())
    {
        const auto* audioInputObject
            = ExpectObject(audioInputVar, "audioInput", errorMessage);
        if(audioInputObject == nullptr)
        {
            return false;
        }
        if(!ParseAudioInputConfigObject(
               *audioInputObject, &parsedScenario.audioInput, errorMessage))
        {
            return false;
        }
    }

    const auto timelineVar = root->getProperty("timeline");
    if(!timelineVar.isVoid())
    {
        const auto* timelineArray = ExpectArray(timelineVar, "timeline", errorMessage);
        if(timelineArray == nullptr)
        {
            return false;
        }

        parsedScenario.timeline.reserve(static_cast<std::size_t>(timelineArray->size()));
        for(const auto& item : *timelineArray)
        {
            RenderTimelineEvent event;
            if(!ParseTimelineEvent(item, &event, errorMessage))
            {
                return false;
            }
            parsedScenario.timeline.push_back(event);
        }
    }

    *scenario = std::move(parsedScenario);
    return true;
}

bool LoadRenderScenarioFromFile(const std::filesystem::path& scenarioPath,
                                RenderScenario*              scenario,
                                std::string*                 errorMessage)
{
    juce::File file(juce::String(scenarioPath.u8string()));
    if(!file.existsAsFile())
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Scenario file does not exist: "
                            + scenarioPath.generic_string();
        }
        return false;
    }

    return ParseRenderScenarioJson(file.loadFileAsString().toStdString(),
                                   scenario,
                                   errorMessage);
}

bool RunRenderScenario(const RenderScenario& scenario,
                       RenderResult*         result,
                       std::string*          errorMessage)
{
    if(result == nullptr)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "RenderResult output pointer must not be null";
        }
        return false;
    }

    if(!scenario.nodes.empty() || !scenario.routes.empty())
    {
        return RunMultiNodeRenderScenario(scenario, result, errorMessage);
    }

    std::string resolvedAppId;
    auto app = CreateHostedAppCore(scenario.appId, "node0", &resolvedAppId);
    if(app == nullptr || resolvedAppId != scenario.appId)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Unknown appId: " + scenario.appId;
        }
        return false;
    }

    std::vector<TimelineEventWithIndex> sortedEvents;
    if(!ValidateScenario(scenario, *app, &sortedEvents, errorMessage))
    {
        return false;
    }

    app->Prepare(scenario.renderConfig.sampleRate, scenario.renderConfig.blockSize);
    app->ResetToDefaultState(scenario.seed);

    std::unordered_map<std::string, float> restoredParameters;
    for(const auto& entry : scenario.initialParameterValues)
    {
        restoredParameters[entry.first] = entry.second;
    }
    if(!restoredParameters.empty())
    {
        app->RestoreStatefulParameterValues(restoredParameters);
    }

    const auto bindings      = app->GetPatchBindings();
    const auto totalFrames   = TimeToFrame(scenario.renderConfig.durationSeconds,
                                         scenario.renderConfig.sampleRate);
    const auto blockSize     = scenario.renderConfig.blockSize;
    const auto outputChannelCount
        = static_cast<std::size_t>(scenario.renderConfig.outputChannelCount);

    std::map<std::string, float> currentCvInputs;
    std::map<std::string, bool>  currentGateInputs;
    for(const auto& portId : bindings.cvInputPortIds)
    {
        if(!portId.empty())
        {
            currentCvInputs[portId] = 0.5f;
        }
    }
    for(const auto& portId : bindings.gateInputPortIds)
    {
        if(!portId.empty())
        {
            currentGateInputs[portId] = false;
        }
    }

    RenderAudioInputConfig currentAudioInput = scenario.audioInput;
    float                  testPhase         = 0.0f;
    std::uint32_t          noiseState        = scenario.seed == 0 ? 1u : scenario.seed;
    bool                   impulseRequested  = false;
    std::size_t            eventIndex        = 0;

    result->audioChannels.assign(outputChannelCount,
                                 std::vector<float>(totalFrames, 0.0f));
    result->manifest = {};
    result->manifest.appId             = resolvedAppId;
    result->manifest.boardId           = scenario.boardId;
    result->manifest.selectedNodeId    = scenario.selectedNodeId;
    result->manifest.entryNodeId       = scenario.entryNodeId;
    result->manifest.outputNodeId      = scenario.outputNodeId;
    result->manifest.appDisplayName    = app->GetAppDisplayName();
    result->manifest.renderConfig      = scenario.renderConfig;
    result->manifest.frameCount        = totalFrames;
    result->manifest.seed              = scenario.seed;
    const auto initialParameterSnapshot = app->CaptureStatefulParameterValues();
    result->manifest.initialParameterValues
        = std::map<std::string, float>(initialParameterSnapshot.begin(),
                                       initialParameterSnapshot.end());
    result->manifest.initialAudioInput = scenario.audioInput;

    std::vector<float> inputBuffer(blockSize, 0.0f);
    std::vector<float> zeroBuffer(blockSize, 0.0f);
    std::array<std::vector<float>, kMaxAudioChannels> scratchOutput;
    for(auto& channel : scratchOutput)
    {
        channel.assign(blockSize, 0.0f);
    }

    std::uint64_t currentFrame = 0;
    while(currentFrame < totalFrames)
    {
        std::vector<MidiMessageEvent> midiEventsForSegment;

        while(eventIndex < sortedEvents.size()
              && TimeToFrame(sortedEvents[eventIndex].event.timeSeconds,
                             scenario.renderConfig.sampleRate)
                     == currentFrame)
        {
            const auto& event = sortedEvents[eventIndex].event;
            result->manifest.executedTimeline.push_back(event);

            switch(event.type)
            {
                case RenderTimelineEventType::kParameterSet:
                    app->SetParameterValue(event.parameterId, event.normalizedValue);
                    break;

                case RenderTimelineEventType::kCvSet:
                    currentCvInputs[event.portId] = event.normalizedValue;
                    break;

                case RenderTimelineEventType::kGateSet:
                    currentGateInputs[event.portId] = event.gateValue;
                    break;

                case RenderTimelineEventType::kMidi:
                    midiEventsForSegment.push_back(event.midiMessage);
                    break;

                case RenderTimelineEventType::kAudioInputConfig:
                    if(event.hasAudioMode)
                    {
                        currentAudioInput.mode = event.audioMode;
                    }
                    if(event.hasAudioLevel)
                    {
                        currentAudioInput.level = event.audioLevel;
                    }
                    if(event.hasAudioFrequency)
                    {
                        currentAudioInput.frequencyHz = event.audioFrequencyHz;
                    }
                    break;

                case RenderTimelineEventType::kImpulse:
                    impulseRequested = true;
                    break;

                case RenderTimelineEventType::kMenuRotate:
                    app->MenuRotate(event.menuDelta);
                    break;

                case RenderTimelineEventType::kMenuPress:
                    app->MenuPress();
                    break;

                case RenderTimelineEventType::kMenuSetItem:
                    app->SetMenuItemValue(event.menuItemId, event.normalizedValue);
                    break;
            }

            ++eventIndex;
        }

        std::uint64_t nextEventFrame = totalFrames;
        if(eventIndex < sortedEvents.size())
        {
            nextEventFrame = TimeToFrame(sortedEvents[eventIndex].event.timeSeconds,
                                         scenario.renderConfig.sampleRate);
        }

        const auto framesUntilNextEvent = nextEventFrame > currentFrame
                                              ? nextEventFrame - currentFrame
                                              : 0ull;
        const auto segmentFrames = static_cast<std::size_t>(
            std::min<std::uint64_t>(blockSize,
                                    framesUntilNextEvent == 0 ? 1 : framesUntilNextEvent));
        if(segmentFrames == 0)
        {
            break;
        }

        for(const auto& entry : currentCvInputs)
        {
            PortValue value;
            value.type   = VirtualPortType::kCv;
            value.scalar = entry.second;
            app->SetPortInput(entry.first, value);
        }

        for(const auto& entry : currentGateInputs)
        {
            PortValue value;
            value.type = VirtualPortType::kGate;
            value.gate = entry.second;
            app->SetPortInput(entry.first, value);
        }

        if(!bindings.midiInputPortId.empty())
        {
            PortValue midiValue;
            midiValue.type       = VirtualPortType::kMidi;
            midiValue.midiEvents = midiEventsForSegment;
            app->SetPortInput(bindings.midiInputPortId, midiValue);
        }

        FillZero(&inputBuffer, segmentFrames);
        FillZero(&zeroBuffer, segmentFrames);
        for(auto& channel : scratchOutput)
        {
            FillZero(&channel, segmentFrames);
        }

        std::array<const float*, kMaxAudioChannels> inputPointers = {
            {zeroBuffer.data(), zeroBuffer.data(), zeroBuffer.data(), zeroBuffer.data()}};
        if(currentAudioInput.mode != static_cast<int>(TestInputSignalMode::kHostInput))
        {
            GenerateSyntheticTestInput(currentAudioInput.mode,
                                       std::clamp(currentAudioInput.level / 10.0f, 0.0f, 1.0f),
                                       currentAudioInput.frequencyHz,
                                       scenario.renderConfig.sampleRate,
                                       &testPhase,
                                       &noiseState,
                                       &impulseRequested,
                                       inputBuffer.data(),
                                       static_cast<int>(segmentFrames));
            inputPointers[0] = inputBuffer.data();
        }

        std::array<float*, kMaxAudioChannels> outputPointers = {
            {scratchOutput[0].data(),
             scratchOutput[1].data(),
             scratchOutput[2].data(),
             scratchOutput[3].data()}};

        app->Process({inputPointers.data(), kMaxAudioChannels},
                     {outputPointers.data(), kMaxAudioChannels},
                     segmentFrames);
        app->TickUi((1000.0 * static_cast<double>(segmentFrames))
                    / scenario.renderConfig.sampleRate);

        for(std::size_t frame = 0; frame < segmentFrames; ++frame)
        {
            const auto destinationFrame = currentFrame + frame;
            switch(outputChannelCount)
            {
                case 1:
                    result->audioChannels[0][destinationFrame]
                        = scratchOutput[std::clamp(bindings.mainOutputChannels[0], 0, 3)][frame];
                    break;

                case 2:
                    result->audioChannels[0][destinationFrame]
                        = scratchOutput[std::clamp(bindings.mainOutputChannels[0], 0, 3)][frame];
                    result->audioChannels[1][destinationFrame]
                        = scratchOutput[std::clamp(bindings.mainOutputChannels[1], 0, 3)][frame];
                    break;

                case 4:
                    for(std::size_t channel = 0; channel < 4; ++channel)
                    {
                        result->audioChannels[channel][destinationFrame]
                            = scratchOutput[channel][frame];
                    }
                    break;
            }
        }

        currentFrame += segmentFrames;
    }

    const auto finalParameterSnapshot = app->CaptureStatefulParameterValues();
    result->manifest.finalParameterValues
        = std::map<std::string, float>(finalParameterSnapshot.begin(),
                                       finalParameterSnapshot.end());
    result->manifest.finalEffectiveParameterValues
        = CaptureEffectiveParameterValues(*app);
    result->manifest.finalCvInputs    = currentCvInputs;
    result->manifest.finalGateInputs  = currentGateInputs;
    result->manifest.finalAudioInput  = currentAudioInput;
    result->manifest.audioChecksum    = HashAudioChannels(result->audioChannels);
    result->manifest.channelSummaries.clear();
    result->manifest.channelSummaries.reserve(result->audioChannels.size());
    for(const auto& channel : result->audioChannels)
    {
        result->manifest.channelSummaries.push_back(SummarizeChannel(channel));
    }

    return true;
}

std::string SerializeRenderManifestJson(const RenderResultManifest& manifest)
{
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("appId", juce::String(manifest.appId));
    root->setProperty("boardId", juce::String(manifest.boardId));
    root->setProperty("selectedNodeId", juce::String(manifest.selectedNodeId));
    root->setProperty("entryNodeId", juce::String(manifest.entryNodeId));
    root->setProperty("outputNodeId", juce::String(manifest.outputNodeId));
    root->setProperty("appDisplayName", juce::String(manifest.appDisplayName));

    auto renderConfig = std::make_unique<juce::DynamicObject>();
    renderConfig->setProperty("sampleRate", manifest.renderConfig.sampleRate);
    renderConfig->setProperty("blockSize",
                              static_cast<int>(manifest.renderConfig.blockSize));
    renderConfig->setProperty("durationSeconds",
                              manifest.renderConfig.durationSeconds);
    renderConfig->setProperty("outputChannelCount",
                              manifest.renderConfig.outputChannelCount);
    root->setProperty("renderConfig", juce::var(renderConfig.release()));
    root->setProperty("frameCount", static_cast<juce::int64>(manifest.frameCount));
    root->setProperty("seed", static_cast<juce::int64>(manifest.seed));
    root->setProperty("initialParameterValues",
                      MakeMapVar(manifest.initialParameterValues));
    root->setProperty("finalParameterValues",
                      MakeMapVar(manifest.finalParameterValues));
    root->setProperty("finalEffectiveParameterValues",
                      MakeMapVar(manifest.finalEffectiveParameterValues));
    root->setProperty("finalCvInputs", MakeMapVar(manifest.finalCvInputs));
    root->setProperty("finalGateInputs", MakeBoolMapVar(manifest.finalGateInputs));
    root->setProperty("initialAudioInput",
                      MakeAudioInputVar(manifest.initialAudioInput));
    root->setProperty("finalAudioInput", MakeAudioInputVar(manifest.finalAudioInput));

    juce::Array<juce::var> timelineArray;
    for(const auto& event : manifest.executedTimeline)
    {
        timelineArray.add(MakeTimelineEventVar(event));
    }
    root->setProperty("executedTimeline", juce::var(timelineArray));

    juce::Array<juce::var> summaryArray;
    for(const auto& channelSummary : manifest.channelSummaries)
    {
        auto summaryObject = std::make_unique<juce::DynamicObject>();
        summaryObject->setProperty("peak", channelSummary.peak);
        summaryObject->setProperty("rms", channelSummary.rms);
        summaryArray.add(juce::var(summaryObject.release()));
    }
    root->setProperty("channelSummaries", juce::var(summaryArray));

    juce::Array<juce::var> nodeArray;
    for(const auto& node : manifest.nodes)
    {
        nodeArray.add(MakeNodeSummaryVar(node));
    }
    root->setProperty("nodes", juce::var(nodeArray));

    juce::Array<juce::var> routeArray;
    for(const auto& route : manifest.routes)
    {
        routeArray.add(MakeRouteVar(route));
    }
    root->setProperty("routes", juce::var(routeArray));

    root->setProperty("audioChecksum", juce::String(manifest.audioChecksum));
    root->setProperty("audioPath", juce::String(manifest.audioPath));
    root->setProperty("manifestPath", juce::String(manifest.manifestPath));
    return juce::JSON::toString(juce::var(root.release()), true).toStdString();
}

bool WriteRenderOutputs(RenderResult*                result,
                        const std::filesystem::path& outputDirectory,
                        std::string*                 errorMessage)
{
    if(result == nullptr)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "RenderResult must not be null";
        }
        return false;
    }

    std::error_code ec;
    std::filesystem::create_directories(outputDirectory, ec);
    if(ec)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Failed to create output directory: "
                            + outputDirectory.generic_string();
        }
        return false;
    }

    const auto audioPath    = std::filesystem::absolute(outputDirectory / "audio.wav");
    const auto manifestPath = std::filesystem::absolute(outputDirectory / "manifest.json");

    juce::AudioBuffer<float> audioBuffer(
        static_cast<int>(result->audioChannels.size()),
        result->audioChannels.empty()
            ? 0
            : static_cast<int>(result->audioChannels.front().size()));
    for(std::size_t channel = 0; channel < result->audioChannels.size(); ++channel)
    {
        audioBuffer.copyFrom(static_cast<int>(channel),
                             0,
                             result->audioChannels[channel].data(),
                             static_cast<int>(result->audioChannels[channel].size()));
    }

    juce::File audioFile(juce::String(audioPath.u8string()));
    audioFile.deleteFile();
    std::unique_ptr<juce::FileOutputStream> audioStream(audioFile.createOutputStream());
    if(audioStream == nullptr)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Failed to open audio output stream";
        }
        return false;
    }

    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatWriter> writer(
        wavFormat.createWriterFor(audioStream.get(),
                                  result->manifest.renderConfig.sampleRate,
                                  static_cast<unsigned int>(result->audioChannels.size()),
                                  32,
                                  {},
                                  0));
    if(writer == nullptr)
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Failed to create WAV writer";
        }
        return false;
    }
    audioStream.release();

    if(!writer->writeFromAudioSampleBuffer(audioBuffer, 0, audioBuffer.getNumSamples()))
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Failed to write WAV file";
        }
        return false;
    }

    result->manifest.audioPath    = audioPath.generic_string();
    result->manifest.manifestPath = manifestPath.generic_string();

    juce::File manifestFile(juce::String(manifestPath.u8string()));
    if(!manifestFile.replaceWithText(SerializeRenderManifestJson(result->manifest)))
    {
        if(errorMessage != nullptr)
        {
            *errorMessage = "Failed to write manifest JSON";
        }
        return false;
    }

    return true;
}
} // namespace daisyhost
