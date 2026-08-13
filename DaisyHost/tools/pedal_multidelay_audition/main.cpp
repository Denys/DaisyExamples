#include "daisyhost/PedalDelayEngine.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace
{
using daisyhost::PedalDelayEngine;
using daisyhost::PedalDelayMode;
using daisyhost::PedalFreezeState;
using daisyhost::PedalSlot;

constexpr double      kSampleRate            = 48000.0;
constexpr std::size_t kBlockSize             = 48;
constexpr std::size_t kProfileWarmupBlocks   = 1000;
constexpr std::size_t kProfileMeasuredBlocks = 50000;
constexpr double      kMinimumAudibleRms      = 1.0e-6;
constexpr double      kMinimumTailRms         = 1.0e-6;
constexpr double      kMaximumAllowedPeak     = 20.0;
constexpr double      kPi                     = 3.14159265358979323846;

std::string Trim(std::string value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if(first == std::string::npos)
    {
        return {};
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string ToUpper(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return value;
}

std::vector<std::string> Split(const std::string& value, char delimiter)
{
    std::vector<std::string> fields;
    std::string              field;
    std::istringstream       stream(value);
    while(std::getline(stream, field, delimiter))
    {
        fields.push_back(Trim(field));
    }
    if(!value.empty() && value.back() == delimiter)
    {
        fields.emplace_back();
    }
    return fields;
}

float ParseFloat(const std::string& text, const std::string& fieldName)
{
    std::size_t consumed = 0;
    const float value    = std::stof(text, &consumed);
    if(consumed != text.size() || !std::isfinite(value))
    {
        throw std::runtime_error("Invalid " + fieldName + ": " + text);
    }
    return value;
}

double ParseDouble(const std::string& text, const std::string& fieldName)
{
    std::size_t consumed = 0;
    const double value   = std::stod(text, &consumed);
    if(consumed != text.size() || !std::isfinite(value))
    {
        throw std::runtime_error("Invalid " + fieldName + ": " + text);
    }
    return value;
}

PedalDelayMode ParseMode(const std::string& text)
{
    const std::string value = ToUpper(text);
    if(value == "DIGI")
    {
        return PedalDelayMode::kDigi;
    }
    if(value == "TAPE")
    {
        return PedalDelayMode::kTape;
    }
    if(value == "MOD")
    {
        return PedalDelayMode::kMod;
    }
    if(value == "REV")
    {
        return PedalDelayMode::kRev;
    }
    if(value == "FREEZE")
    {
        return PedalDelayMode::kFreeze;
    }
    throw std::runtime_error("Unknown mode: " + text);
}

PedalSlot ParseSlot(const std::string& text)
{
    const std::string value = ToUpper(text);
    if(value == "TIME")
    {
        return PedalSlot::kTime;
    }
    if(value == "FEEDBACK")
    {
        return PedalSlot::kFeedback;
    }
    if(value == "MIX")
    {
        return PedalSlot::kMix;
    }
    if(value == "COLOR")
    {
        return PedalSlot::kColor;
    }
    if(value == "MOTION")
    {
        return PedalSlot::kMotion;
    }
    throw std::runtime_error("Unknown slot: " + text);
}

PedalFreezeState ParseFreezeState(const std::string& text)
{
    const std::string value = ToUpper(text);
    if(value == "CAPTURE")
    {
        return PedalFreezeState::kCapture;
    }
    if(value == "HOLD")
    {
        return PedalFreezeState::kHold;
    }
    if(value == "ACCUMULATE")
    {
        return PedalFreezeState::kAccumulate;
    }
    if(value == "REPLACE")
    {
        return PedalFreezeState::kReplace;
    }
    if(value == "IDLE")
    {
        return PedalFreezeState::kIdle;
    }
    throw std::runtime_error("Unknown freeze state: " + text);
}

struct Event
{
    enum class Kind
    {
        kSlot,
        kFreezeState,
        kFreezeClear,
    };

    std::size_t      frame = 0;
    Kind             kind  = Kind::kSlot;
    PedalSlot        slot  = PedalSlot::kTime;
    float            value = 0.0f;
    PedalFreezeState freezeState = PedalFreezeState::kIdle;
};

struct Scenario
{
    std::string          name;
    PedalDelayMode       mode = PedalDelayMode::kDigi;
    std::string          cell;
    std::string          source;
    double               durationSeconds = 0.0;
    std::array<float, 5> slots{};
    std::string          eventText;
    std::vector<Event>   events;
    bool                 expectTail = false;
};

std::vector<Event> ParseEvents(const std::string& eventText)
{
    std::vector<Event> events;
    if(Trim(eventText).empty())
    {
        return events;
    }

    for(const std::string& token : Split(eventText, '|'))
    {
        const auto parts = Split(token, ':');
        if(parts.size() < 3)
        {
            throw std::runtime_error("Malformed event: " + token);
        }

        const double timeSeconds = ParseDouble(parts[0], "event time");
        Event        event;
        const double exactFrame = timeSeconds * kSampleRate;
        event.frame = static_cast<std::size_t>(
            std::llround(exactFrame / static_cast<double>(kBlockSize)))
                      * kBlockSize;

        const std::string kind = ToUpper(parts[1]);
        if(kind == "SLOT")
        {
            if(parts.size() != 4)
            {
                throw std::runtime_error("Malformed slot event: " + token);
            }
            event.kind  = Event::Kind::kSlot;
            event.slot  = ParseSlot(parts[2]);
            event.value = std::clamp(ParseFloat(parts[3], "slot value"), 0.0f, 1.0f);
        }
        else if(kind == "FREEZE")
        {
            const std::string operation = ToUpper(parts[2]);
            if(operation == "CLEAR")
            {
                event.kind = Event::Kind::kFreezeClear;
            }
            else
            {
                event.kind        = Event::Kind::kFreezeState;
                event.freezeState = ParseFreezeState(operation);
            }
        }
        else
        {
            throw std::runtime_error("Unknown event kind: " + parts[1]);
        }
        events.push_back(event);
    }

    std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        return a.frame < b.frame;
    });
    return events;
}

std::vector<Scenario> ReadMatrix(const fs::path& path)
{
    std::ifstream input(path);
    if(!input)
    {
        throw std::runtime_error("Unable to open matrix: " + path.string());
    }

    std::vector<Scenario> scenarios;
    std::string           line;
    std::size_t           lineNumber = 0;
    while(std::getline(input, line))
    {
        ++lineNumber;
        const std::string trimmed = Trim(line);
        if(lineNumber == 1 || trimmed.empty() || trimmed.front() == '#')
        {
            continue;
        }

        const auto fields = Split(line, ',');
        if(fields.size() != 12)
        {
            throw std::runtime_error("Matrix line " + std::to_string(lineNumber)
                                     + " has " + std::to_string(fields.size())
                                     + " fields; expected 12");
        }

        Scenario scenario;
        scenario.name            = fields[0];
        scenario.mode            = ParseMode(fields[1]);
        scenario.cell            = fields[2];
        scenario.source          = fields[3];
        scenario.durationSeconds = ParseDouble(fields[4], "duration");
        for(std::size_t slot = 0; slot < scenario.slots.size(); ++slot)
        {
            scenario.slots[slot]
                = std::clamp(ParseFloat(fields[5 + slot], "normalized slot"),
                             0.0f,
                             1.0f);
        }
        scenario.eventText  = fields[10];
        scenario.events     = ParseEvents(scenario.eventText);
        scenario.expectTail = ParseFloat(fields[11], "expect_tail") >= 0.5f;

        if(scenario.name.empty() || scenario.durationSeconds <= 0.0)
        {
            throw std::runtime_error("Invalid scenario at line "
                                     + std::to_string(lineNumber));
        }
        scenarios.push_back(std::move(scenario));
    }

    if(scenarios.size() != 15)
    {
        throw std::runtime_error("Audition matrix must contain exactly 15 cells; got "
                                 + std::to_string(scenarios.size()));
    }
    return scenarios;
}

class SourceGenerator
{
  public:
    SourceGenerator(std::string source, std::uint32_t seed)
    : source_(std::move(source)), state_(seed == 0 ? 1U : seed)
    {
    }

    float Next(std::size_t sampleIndex)
    {
        const double t = static_cast<double>(sampleIndex) / kSampleRate;
        if(source_ == "sine")
        {
            return 0.45f * static_cast<float>(std::sin(2.0 * kPi * 220.0 * t));
        }
        if(source_ == "dual_tone")
        {
            return 0.28f * static_cast<float>(std::sin(2.0 * kPi * 165.0 * t))
                   + 0.14f
                         * static_cast<float>(std::sin(2.0 * kPi * 247.5 * t));
        }
        if(source_ == "transient_train")
        {
            const double local = std::fmod(t, 0.40);
            const float  sign  = (static_cast<std::size_t>(t / 0.40) & 1U) == 0U
                                     ? 1.0f
                                     : -1.0f;
            return sign * 0.9f * static_cast<float>(std::exp(-180.0 * local));
        }
        if(source_ == "single_pluck")
        {
            return t < 0.18 ? Pluck(t) : 0.0f;
        }
        if(source_ == "pluck_train")
        {
            const double local = std::fmod(t, 0.80);
            return local < 0.18 ? Pluck(local) : 0.0f;
        }
        throw std::runtime_error("Unknown source: " + source_);
    }

  private:
    float RandomSigned()
    {
        state_ = state_ * 1664525U + 1013904223U;
        const float unit = static_cast<float>((state_ >> 8U) & 0x00FFFFFFU)
                           / static_cast<float>(0x01000000U);
        return unit * 2.0f - 1.0f;
    }

    float Pluck(double local)
    {
        const float envelope = static_cast<float>(std::exp(-24.0 * local));
        const float harmonic
            = 0.34f * static_cast<float>(std::sin(2.0 * kPi * 110.0 * local))
              + 0.18f
                    * static_cast<float>(std::sin(2.0 * kPi * 220.0 * local));
        return envelope * (0.48f * RandomSigned() + harmonic);
    }

    std::string   source_;
    std::uint32_t state_;
};

struct EngineFixture
{
    PedalDelayEngine   engine;
    std::vector<float> history;
    std::vector<float> freeze;

    EngineFixture()
    {
        const std::size_t historySamples
            = PedalDelayEngine::HistorySamplesForRate(kSampleRate);
        const std::size_t freezeSamples
            = PedalDelayEngine::FreezeSamplesForRate(kSampleRate);
        history.assign(historySamples * daisyhost::kPedalChannelCount, 0.0f);
        freeze.assign(freezeSamples * daisyhost::kPedalChannelCount, 0.0f);
        engine.AttachStorage(
            history.data(), historySamples, freeze.data(), freezeSamples);
        engine.Prepare(kSampleRate, kBlockSize);
    }
};

void ApplyScenarioState(EngineFixture& fixture, const Scenario& scenario)
{
    fixture.engine.SetMode(scenario.mode);
    for(std::size_t slot = 0; slot < scenario.slots.size(); ++slot)
    {
        if(!fixture.engine.SetSlotNormalized(static_cast<PedalSlot>(slot),
                                             scenario.slots[slot]))
        {
            throw std::runtime_error("Failed to set slot for " + scenario.name);
        }
    }
    fixture.engine.Reset();
}

void ApplyEvent(PedalDelayEngine& engine, const Event& event)
{
    switch(event.kind)
    {
        case Event::Kind::kSlot:
            engine.SetSlotNormalized(event.slot, event.value);
            break;
        case Event::Kind::kFreezeState:
            engine.SetFreezeState(event.freezeState);
            break;
        case Event::Kind::kFreezeClear:
            engine.FreezeClear();
            break;
    }
}

void WriteLe16(std::ostream& output, std::uint16_t value)
{
    const char bytes[2] = {static_cast<char>(value & 0xFFU),
                           static_cast<char>((value >> 8U) & 0xFFU)};
    output.write(bytes, 2);
}

void WriteLe32(std::ostream& output, std::uint32_t value)
{
    const char bytes[4] = {static_cast<char>(value & 0xFFU),
                           static_cast<char>((value >> 8U) & 0xFFU),
                           static_cast<char>((value >> 16U) & 0xFFU),
                           static_cast<char>((value >> 24U) & 0xFFU)};
    output.write(bytes, 4);
}

void WriteFloat32Wav(const fs::path& path,
                     const std::vector<float>& interleaved,
                     std::uint32_t             sampleRate,
                     std::uint16_t             channels)
{
    std::ofstream output(path, std::ios::binary);
    if(!output)
    {
        throw std::runtime_error("Unable to create WAV: " + path.string());
    }

    const std::uint32_t dataBytes
        = static_cast<std::uint32_t>(interleaved.size() * sizeof(float));
    output.write("RIFF", 4);
    WriteLe32(output, 36U + dataBytes);
    output.write("WAVE", 4);
    output.write("fmt ", 4);
    WriteLe32(output, 16U);
    WriteLe16(output, 3U);
    WriteLe16(output, channels);
    WriteLe32(output, sampleRate);
    WriteLe32(output, sampleRate * channels * sizeof(float));
    WriteLe16(output, static_cast<std::uint16_t>(channels * sizeof(float)));
    WriteLe16(output, 32U);
    output.write("data", 4);
    WriteLe32(output, dataBytes);

    for(float sample : interleaved)
    {
        std::uint32_t bits = 0;
        static_assert(sizeof(bits) == sizeof(sample), "float32 required");
        std::memcpy(&bits, &sample, sizeof(bits));
        WriteLe32(output, bits);
    }
}

std::string Fnv1a64(const std::vector<float>& samples)
{
    std::uint64_t hash = 1469598103934665603ULL;
    for(float sample : samples)
    {
        std::uint32_t bits = 0;
        std::memcpy(&bits, &sample, sizeof(bits));
        for(unsigned shift = 0; shift < 32; shift += 8)
        {
            hash ^= static_cast<std::uint8_t>((bits >> shift) & 0xFFU);
            hash *= 1099511628211ULL;
        }
    }
    std::ostringstream text;
    text << std::hex << std::setfill('0') << std::setw(16) << hash;
    return text.str();
}

std::string JsonEscape(std::string_view value)
{
    std::ostringstream output;
    for(char character : value)
    {
        switch(character)
        {
            case '\\': output << "\\\\"; break;
            case '"': output << "\\\""; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default: output << character; break;
        }
    }
    return output.str();
}

struct RenderMetrics
{
    double      peak             = 0.0;
    double      rms              = 0.0;
    double      tailRms          = 0.0;
    double      maxStep          = 0.0;
    double      stereoDifference = 0.0;
    std::size_t nonFinite        = 0;
    std::size_t nonZero          = 0;
    std::string checksum;
    bool        passed = false;
};

RenderMetrics MeasureRender(const std::vector<float>& interleaved,
                            std::size_t               tailFrames,
                            bool                      expectTail)
{
    RenderMetrics metrics;
    if(interleaved.empty())
    {
        return metrics;
    }

    double totalSquares = 0.0;
    double tailSquares  = 0.0;
    double stereoDiff   = 0.0;
    const std::size_t frames = interleaved.size() / 2U;
    const std::size_t tailStart = frames > tailFrames ? frames - tailFrames : 0U;
    float previousLeft = 0.0f;

    for(std::size_t frame = 0; frame < frames; ++frame)
    {
        const float left  = interleaved[frame * 2U];
        const float right = interleaved[frame * 2U + 1U];
        if(!std::isfinite(left) || !std::isfinite(right))
        {
            ++metrics.nonFinite;
            continue;
        }
        metrics.peak = std::max(
            metrics.peak,
            static_cast<double>(std::max(std::abs(left), std::abs(right))));
        totalSquares += static_cast<double>(left) * left
                        + static_cast<double>(right) * right;
        if(frame >= tailStart)
        {
            tailSquares += static_cast<double>(left) * left
                           + static_cast<double>(right) * right;
        }
        if(frame > 0)
        {
            metrics.maxStep
                = std::max(metrics.maxStep,
                           static_cast<double>(std::abs(left - previousLeft)));
        }
        previousLeft = left;
        stereoDiff += static_cast<double>(left - right) * (left - right);
        if(std::abs(left) > 1.0e-9f || std::abs(right) > 1.0e-9f)
        {
            ++metrics.nonZero;
        }
    }

    const double sampleCount = static_cast<double>(frames * 2U);
    const double tailSampleCount
        = static_cast<double>((frames - tailStart) * 2U);
    metrics.rms = sampleCount > 0.0 ? std::sqrt(totalSquares / sampleCount) : 0.0;
    metrics.tailRms
        = tailSampleCount > 0.0 ? std::sqrt(tailSquares / tailSampleCount) : 0.0;
    metrics.stereoDifference
        = frames > 0 ? std::sqrt(stereoDiff / static_cast<double>(frames)) : 0.0;
    metrics.checksum = Fnv1a64(interleaved);
    metrics.passed = metrics.nonFinite == 0 && metrics.nonZero > 0
                     && metrics.rms > kMinimumAudibleRms
                     && metrics.peak < kMaximumAllowedPeak
                     && (!expectTail || metrics.tailRms > kMinimumTailRms);
    return metrics;
}

struct RenderResult
{
    Scenario             scenario;
    std::array<float, 5> nativeSlots{};
    RenderMetrics        metrics;
    PedalFreezeState     finalFreezeState = PedalFreezeState::kIdle;
    fs::path             wavPath;
    fs::path             manifestPath;
};

RenderResult RenderScenario(const Scenario& scenario,
                            const fs::path&  outputDirectory,
                            std::uint32_t    seed)
{
    EngineFixture fixture;
    ApplyScenarioState(fixture, scenario);
    std::array<float, 5> initialNativeSlots{};
    for(std::size_t slot = 0; slot < initialNativeSlots.size(); ++slot)
    {
        initialNativeSlots[slot]
            = fixture.engine.GetSlotNative(static_cast<PedalSlot>(slot));
    }

    const std::size_t frames = static_cast<std::size_t>(
        std::llround(scenario.durationSeconds * kSampleRate));
    std::vector<float> interleaved(frames * 2U, 0.0f);
    std::array<float, kBlockSize> inputLeft{};
    std::array<float, kBlockSize> inputRight{};
    std::array<float, kBlockSize> outputLeft{};
    std::array<float, kBlockSize> outputRight{};
    SourceGenerator               source(scenario.source, seed);
    std::size_t                   eventIndex = 0;

    for(std::size_t offset = 0; offset < frames; offset += kBlockSize)
    {
        while(eventIndex < scenario.events.size()
              && scenario.events[eventIndex].frame <= offset)
        {
            ApplyEvent(fixture.engine, scenario.events[eventIndex]);
            ++eventIndex;
        }

        const std::size_t blockFrames = std::min(kBlockSize, frames - offset);
        for(std::size_t frame = 0; frame < blockFrames; ++frame)
        {
            inputLeft[frame]  = source.Next(offset + frame);
            inputRight[frame] = inputLeft[frame];
        }
        fixture.engine.Process(inputLeft.data(),
                               inputRight.data(),
                               outputLeft.data(),
                               outputRight.data(),
                               blockFrames);
        for(std::size_t frame = 0; frame < blockFrames; ++frame)
        {
            interleaved[(offset + frame) * 2U]      = outputLeft[frame];
            interleaved[(offset + frame) * 2U + 1U] = outputRight[frame];
        }
    }

    RenderResult result;
    result.scenario         = scenario;
    result.finalFreezeState = fixture.engine.GetFreezeState();
    result.nativeSlots      = initialNativeSlots;
    result.metrics = MeasureRender(
        interleaved, static_cast<std::size_t>(0.5 * kSampleRate), scenario.expectTail);
    result.wavPath      = outputDirectory / (scenario.name + ".wav");
    result.manifestPath = outputDirectory / (scenario.name + ".json");
    WriteFloat32Wav(result.wavPath,
                    interleaved,
                    static_cast<std::uint32_t>(kSampleRate),
                    2U);

    std::ofstream manifest(result.manifestPath);
    if(!manifest)
    {
        throw std::runtime_error("Unable to create manifest: "
                                 + result.manifestPath.string());
    }
    manifest << std::setprecision(12);
    manifest << "{\n"
             << "  \"scenario\": \"" << JsonEscape(scenario.name) << "\",\n"
             << "  \"mode\": \"" << daisyhost::PedalDelayModeName(scenario.mode)
             << "\",\n"
             << "  \"cell\": \"" << JsonEscape(scenario.cell) << "\",\n"
             << "  \"source\": \"" << JsonEscape(scenario.source) << "\",\n"
             << "  \"sample_rate_hz\": " << kSampleRate << ",\n"
             << "  \"block_frames\": " << kBlockSize << ",\n"
             << "  \"duration_seconds\": " << scenario.durationSeconds << ",\n"
             << "  \"slot_normalized\": [";
    for(std::size_t slot = 0; slot < 5; ++slot)
    {
        manifest << (slot == 0 ? "" : ", ") << scenario.slots[slot];
    }
    manifest << "],\n  \"slot_native\": [";
    for(std::size_t slot = 0; slot < 5; ++slot)
    {
        manifest << (slot == 0 ? "" : ", ") << result.nativeSlots[slot];
    }
    manifest << "],\n"
             << "  \"events\": \"" << JsonEscape(scenario.eventText) << "\",\n"
             << "  \"expect_tail\": " << (scenario.expectTail ? "true" : "false")
             << ",\n"
             << "  \"final_freeze_state\": \""
             << daisyhost::PedalFreezeStateName(result.finalFreezeState) << "\",\n"
             << "  \"metrics\": {\n"
             << "    \"peak\": " << result.metrics.peak << ",\n"
             << "    \"rms\": " << result.metrics.rms << ",\n"
             << "    \"tail_rms\": " << result.metrics.tailRms << ",\n"
             << "    \"max_step\": " << result.metrics.maxStep << ",\n"
             << "    \"stereo_difference_rms\": "
             << result.metrics.stereoDifference << ",\n"
             << "    \"non_finite_samples\": " << result.metrics.nonFinite << ",\n"
             << "    \"non_zero_frames\": " << result.metrics.nonZero << ",\n"
             << "    \"checksum_fnv1a64\": \"" << result.metrics.checksum
             << "\"\n"
             << "  },\n"
             << "  \"status\": \""
             << (result.metrics.passed ? "PASS" : "FAIL") << "\"\n"
             << "}\n";
    return result;
}

double Percentile(const std::vector<double>& values, double quantile)
{
    if(values.empty())
    {
        return 0.0;
    }
    std::vector<double> scratch(values);
    const std::size_t rank = static_cast<std::size_t>(
        std::ceil(quantile * static_cast<double>(scratch.size())));
    const std::size_t index = std::min(
        scratch.size() - 1U, rank == 0U ? 0U : rank - 1U);
    std::nth_element(
        scratch.begin(), scratch.begin() + static_cast<std::ptrdiff_t>(index), scratch.end());
    return scratch[index];
}

double MeasureClockOverheadUs()
{
    using Clock = std::chrono::steady_clock;
    double minimum = std::numeric_limits<double>::infinity();
    for(std::size_t iteration = 0; iteration < 10000; ++iteration)
    {
        const auto start = Clock::now();
        const auto stop  = Clock::now();
        const double duration
            = std::chrono::duration<double, std::micro>(stop - start).count();
        minimum = std::min(minimum, duration);
    }
    return std::isfinite(minimum) ? minimum : 0.0;
}

struct TimingSummary
{
    std::string mode;
    std::size_t warmupBlocks        = 0;
    std::size_t measuredBlocks      = 0;
    double      measuredDurationS   = 0.0;
    double      blockBudgetUs       = 0.0;
    double      meanUs              = 0.0;
    double      p99Us               = 0.0;
    double      p999Us              = 0.0;
    double      maxUs               = 0.0;
    double      marginP999Us        = 0.0;
    double      tailUtilization     = 0.0;
    std::size_t deadlineMisses      = 0;
    double      deadlineMissRate    = 0.0;
    double      maxLatenessUs       = 0.0;
    std::size_t processingFailures  = 0;
    double      timerOverheadUs     = 0.0;
    std::string status;
};

void ConfigureProfileMode(EngineFixture& fixture, PedalDelayMode mode)
{
    fixture.engine.SetMode(mode);
    std::array<float, 5> values = {0.75f, 0.95f, 0.85f, 0.90f, 0.90f};
    if(mode == PedalDelayMode::kMod)
    {
        values = {0.10f, 1.0f, 1.0f, 1.0f, 1.0f};
    }
    else if(mode == PedalDelayMode::kRev)
    {
        values = {0.12f, 1.0f, 1.0f, 1.0f, 0.10f};
    }
    else if(mode == PedalDelayMode::kFreeze)
    {
        values = {0.18f, 1.0f, 0.90f, 0.85f, 1.0f};
    }
    for(std::size_t slot = 0; slot < values.size(); ++slot)
    {
        fixture.engine.SetSlotNormalized(static_cast<PedalSlot>(slot), values[slot]);
    }
    fixture.engine.Reset();
}

bool BlockIsFinite(const std::array<float, kBlockSize>& left,
                   const std::array<float, kBlockSize>& right)
{
    for(std::size_t frame = 0; frame < kBlockSize; ++frame)
    {
        if(!std::isfinite(left[frame]) || !std::isfinite(right[frame]))
        {
            return false;
        }
    }
    return true;
}

TimingSummary ProfileMode(PedalDelayMode mode, double timerOverheadUs)
{
    using Clock = std::chrono::steady_clock;
    EngineFixture fixture;
    ConfigureProfileMode(fixture, mode);

    std::array<float, kBlockSize> inputLeft{};
    std::array<float, kBlockSize> inputRight{};
    std::array<float, kBlockSize> outputLeft{};
    std::array<float, kBlockSize> outputRight{};
    for(std::size_t frame = 0; frame < kBlockSize; ++frame)
    {
        inputLeft[frame]
            = 0.35f * static_cast<float>(std::sin(2.0 * kPi * 317.0
                                                  * static_cast<double>(frame)
                                                  / kSampleRate));
        inputRight[frame] = inputLeft[frame];
    }

    if(mode == PedalDelayMode::kFreeze)
    {
        fixture.engine.SetFreezeState(PedalFreezeState::kCapture);
        const std::size_t captureBlocks
            = static_cast<std::size_t>(kSampleRate / kBlockSize);
        for(std::size_t block = 0; block < captureBlocks; ++block)
        {
            fixture.engine.Process(inputLeft.data(),
                                   inputRight.data(),
                                   outputLeft.data(),
                                   outputRight.data(),
                                   kBlockSize);
        }
        fixture.engine.SetFreezeState(PedalFreezeState::kAccumulate);
    }

    for(std::size_t block = 0; block < kProfileWarmupBlocks; ++block)
    {
        if((mode == PedalDelayMode::kDigi || mode == PedalDelayMode::kTape)
           && block % 64U == 0U)
        {
            const float value = (block / 64U) % 2U == 0U ? 0.12f : 0.88f;
            fixture.engine.SetSlotNormalized(PedalSlot::kTime, value);
        }
        fixture.engine.Process(inputLeft.data(),
                               inputRight.data(),
                               outputLeft.data(),
                               outputRight.data(),
                               kBlockSize);
    }

    std::vector<double> durations(kProfileMeasuredBlocks, 0.0);
    std::size_t         processingFailures = 0;
    for(std::size_t block = 0; block < kProfileMeasuredBlocks; ++block)
    {
        if((mode == PedalDelayMode::kDigi || mode == PedalDelayMode::kTape)
           && block % 64U == 0U)
        {
            const float value = (block / 64U) % 2U == 0U ? 0.12f : 0.88f;
            fixture.engine.SetSlotNormalized(PedalSlot::kTime, value);
        }
        const auto start = Clock::now();
        fixture.engine.Process(inputLeft.data(),
                               inputRight.data(),
                               outputLeft.data(),
                               outputRight.data(),
                               kBlockSize);
        const auto stop = Clock::now();
        durations[block]
            = std::chrono::duration<double, std::micro>(stop - start).count();
        if(!std::isfinite(durations[block])
           || !BlockIsFinite(outputLeft, outputRight))
        {
            ++processingFailures;
        }
    }

    TimingSummary summary;
    summary.mode              = daisyhost::PedalDelayModeName(mode);
    summary.warmupBlocks      = kProfileWarmupBlocks;
    summary.measuredBlocks    = kProfileMeasuredBlocks;
    summary.measuredDurationS = static_cast<double>(kProfileMeasuredBlocks * kBlockSize)
                                / kSampleRate;
    summary.blockBudgetUs = 1000000.0 * static_cast<double>(kBlockSize) / kSampleRate;
    summary.meanUs = std::accumulate(durations.begin(), durations.end(), 0.0)
                     / static_cast<double>(durations.size());
    summary.p99Us  = Percentile(durations, 0.99);
    summary.p999Us = Percentile(durations, 0.999);
    summary.maxUs  = *std::max_element(durations.begin(), durations.end());
    summary.marginP999Us    = summary.blockBudgetUs - summary.p999Us;
    summary.tailUtilization = summary.p999Us / summary.blockBudgetUs;
    summary.deadlineMisses = static_cast<std::size_t>(std::count_if(
        durations.begin(), durations.end(), [&](double duration) {
            return duration > summary.blockBudgetUs;
        }));
    summary.deadlineMissRate
        = static_cast<double>(summary.deadlineMisses)
          / static_cast<double>(summary.measuredBlocks);
    summary.maxLatenessUs
        = std::max(0.0, summary.maxUs - summary.blockBudgetUs);
    summary.processingFailures = processingFailures;
    summary.timerOverheadUs     = timerOverheadUs;
    if(!Clock::is_steady || processingFailures != 0U)
    {
        summary.status = "HOST_MEASUREMENT_INVALID";
    }
    else if(summary.measuredBlocks < kProfileMeasuredBlocks)
    {
        summary.status = "INSUFFICIENT_SAMPLE_COUNT";
    }
    else if(summary.p999Us <= summary.blockBudgetUs
            && summary.deadlineMisses == 0U)
    {
        summary.status = "HOST_OBSERVED_WITHIN_BUDGET";
    }
    else
    {
        summary.status = "HOST_OBSERVED_TAIL_VIOLATION";
    }
    return summary;
}

std::string CompilerIdentity()
{
#if defined(__clang__)
    return std::string("Clang ") + __clang_version__;
#elif defined(__GNUC__)
    return std::string("GCC ") + __VERSION__;
#elif defined(_MSC_VER)
    return std::string("MSVC ") + std::to_string(_MSC_VER);
#else
    return "unknown";
#endif
}

std::string OsIdentity()
{
#if defined(_WIN32)
    return "Windows";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(__linux__)
    return "Linux";
#else
    return "unknown";
#endif
}

void WriteTimingReports(const fs::path& outputDirectory,
                        const std::vector<TimingSummary>& summaries)
{
    std::ofstream csv(outputDirectory / "host_profile.csv");
    csv << "mode,warmup_blocks,measured_blocks,measured_duration_s,block_budget_us,"
           "mean_us,p99_us,p99_9_us,max_us,margin_p99_9_us,tail_utilization_ratio,"
           "deadline_miss_count,deadline_miss_rate,max_lateness_us,"
           "processing_failure_count,timer_overhead_us,status\n";
    csv << std::setprecision(12);
    for(const auto& summary : summaries)
    {
        csv << summary.mode << ',' << summary.warmupBlocks << ','
            << summary.measuredBlocks << ',' << summary.measuredDurationS << ','
            << summary.blockBudgetUs << ',' << summary.meanUs << ','
            << summary.p99Us << ',' << summary.p999Us << ',' << summary.maxUs
            << ',' << summary.marginP999Us << ',' << summary.tailUtilization
            << ',' << summary.deadlineMisses << ',' << summary.deadlineMissRate
            << ',' << summary.maxLatenessUs << ',' << summary.processingFailures
            << ',' << summary.timerOverheadUs << ',' << summary.status << '\n';
    }

    std::ofstream environment(outputDirectory / "host_profile_environment.json");
    environment << "{\n"
                << "  \"clock\": \"std::chrono::steady_clock\",\n"
                << "  \"clock_is_steady\": "
                << (std::chrono::steady_clock::is_steady ? "true" : "false")
                << ",\n"
                << "  \"clock_period_num\": "
                << std::chrono::steady_clock::period::num << ",\n"
                << "  \"clock_period_den\": "
                << std::chrono::steady_clock::period::den << ",\n"
                << "  \"compiler\": \"" << JsonEscape(CompilerIdentity())
                << "\",\n"
                << "  \"os\": \"" << JsonEscape(OsIdentity()) << "\",\n"
                << "  \"sample_rate_hz\": " << kSampleRate << ",\n"
                << "  \"block_frames_per_channel\": " << kBlockSize << ",\n"
                << "  \"warmup_blocks\": " << kProfileWarmupBlocks << ",\n"
                << "  \"measured_blocks\": " << kProfileMeasuredBlocks << ",\n"
                << "  \"sensitivity_model_enabled\": false,\n"
                << "  \"target_timing_claim\": \"NOT_RUN\"\n"
                << "}\n";
}

void WriteAggregateManifest(const fs::path& outputDirectory,
                            const std::vector<RenderResult>& results)
{
    std::ofstream csv(outputDirectory / "audition_manifest.csv");
    csv << "scenario,mode,cell,source,duration_s,time_norm,feedback_norm,mix_norm,"
           "color_norm,motion_norm,time_native,feedback_native,mix_native,color_native,"
           "motion_native,wav,peak,rms,tail_rms,max_step,stereo_difference_rms,"
           "non_finite_samples,checksum_fnv1a64,final_freeze_state,status\n";
    csv << std::setprecision(12);
    for(const auto& result : results)
    {
        csv << result.scenario.name << ','
            << daisyhost::PedalDelayModeName(result.scenario.mode) << ','
            << result.scenario.cell << ',' << result.scenario.source << ','
            << result.scenario.durationSeconds;
        for(float value : result.scenario.slots)
        {
            csv << ',' << value;
        }
        for(float value : result.nativeSlots)
        {
            csv << ',' << value;
        }
        csv << ',' << result.wavPath.filename().string() << ','
            << result.metrics.peak << ',' << result.metrics.rms << ','
            << result.metrics.tailRms << ',' << result.metrics.maxStep << ','
            << result.metrics.stereoDifference << ',' << result.metrics.nonFinite
            << ',' << result.metrics.checksum << ','
            << daisyhost::PedalFreezeStateName(result.finalFreezeState) << ','
            << (result.metrics.passed ? "PASS" : "FAIL") << '\n';
    }
}

struct Arguments
{
    fs::path matrix;
    fs::path outputDirectory;
};

Arguments ParseArguments(int argc, char** argv)
{
    Arguments arguments;
    for(int index = 1; index < argc; ++index)
    {
        const std::string option = argv[index];
        if(option == "--matrix" && index + 1 < argc)
        {
            arguments.matrix = argv[++index];
        }
        else if(option == "--output-dir" && index + 1 < argc)
        {
            arguments.outputDirectory = argv[++index];
        }
        else
        {
            throw std::runtime_error("Unknown or incomplete option: " + option);
        }
    }
    if(arguments.matrix.empty() || arguments.outputDirectory.empty())
    {
        throw std::runtime_error(
            "Usage: pedal_multidelay_audition --matrix <csv> --output-dir <dir>");
    }
    return arguments;
}
} // namespace

int main(int argc, char** argv)
{
    try
    {
        const Arguments arguments = ParseArguments(argc, argv);
        fs::create_directories(arguments.outputDirectory);
        const auto scenarios = ReadMatrix(arguments.matrix);

        std::vector<RenderResult> results;
        results.reserve(scenarios.size());
        bool allRendersPassed = true;
        for(std::size_t index = 0; index < scenarios.size(); ++index)
        {
            auto result = RenderScenario(scenarios[index],
                                         arguments.outputDirectory,
                                         static_cast<std::uint32_t>(17U + index));
            std::cout << result.scenario.name << ' '
                      << (result.metrics.passed ? "PASS" : "FAIL")
                      << " peak=" << result.metrics.peak
                      << " rms=" << result.metrics.rms
                      << " tail_rms=" << result.metrics.tailRms
                      << " checksum=" << result.metrics.checksum << '\n';
            allRendersPassed = allRendersPassed && result.metrics.passed;
            results.push_back(std::move(result));
        }
        WriteAggregateManifest(arguments.outputDirectory, results);

        const double timerOverheadUs = MeasureClockOverheadUs();
        std::vector<TimingSummary> timing;
        for(PedalDelayMode mode : {PedalDelayMode::kDigi,
                                   PedalDelayMode::kTape,
                                   PedalDelayMode::kMod,
                                   PedalDelayMode::kRev,
                                   PedalDelayMode::kFreeze})
        {
            timing.push_back(ProfileMode(mode, timerOverheadUs));
            const auto& summary = timing.back();
            std::cout << "PROFILE " << summary.mode
                      << " mean_us=" << summary.meanUs
                      << " p99_9_us=" << summary.p999Us
                      << " max_us=" << summary.maxUs
                      << " misses=" << summary.deadlineMisses
                      << " status=" << summary.status << '\n';
        }
        WriteTimingReports(arguments.outputDirectory, timing);

        const bool profileValid = std::all_of(
            timing.begin(), timing.end(), [](const TimingSummary& summary) {
                return summary.status != "HOST_MEASUREMENT_INVALID"
                       && summary.status != "INSUFFICIENT_SAMPLE_COUNT"
                       && summary.processingFailures == 0U;
            });

        if(!allRendersPassed || !profileValid)
        {
            std::cerr << "Audition/profiling gate failed\n";
            return 2;
        }
        std::cout << "HOST_AUDITION_PACKAGE_READY\n";
        std::cout << "HOST_LISTENING_PENDING\n";
        std::cout << "TARGET_TIMING_NOT_RUN\n";
        return 0;
    }
    catch(const std::exception& error)
    {
        std::cerr << "ERROR: " << error.what() << '\n';
        return 1;
    }
}
