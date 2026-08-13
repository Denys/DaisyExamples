#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

#include "daisyhost/AppRegistry.h"
#include "daisyhost/PedalDelayEngine.h"
#include "daisyhost/apps/PedalDelayCore.h"

namespace
{
using daisyhost::GetPedalSlotDescriptor;
using daisyhost::kPedalChannelCount;
using daisyhost::kPedalDelayModeCount;
using daisyhost::kPedalSlotCount;
using daisyhost::PedalDelayEngine;
using daisyhost::PedalDelayMode;
using daisyhost::PedalFreezeState;
using daisyhost::PedalSlot;
using daisyhost::apps::PedalDelayCore;

constexpr double      kSampleRate = 48000.0;
constexpr std::size_t kBlockSize  = 64;

PedalDelayMode ModeAt(std::size_t index)
{
    return static_cast<PedalDelayMode>(index);
}

PedalSlot SlotAt(std::size_t index)
{
    return static_cast<PedalSlot>(index);
}

struct EngineFixture
{
    PedalDelayEngine   engine;
    std::vector<float> history;
    std::vector<float> freeze;

    explicit EngineFixture(double sampleRate = kSampleRate)
    {
        const std::size_t historySamples
            = PedalDelayEngine::HistorySamplesForRate(sampleRate);
        const std::size_t freezeSamples
            = PedalDelayEngine::FreezeSamplesForRate(sampleRate);
        history.assign(historySamples * kPedalChannelCount, 0.0f);
        freeze.assign(freezeSamples * kPedalChannelCount, 0.0f);
        engine.AttachStorage(
            history.data(), historySamples, freeze.data(), freezeSamples);
        engine.Prepare(sampleRate, kBlockSize);
    }

    void SetAllSlotsToDefault()
    {
        for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
        {
            const auto& descriptor
                = GetPedalSlotDescriptor(engine.GetMode(), SlotAt(slot));
            engine.SetSlotNormalized(
                SlotAt(slot),
                engine.NativeToNormalized(
                    engine.GetMode(), SlotAt(slot), descriptor.defaultValue));
        }
    }

    void SetSlotNative(PedalSlot slot, float nativeValue)
    {
        engine.SetSlotNormalized(
            slot,
            engine.NativeToNormalized(engine.GetMode(), slot, nativeValue));
    }

    // Processes `input` as the left/right channel and returns the left output.
    std::vector<float> Run(const std::vector<float>& input)
    {
        std::vector<float> outputLeft(input.size(), 0.0f);
        std::vector<float> outputRight(input.size(), 0.0f);
        for(std::size_t offset = 0; offset < input.size(); offset += kBlockSize)
        {
            const std::size_t frames
                = std::min(kBlockSize, input.size() - offset);
            engine.Process(input.data() + offset,
                           input.data() + offset,
                           outputLeft.data() + offset,
                           outputRight.data() + offset,
                           frames);
        }
        return outputLeft;
    }

    void RunSilence(std::size_t frameCount)
    {
        const std::vector<float> silence(frameCount, 0.0f);
        Run(silence);
    }
};

std::vector<float> Silence(std::size_t frameCount)
{
    return std::vector<float>(frameCount, 0.0f);
}

std::vector<float>
Sine(std::size_t frameCount, float frequencyHz, float amplitude = 0.5f)
{
    std::vector<float> buffer(frameCount, 0.0f);
    for(std::size_t i = 0; i < frameCount; ++i)
    {
        buffer[i]
            = amplitude
              * std::sin(6.28318530717959f * frequencyHz * static_cast<float>(i)
                         / static_cast<float>(kSampleRate));
    }
    return buffer;
}

std::vector<float> Impulse(std::size_t frameCount)
{
    std::vector<float> buffer(frameCount, 0.0f);
    buffer[0] = 1.0f;
    return buffer;
}

bool AllFinite(const std::vector<float>& buffer)
{
    return std::all_of(buffer.begin(), buffer.end(), [](float value) {
        return std::isfinite(value);
    });
}

float Peak(const std::vector<float>& buffer)
{
    float peak = 0.0f;
    for(float value : buffer)
    {
        peak = std::max(peak, std::abs(value));
    }
    return peak;
}

float Rms(const std::vector<float>& buffer, std::size_t begin = 0)
{
    if(begin >= buffer.size())
    {
        return 0.0f;
    }
    double sum = 0.0;
    for(std::size_t i = begin; i < buffer.size(); ++i)
    {
        sum += static_cast<double>(buffer[i]) * buffer[i];
    }
    return static_cast<float>(
        std::sqrt(sum / static_cast<double>(buffer.size() - begin)));
}

// Normalized first-difference energy: a proxy for high-frequency content.
float HighFrequencyRatio(const std::vector<float>& buffer,
                         std::size_t               begin = 1)
{
    double difference = 0.0;
    double magnitude  = 0.0;
    for(std::size_t i = std::max<std::size_t>(begin, 1); i < buffer.size(); ++i)
    {
        difference += std::abs(buffer[i] - buffer[i - 1]);
        magnitude += std::abs(buffer[i]);
    }
    return magnitude > 1e-9 ? static_cast<float>(difference / magnitude) : 0.0f;
}

float MaxStep(const std::vector<float>& buffer, std::size_t begin = 1)
{
    float maximum = 0.0f;
    for(std::size_t i = std::max<std::size_t>(begin, 1); i < buffer.size(); ++i)
    {
        maximum = std::max(maximum, std::abs(buffer[i] - buffer[i - 1]));
    }
    return maximum;
}

// Largest jump in the short-window RMS envelope. A click shows up here as a
// broadband spike; a slewed (pitch-warped) read head does not, because it only
// changes the instantaneous frequency, never the level.
float EnvelopeMaxStep(const std::vector<float>& buffer,
                      std::size_t               window = 128,
                      std::size_t               begin  = 0)
{
    float previous = -1.0f;
    float maximum  = 0.0f;
    for(std::size_t start = begin; start + window <= buffer.size();
        start += window)
    {
        double sum = 0.0;
        for(std::size_t i = start; i < start + window; ++i)
        {
            sum += static_cast<double>(buffer[i]) * buffer[i];
        }
        const float rms
            = static_cast<float>(std::sqrt(sum / static_cast<double>(window)));
        if(previous >= 0.0f)
        {
            maximum = std::max(maximum, std::abs(rms - previous));
        }
        previous = rms;
    }
    return maximum;
}

float MaxDifference(const std::vector<float>& a, const std::vector<float>& b)
{
    const std::size_t count   = std::min(a.size(), b.size());
    float             maximum = 0.0f;
    for(std::size_t i = 0; i < count; ++i)
    {
        maximum = std::max(maximum, std::abs(a[i] - b[i]));
    }
    return maximum;
}

// Runs the given mode with one slot forced to `normalizedValue` and returns
// the wet-heavy output, so two runs can be compared for real DSP effect.
std::vector<float>
RunWithSlot(PedalDelayMode mode, PedalSlot slot, float normalizedValue)
{
    EngineFixture fixture;
    fixture.engine.SetMode(mode);
    fixture.SetAllSlotsToDefault();
    if(slot != PedalSlot::kMix)
    {
        fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
    }
    fixture.engine.SetSlotNormalized(slot, normalizedValue);

    std::vector<float> input
        = Sine(static_cast<std::size_t>(kSampleRate / 2), 220.0f);
    input.resize(static_cast<std::size_t>(kSampleRate), 0.0f);
    return fixture.Run(input);
}

std::unique_ptr<PedalDelayCore> MakeCore()
{
    auto core = std::make_unique<PedalDelayCore>("node0");
    core->Prepare(kSampleRate, kBlockSize);
    core->ResetToDefaultState(0);
    return core;
}
} // namespace

// ---------------------------------------------------------------------------
// Descriptor / control contract
// ---------------------------------------------------------------------------

TEST(PedalDelayDescriptorTest, EveryModeDeclaresFiveCompleteSlots)
{
    for(std::size_t mode = 0; mode < kPedalDelayModeCount; ++mode)
    {
        for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
        {
            const auto& descriptor
                = GetPedalSlotDescriptor(ModeAt(mode), SlotAt(slot));
            const std::string context
                = std::string(daisyhost::PedalDelayModeName(ModeAt(mode))) + "/"
                  + descriptor.slotId;

            EXPECT_FALSE(std::string(descriptor.slotId).empty()) << context;
            EXPECT_FALSE(std::string(descriptor.musicianLabel).empty())
                << context;
            EXPECT_FALSE(std::string(descriptor.engineeringLabel).empty())
                << context;
            EXPECT_FALSE(std::string(descriptor.unit).empty()) << context;
            EXPECT_FALSE(std::string(descriptor.clockwiseMeaning).empty())
                << context;
            EXPECT_FALSE(std::string(descriptor.audibleEffect).empty())
                << context;

            EXPECT_TRUE(std::isfinite(descriptor.minimum)) << context;
            EXPECT_TRUE(std::isfinite(descriptor.maximum)) << context;
            EXPECT_TRUE(std::isfinite(descriptor.defaultValue)) << context;
            EXPECT_LT(descriptor.minimum, descriptor.maximum) << context;
            EXPECT_GE(descriptor.defaultValue, descriptor.minimum) << context;
            EXPECT_LE(descriptor.defaultValue, descriptor.maximum) << context;
            EXPECT_GE(descriptor.smoothingMs, 0.0f) << context;

            const std::string curve = descriptor.curve;
            EXPECT_TRUE(curve == "linear" || curve == "log"
                        || curve == "bipolar" || curve == "custom")
                << context << " curve=" << curve;

            std::size_t targetCount = 0;
            for(const char* target : descriptor.dspTargets)
            {
                if(target != nullptr && target[0] != '\0')
                {
                    ++targetCount;
                }
            }
            EXPECT_GT(targetCount, 0u) << context;
            if(descriptor.isMacro)
            {
                EXPECT_GT(targetCount, 1u)
                    << context << " macro must enumerate every target";
            }
        }
    }
}

TEST(PedalDelayDescriptorTest, SlotOrderMatchesTheProductContract)
{
    static const std::array<const char*, kPedalSlotCount> kExpected
        = {{"TIME", "FEEDBACK", "MIX", "COLOR", "MOTION"}};
    for(std::size_t mode = 0; mode < kPedalDelayModeCount; ++mode)
    {
        for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
        {
            EXPECT_STREQ(
                kExpected[slot],
                GetPedalSlotDescriptor(ModeAt(mode), SlotAt(slot)).slotId);
        }
    }
}

TEST(PedalDelayDescriptorTest, NormalizedEndpointsMapToValidEngineeringValues)
{
    EngineFixture fixture;
    for(std::size_t mode = 0; mode < kPedalDelayModeCount; ++mode)
    {
        for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
        {
            const auto& descriptor
                = GetPedalSlotDescriptor(ModeAt(mode), SlotAt(slot));
            for(float normalized : {0.0f, 0.5f, 1.0f})
            {
                const float native = fixture.engine.NormalizedToNative(
                    ModeAt(mode), SlotAt(slot), normalized);
                EXPECT_TRUE(std::isfinite(native));
                EXPECT_GE(native, descriptor.minimum - 1e-3f);
                EXPECT_LE(native, descriptor.maximum + 1e-3f);
                const float roundTrip = fixture.engine.NativeToNormalized(
                    ModeAt(mode), SlotAt(slot), native);
                EXPECT_NEAR(normalized, roundTrip, 1e-3f);
            }
        }
    }
}

TEST(PedalDelayDescriptorTest, RejectsNonFiniteAndClampsOutOfRangeInput)
{
    EngineFixture fixture;
    fixture.engine.SetSlotNormalized(PedalSlot::kFeedback, 0.4f);

    EXPECT_FALSE(fixture.engine.SetSlotNormalized(
        PedalSlot::kFeedback, std::numeric_limits<float>::quiet_NaN()));
    EXPECT_FALSE(fixture.engine.SetSlotNormalized(
        PedalSlot::kFeedback, std::numeric_limits<float>::infinity()));
    EXPECT_FLOAT_EQ(0.4f,
                    fixture.engine.GetSlotNormalized(PedalSlot::kFeedback));

    // Documented policy for finite out-of-range input: clamp, never reject.
    EXPECT_TRUE(fixture.engine.SetSlotNormalized(PedalSlot::kFeedback, 12.0f));
    EXPECT_FLOAT_EQ(1.0f,
                    fixture.engine.GetSlotNormalized(PedalSlot::kFeedback));
    EXPECT_TRUE(fixture.engine.SetSlotNormalized(PedalSlot::kFeedback, -3.0f));
    EXPECT_FLOAT_EQ(0.0f,
                    fixture.engine.GetSlotNormalized(PedalSlot::kFeedback));
}

// ---------------------------------------------------------------------------
// Host app surface
// ---------------------------------------------------------------------------

TEST(PedalDelayCoreTest, IsRegisteredAndSelectableByAppId)
{
    std::string resolved;
    auto        app = daisyhost::CreateHostedAppCore(
        "pedal_multidelay", "node0", &resolved);
    ASSERT_NE(nullptr, app);
    EXPECT_EQ("pedal_multidelay", resolved);
    EXPECT_EQ("pedal_multidelay", app->GetAppId());
    EXPECT_TRUE(app->GetCapabilities().acceptsAudioInput);
}

TEST(PedalDelayCoreTest, ExposesExactlyFivePerformanceSlotsPerMode)
{
    auto core = MakeCore();
    for(std::size_t mode = 0; mode < kPedalDelayModeCount; ++mode)
    {
        core->SetParameterValue(
            "node0/param/mode",
            static_cast<float>(mode)
                / static_cast<float>(kPedalDelayModeCount - 1));
        ASSERT_EQ(ModeAt(mode), core->GetEngine().GetMode());

        std::size_t slotParameters = 0;
        for(const auto& parameter : core->GetParameters())
        {
            if(parameter.slotId.empty())
            {
                continue;
            }
            ++slotParameters;
            EXPECT_FALSE(parameter.label.empty());
            EXPECT_FALSE(parameter.engineeringLabel.empty());
            EXPECT_FALSE(parameter.unitLabel.empty());
            EXPECT_FALSE(parameter.curve.empty());
            EXPECT_FALSE(parameter.dspTargets.empty()) << parameter.id;
            EXPECT_TRUE(std::isfinite(parameter.nativeMinimum));
            EXPECT_TRUE(std::isfinite(parameter.nativeMaximum));
            EXPECT_LT(parameter.nativeMinimum, parameter.nativeMaximum);
            EXPECT_GE(parameter.nativeDefault, parameter.nativeMinimum);
            EXPECT_LE(parameter.nativeDefault, parameter.nativeMaximum);
            if(parameter.isMacro)
            {
                EXPECT_GT(parameter.dspTargets.size(), 1u) << parameter.id;
            }
        }
        EXPECT_EQ(kPedalSlotCount, slotParameters)
            << "mode " << daisyhost::PedalDelayModeName(ModeAt(mode));
    }
}

TEST(PedalDelayCoreTest, ModeChangeRelabelsControlsWithoutRestart)
{
    auto       core    = MakeCore();
    const auto slotIds = core->GetPerformanceSlotParameterIds();

    const auto labelFor = [&core](const std::string& parameterId) {
        for(const auto& parameter : core->GetParameters())
        {
            if(parameter.id == parameterId)
            {
                return parameter.label;
            }
        }
        return std::string();
    };

    EXPECT_EQ("TIME", labelFor(slotIds[0]));
    core->SetParameterValue("node0/param/mode", 3.0f / 4.0f); // REV
    EXPECT_EQ(PedalDelayMode::kRev, core->GetEngine().GetMode());
    EXPECT_EQ("SLICE", labelFor(slotIds[0]));
    EXPECT_EQ("REVERSE", labelFor(slotIds[3]));
    core->SetParameterValue("node0/param/mode", 1.0f); // FREEZE
    EXPECT_EQ("LOOP", labelFor(slotIds[0]));
    EXPECT_EQ("DECAY", labelFor(slotIds[1]));
    // The app instance is never recreated.
    EXPECT_EQ("pedal_multidelay", core->GetAppId());
}

TEST(PedalDelayCoreTest, DisplayShowsMusicianAndEngineeringLabelsWithTarget)
{
    auto core = MakeCore();
    core->SetParameterValue("node0/param/color", 0.7f);
    core->TickUi(16.0);

    std::string joined;
    for(const auto& text : core->GetDisplayModel().texts)
    {
        joined += text.text + "\n";
    }
    EXPECT_NE(std::string::npos, joined.find("COLOR - BRIGHT"));
    EXPECT_NE(std::string::npos,
              joined.find("Feedback-path high-cut LPF cutoff"));
    EXPECT_NE(std::string::npos, joined.find("Hz"));
    EXPECT_NE(std::string::npos,
              joined.find("Target: feedback_conditioner.lpf_fc_hz"));
}

TEST(PedalDelayCoreTest, RejectsMalformedParameterValues)
{
    auto core = MakeCore();
    ASSERT_TRUE(core->SetParameterValue("node0/param/feedback", 0.6f));
    const auto before = core->GetParameterValue("node0/param/feedback");

    EXPECT_FALSE(core->SetParameterValue(
        "node0/param/feedback", std::numeric_limits<float>::quiet_NaN()));
    EXPECT_FALSE(core->SetParameterValue("node0/param/does_not_exist", 0.5f));

    const auto after = core->GetParameterValue("node0/param/feedback");
    EXPECT_FLOAT_EQ(before.value, after.value);
}

TEST(PedalDelayCoreTest, StatefulValuesSurviveCaptureAndRestore)
{
    auto core = MakeCore();
    core->SetParameterValue("node0/param/mode", 2.0f / 4.0f); // MOD
    core->SetParameterValue("node0/param/color", 0.82f);
    const auto captured = core->CaptureStatefulParameterValues();

    core->ResetToDefaultState(0);
    EXPECT_EQ(PedalDelayMode::kDigi, core->GetEngine().GetMode());

    core->RestoreStatefulParameterValues(captured);
    EXPECT_EQ(PedalDelayMode::kMod, core->GetEngine().GetMode());
    EXPECT_NEAR(
        0.82f, core->GetParameterValue("node0/param/color").value, 1e-4f);
}

TEST(PedalDelayCoreTest, PassesAudioThroughTheSelectedMode)
{
    auto core = MakeCore();
    core->SetParameterValue("node0/param/mix", 1.0f);

    const std::size_t  frames = kBlockSize;
    std::vector<float> input  = Sine(frames, 440.0f);
    std::vector<float> left(frames, 0.0f);
    std::vector<float> right(frames, 0.0f);
    const float*       inputChannels[2]  = {input.data(), input.data()};
    float*             outputChannels[2] = {left.data(), right.data()};

    daisyhost::AudioBufferView      view{inputChannels, 2};
    daisyhost::AudioBufferWriteView writeView{outputChannels, 2};
    core->Process(view, writeView, frames);

    EXPECT_TRUE(AllFinite(left));
    EXPECT_TRUE(AllFinite(right));
    // Mono-in stereo-out collapse: both channels stay identical.
    EXPECT_FLOAT_EQ(0.0f, MaxDifference(left, right));
}

// ---------------------------------------------------------------------------
// Cross-cutting DSP vectors
// ---------------------------------------------------------------------------

TEST(PedalDelayEngineTest, EverySlotChangesActualDspStateInEveryDelayMode)
{
    for(PedalDelayMode mode : {PedalDelayMode::kDigi,
                               PedalDelayMode::kTape,
                               PedalDelayMode::kMod,
                               PedalDelayMode::kRev})
    {
        for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
        {
            const auto low  = RunWithSlot(mode, SlotAt(slot), 0.15f);
            const auto high = RunWithSlot(mode, SlotAt(slot), 0.9f);
            ASSERT_TRUE(AllFinite(low));
            ASSERT_TRUE(AllFinite(high));
            EXPECT_GT(MaxDifference(low, high), 1e-4f)
                << daisyhost::PedalDelayModeName(mode) << " slot "
                << GetPedalSlotDescriptor(mode, SlotAt(slot)).slotId
                << " did not reach the DSP";
        }
    }
}

TEST(PedalDelayEngineTest, SilenceInProducesSilenceOutForEveryMode)
{
    for(std::size_t mode = 0; mode < kPedalDelayModeCount; ++mode)
    {
        EngineFixture fixture;
        fixture.engine.SetMode(ModeAt(mode));
        fixture.SetAllSlotsToDefault();
        fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
        const auto output = fixture.Run(Silence(4096));
        EXPECT_TRUE(AllFinite(output));
        EXPECT_FLOAT_EQ(0.0f, Peak(output))
            << daisyhost::PedalDelayModeName(ModeAt(mode));
    }
}

TEST(PedalDelayEngineTest, StaysFiniteUnderParameterExtremesAndModeChanges)
{
    EngineFixture      fixture;
    std::vector<float> input
        = Sine(static_cast<std::size_t>(kSampleRate), 330.0f, 0.9f);

    for(std::size_t mode = 0; mode < kPedalDelayModeCount; ++mode)
    {
        fixture.engine.SetMode(ModeAt(mode));
        for(float extreme : {0.0f, 1.0f})
        {
            for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
            {
                fixture.engine.SetSlotNormalized(SlotAt(slot), extreme);
            }
            const auto output = fixture.Run(input);
            EXPECT_TRUE(AllFinite(output))
                << daisyhost::PedalDelayModeName(ModeAt(mode));
            EXPECT_LT(Peak(output), 20.0f)
                << daisyhost::PedalDelayModeName(ModeAt(mode));
        }
    }
}

TEST(PedalDelayEngineTest, ModeChangesWhileRunningStayFinite)
{
    EngineFixture fixture;
    fixture.SetAllSlotsToDefault();
    const auto         input = Sine(kBlockSize, 440.0f, 0.8f);
    std::vector<float> left(kBlockSize, 0.0f);
    std::vector<float> right(kBlockSize, 0.0f);

    for(std::size_t block = 0; block < 200; ++block)
    {
        fixture.engine.SetMode(ModeAt(block % kPedalDelayModeCount));
        fixture.engine.Process(
            input.data(), input.data(), left.data(), right.data(), kBlockSize);
        ASSERT_TRUE(AllFinite(left)) << "block " << block;
        ASSERT_LT(Peak(left), 20.0f) << "block " << block;
    }
}

TEST(PedalDelayEngineTest, ImpulseWrapsAroundTheCircularHistory)
{
    EngineFixture fixture;
    fixture.SetAllSlotsToDefault();
    fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
    fixture.engine.SetSlotNormalized(PedalSlot::kMotion, 0.0f);
    fixture.SetSlotNative(PedalSlot::kTime, 500.0f);
    fixture.SetSlotNative(PedalSlot::kFeedback, 0.8f);
    fixture.RunSilence(static_cast<std::size_t>(kSampleRate));

    // Six seconds is longer than the 2.5 s history: the write pointer wraps
    // more than twice while the feedback loop keeps recirculating.
    std::vector<float> input
        = Impulse(static_cast<std::size_t>(kSampleRate * 6));
    const auto output = fixture.Run(input);
    EXPECT_TRUE(AllFinite(output));
    EXPECT_LT(Peak(output), 2.0f);
    EXPECT_GT(Rms(output, static_cast<std::size_t>(kSampleRate * 4)), 0.0f);
}

// ---------------------------------------------------------------------------
// DIGI
// ---------------------------------------------------------------------------

TEST(PedalDelayDigiTest, FractionalReadPlacesEnergyOnBothNeighbouringSamples)
{
    // Each case is a requested delay in samples with a deliberate fractional
    // part, so the impulse must land split across two adjacent output samples.
    for(float targetSamples : {480.5f, 4800.25f, 9600.5f})
    {
        EngineFixture fixture;
        fixture.SetAllSlotsToDefault();
        fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
        fixture.engine.SetSlotNormalized(PedalSlot::kFeedback, 0.0f);
        fixture.engine.SetSlotNormalized(PedalSlot::kMotion, 0.0f);
        fixture.engine.SetSlotNormalized(PedalSlot::kColor, 1.0f);
        fixture.SetSlotNative(PedalSlot::kTime,
                              targetSamples * 1000.0f
                                  / static_cast<float>(kSampleRate));
        fixture.RunSilence(static_cast<std::size_t>(kSampleRate));

        const float requestedSamples
            = fixture.engine.GetSlotNative(PedalSlot::kTime) * 0.001f
              * static_cast<float>(kSampleRate);
        const auto output
            = fixture.Run(Impulse(static_cast<std::size_t>(kSampleRate / 2)));
        ASSERT_TRUE(AllFinite(output));

        // Locate the impulse and describe it by energy and centroid.
        std::size_t peakIndex = 0;
        for(std::size_t i = 1; i < output.size(); ++i)
        {
            if(std::abs(output[i]) > std::abs(output[peakIndex]))
            {
                peakIndex = i;
            }
        }

        double      energy   = 0.0;
        double      centroid = 0.0;
        std::size_t nonZero  = 0;
        for(std::size_t i = peakIndex - 4; i <= peakIndex + 4; ++i)
        {
            const double magnitude = std::abs(output[i]);
            energy += magnitude;
            centroid += magnitude * static_cast<double>(i);
            if(magnitude > 1e-4)
            {
                ++nonZero;
            }
        }
        centroid /= std::max(energy, 1e-12);

        std::printf(
            "[digi-frac] requested=%.4f measured=%.4f energy=%.5f "
            "taps=%zu\n",
            requestedSamples,
            centroid,
            energy,
            nonZero);

        // Linear interpolation spreads the impulse over exactly two taps.
        EXPECT_EQ(2u, nonZero) << "requested " << requestedSamples;
        // No gain is lost or invented by the interpolator.
        EXPECT_NEAR(1.0, energy, 0.005) << "requested " << requestedSamples;
        // The delivered delay is sample-exact: budget is a twentieth of a
        // sample, which is what catches a smoother or mapping regression.
        EXPECT_LT(std::abs(centroid - requestedSamples), 0.05)
            << "requested " << requestedSamples << " measured " << centroid;
    }
}

TEST(PedalDelayDigiTest,
     BrightControlChangesRepeatBandwidthInTheStatedDirection)
{
    const auto measure = [](float colorNormalized) {
        EngineFixture fixture;
        fixture.SetAllSlotsToDefault();
        fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
        fixture.engine.SetSlotNormalized(PedalSlot::kMotion, 0.0f);
        fixture.SetSlotNative(PedalSlot::kTime, 120.0f);
        fixture.SetSlotNative(PedalSlot::kFeedback, 0.85f);
        fixture.engine.SetSlotNormalized(PedalSlot::kColor, colorNormalized);
        fixture.RunSilence(static_cast<std::size_t>(kSampleRate));

        std::vector<float> input
            = Impulse(static_cast<std::size_t>(kSampleRate * 2));
        const auto output = fixture.Run(input);
        // Measure well after the first repeat so the loop filter has acted
        // several times.
        return HighFrequencyRatio(output,
                                  static_cast<std::size_t>(kSampleRate));
    };

    const float dark   = measure(0.0f);
    const float bright = measure(1.0f);
    EXPECT_GT(bright, dark)
        << "clockwise BRIGHT must raise the feedback LPF cutoff";
}

TEST(PedalDelayDigiTest, StaysStableAtMaximumFeedback)
{
    EngineFixture fixture;
    fixture.SetAllSlotsToDefault();
    fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
    fixture.engine.SetSlotNormalized(PedalSlot::kFeedback, 1.0f);
    fixture.engine.SetSlotNormalized(PedalSlot::kColor, 1.0f);
    fixture.SetSlotNative(PedalSlot::kTime, 100.0f);

    std::vector<float> input
        = Sine(static_cast<std::size_t>(kSampleRate), 440.0f, 0.9f);
    input.resize(static_cast<std::size_t>(kSampleRate * 20), 0.0f);
    const auto output = fixture.Run(input);

    EXPECT_TRUE(AllFinite(output));
    EXPECT_LT(Peak(output), 10.0f);
    // The tail decays instead of running away.
    const auto lateStart = static_cast<std::size_t>(kSampleRate * 18);
    const auto midStart  = static_cast<std::size_t>(kSampleRate * 3);
    EXPECT_LT(Rms(output, lateStart), Rms(output, midStart));
}

TEST(PedalDelayDigiTest, LargeTimeChangeDoesNotClick)
{
    EngineFixture fixture;
    fixture.SetAllSlotsToDefault();
    fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
    fixture.engine.SetSlotNormalized(PedalSlot::kMotion, 0.0f);
    fixture.SetSlotNative(PedalSlot::kFeedback, 0.3f);
    fixture.SetSlotNative(PedalSlot::kTime, 120.0f);

    const auto tone = Sine(static_cast<std::size_t>(kSampleRate), 440.0f, 0.7f);
    const auto steady                = fixture.Run(tone);
    const float baselineEnvelopeStep = EnvelopeMaxStep(steady, 128, kBlockSize);

    fixture.SetSlotNative(PedalSlot::kTime, 900.0f);
    const auto transition = fixture.Run(tone);

    EXPECT_TRUE(AllFinite(transition));
    // The documented transition policy is a slewed read head (pitch warp), so
    // the instantaneous frequency changes but the level must not step. A
    // pointer jump would drop a discontinuity into the envelope.
    EXPECT_LT(EnvelopeMaxStep(transition, 128, kBlockSize),
              baselineEnvelopeStep + 0.10f);
    EXPECT_LT(Peak(transition), 1.5f);
}

// ---------------------------------------------------------------------------
// MOD
// ---------------------------------------------------------------------------

TEST(PedalDelayModTest, MovingHeadStaysInsideTheBufferAtEveryExtreme)
{
    for(float baseTime : {0.0f, 1.0f})
    {
        for(float depth : {0.0f, 1.0f})
        {
            for(float rate : {0.0f, 1.0f})
            {
                for(float resonance : {0.0f, 0.5f, 1.0f})
                {
                    EngineFixture fixture;
                    fixture.engine.SetMode(PedalDelayMode::kMod);
                    fixture.SetAllSlotsToDefault();
                    fixture.engine.SetSlotNormalized(PedalSlot::kTime,
                                                     baseTime);
                    fixture.engine.SetSlotNormalized(PedalSlot::kColor, depth);
                    fixture.engine.SetSlotNormalized(PedalSlot::kMotion, rate);
                    fixture.engine.SetSlotNormalized(PedalSlot::kFeedback,
                                                     resonance);
                    const auto output = fixture.Run(Sine(
                        static_cast<std::size_t>(kSampleRate), 220.0f, 0.9f));
                    ASSERT_TRUE(AllFinite(output));
                    ASSERT_LT(Peak(output), 20.0f);
                }
            }
        }
    }
}

TEST(PedalDelayModTest, SignedResonanceProducesDifferentCombPatterns)
{
    const auto positive
        = RunWithSlot(PedalDelayMode::kMod, PedalSlot::kFeedback, 0.95f);
    const auto negative
        = RunWithSlot(PedalDelayMode::kMod, PedalSlot::kFeedback, 0.05f);
    EXPECT_TRUE(AllFinite(positive));
    EXPECT_TRUE(AllFinite(negative));
    EXPECT_GT(MaxDifference(positive, negative), 1e-3f);
}

TEST(PedalDelayModTest, ControlRateUpdatesDoNotZipper)
{
    EngineFixture fixture;
    fixture.engine.SetMode(PedalDelayMode::kMod);
    fixture.SetAllSlotsToDefault();

    // 750 Hz is exactly one period per 64-frame block, so replaying the same
    // buffer produces a phase-continuous tone: any step found on a block
    // boundary comes from the control update, not from the test signal.
    const auto         input = Sine(kBlockSize, 750.0f, 0.6f);
    std::vector<float> left(kBlockSize, 0.0f);
    std::vector<float> right(kBlockSize, 0.0f);
    std::vector<float> collected;

    // Sweep DEPTH across its whole range and back, one block-rate step at a
    // time: exactly the update pattern that produces zipper noise when a
    // control is applied without smoothing.
    constexpr std::size_t kBlocks = 800;
    for(std::size_t block = 0; block < kBlocks; ++block)
    {
        const float ramp = static_cast<float>(block) / (kBlocks - 1);
        fixture.engine.SetSlotNormalized(PedalSlot::kColor,
                                         ramp < 0.5f ? ramp * 2.0f
                                                     : (1.0f - ramp) * 2.0f);
        fixture.engine.Process(
            input.data(), input.data(), left.data(), right.data(), kBlockSize);
        collected.insert(collected.end(), left.begin(), left.end());
    }

    ASSERT_TRUE(AllFinite(collected));

    // Zippering is a discontinuity that lands exactly on block boundaries.
    // Compare the worst boundary step against the worst step inside blocks: an
    // unsmoothed control makes the boundary steps stand out, a smoothed one
    // leaves them indistinguishable from ordinary signal slope.
    float boundaryStep = 0.0f;
    float interiorStep = 0.0f;
    for(std::size_t i = kBlockSize * 4; i < collected.size(); ++i)
    {
        const float step = std::abs(collected[i] - collected[i - 1]);
        if(i % kBlockSize == 0)
        {
            boundaryStep = std::max(boundaryStep, step);
        }
        else
        {
            interiorStep = std::max(interiorStep, step);
        }
    }
    std::printf("[mod-zipper] boundary=%.6f interior=%.6f\n",
                boundaryStep,
                interiorStep);
    EXPECT_LE(boundaryStep, interiorStep * 1.05f + 1e-4f);
}

// ---------------------------------------------------------------------------
// TAPE
// ---------------------------------------------------------------------------

TEST(PedalDelayTapeTest, AgeReducesRepeatBandwidth)
{
    const auto measure = [](float age) {
        EngineFixture fixture;
        fixture.engine.SetMode(PedalDelayMode::kTape);
        fixture.SetAllSlotsToDefault();
        fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
        fixture.engine.SetSlotNormalized(PedalSlot::kMotion, 0.0f);
        fixture.SetSlotNative(PedalSlot::kTime, 120.0f);
        fixture.SetSlotNative(PedalSlot::kFeedback, 0.85f);
        fixture.engine.SetSlotNormalized(PedalSlot::kColor, age);
        fixture.RunSilence(static_cast<std::size_t>(kSampleRate));
        const auto output
            = fixture.Run(Impulse(static_cast<std::size_t>(kSampleRate * 2)));
        return HighFrequencyRatio(output,
                                  static_cast<std::size_t>(kSampleRate));
    };

    EXPECT_LT(measure(1.0f), measure(0.0f))
        << "clockwise AGE must darken the repeats";
}

TEST(PedalDelayTapeTest, WarbleIncreasesReadPositionModulationAndStaysBounded)
{
    const auto run = [](float warble) {
        EngineFixture fixture;
        fixture.engine.SetMode(PedalDelayMode::kTape);
        fixture.SetAllSlotsToDefault();
        fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
        fixture.engine.SetSlotNormalized(PedalSlot::kColor, 0.0f);
        fixture.SetSlotNative(PedalSlot::kTime, 300.0f);
        fixture.SetSlotNative(PedalSlot::kFeedback, 0.2f);
        fixture.engine.SetSlotNormalized(PedalSlot::kMotion, warble);
        fixture.RunSilence(static_cast<std::size_t>(kSampleRate));
        return fixture.Run(
            Sine(static_cast<std::size_t>(kSampleRate * 2), 440.0f, 0.7f));
    };

    const auto stable  = run(0.0f);
    const auto wobbled = run(1.0f);
    EXPECT_TRUE(AllFinite(stable));
    EXPECT_TRUE(AllFinite(wobbled));
    EXPECT_GT(MaxDifference(stable, wobbled), 1e-3f);
    // The transport never runs away: total depth is bounded by the documented
    // drift + wow + flutter budget.
    EXPECT_LT(Peak(wobbled), 2.0f);
}

TEST(PedalDelayTapeTest, StaysFiniteAtHighFeedbackAndFullAge)
{
    EngineFixture fixture;
    fixture.engine.SetMode(PedalDelayMode::kTape);
    fixture.SetAllSlotsToDefault();
    fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
    fixture.engine.SetSlotNormalized(PedalSlot::kFeedback, 1.0f);
    fixture.engine.SetSlotNormalized(PedalSlot::kColor, 1.0f);
    fixture.engine.SetSlotNormalized(PedalSlot::kMotion, 1.0f);

    std::vector<float> input
        = Sine(static_cast<std::size_t>(kSampleRate), 220.0f, 0.9f);
    input.resize(static_cast<std::size_t>(kSampleRate * 15), 0.0f);
    const auto output = fixture.Run(input);

    EXPECT_TRUE(AllFinite(output));
    EXPECT_LT(Peak(output), 5.0f);
}

// ---------------------------------------------------------------------------
// REV
// ---------------------------------------------------------------------------

TEST(PedalDelayRevTest, OverlapAddGainStaysBoundedForEveryGrainSetting)
{
    for(float grain : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f})
    {
        EngineFixture fixture;
        fixture.engine.SetMode(PedalDelayMode::kRev);
        fixture.SetAllSlotsToDefault();
        fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
        fixture.engine.SetSlotNormalized(PedalSlot::kColor,
                                         1.0f); // full reverse
        fixture.engine.SetSlotNormalized(PedalSlot::kFeedback, 0.0f);
        fixture.engine.SetSlotNormalized(PedalSlot::kMotion, grain);
        fixture.SetSlotNative(PedalSlot::kTime, 200.0f);

        // Constant input: a correctly normalized overlap-add returns the same
        // constant, whatever the window taper is.
        const std::vector<float> dc(static_cast<std::size_t>(kSampleRate * 3),
                                    1.0f);
        const auto               output = fixture.Run(dc);
        ASSERT_TRUE(AllFinite(output));

        const auto begin   = static_cast<std::size_t>(kSampleRate * 2);
        float      minimum = 2.0f;
        float      maximum = -2.0f;
        for(std::size_t i = begin; i < output.size(); ++i)
        {
            minimum = std::min(minimum, output[i]);
            maximum = std::max(maximum, output[i]);
        }
        EXPECT_GT(minimum, 0.9f) << "grain " << grain;
        EXPECT_LT(maximum, 1.05f) << "grain " << grain;
    }
}

TEST(PedalDelayRevTest, ReportsAlgorithmicLatencyOnlyForReverse)
{
    EngineFixture fixture;
    fixture.SetAllSlotsToDefault();
    EXPECT_FLOAT_EQ(0.0f, fixture.engine.GetAlgorithmicLatencySamples());

    fixture.engine.SetMode(PedalDelayMode::kRev);
    fixture.SetAllSlotsToDefault();
    fixture.SetSlotNative(PedalSlot::kTime, 300.0f);
    fixture.RunSilence(static_cast<std::size_t>(kSampleRate));

    const float expected = 0.300f * static_cast<float>(kSampleRate);
    EXPECT_NEAR(
        expected, fixture.engine.GetAlgorithmicLatencySamples(), 200.0f);
}

TEST(PedalDelayRevTest, SeamsAndReinjectionStayBoundedOnTransients)
{
    EngineFixture fixture;
    fixture.engine.SetMode(PedalDelayMode::kRev);
    fixture.SetAllSlotsToDefault();
    fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
    fixture.engine.SetSlotNormalized(PedalSlot::kColor, 1.0f);
    fixture.engine.SetSlotNormalized(PedalSlot::kFeedback, 1.0f);
    fixture.engine.SetSlotNormalized(PedalSlot::kMotion,
                                     0.0f); // shortest taper
    fixture.SetSlotNative(PedalSlot::kTime, 80.0f);

    // Repeated hard transients, the worst case for grain seams.
    std::vector<float> input(static_cast<std::size_t>(kSampleRate * 4), 0.0f);
    for(std::size_t i = 0; i < input.size(); i += 4800)
    {
        input[i] = 1.0f;
    }
    const auto output = fixture.Run(input);

    EXPECT_TRUE(AllFinite(output));
    EXPECT_LT(Peak(output), 4.0f);
}

// ---------------------------------------------------------------------------
// FREEZE
// ---------------------------------------------------------------------------

TEST(PedalDelayFreezeTest, CaptureTransitionsToHoldAndRejectsFreshInput)
{
    EngineFixture fixture;
    fixture.engine.SetMode(PedalDelayMode::kFreeze);
    fixture.SetAllSlotsToDefault();
    fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
    fixture.engine.SetSlotNormalized(PedalSlot::kMotion, 0.0f);
    fixture.SetSlotNative(PedalSlot::kTime, 200.0f);

    EXPECT_EQ(PedalFreezeState::kIdle, fixture.engine.GetFreezeState());
    fixture.engine.SetFreezeState(PedalFreezeState::kCapture);
    fixture.Run(Sine(static_cast<std::size_t>(kSampleRate / 2), 220.0f, 0.7f));
    EXPECT_EQ(PedalFreezeState::kHold, fixture.engine.GetFreezeState());

    const auto heldWithSilence = fixture.Run(Silence(kBlockSize * 16));
    EXPECT_GT(Rms(heldWithSilence), 1e-4f)
        << "the captured loop keeps sounding";

    // Same engine state reached twice: hold must ignore whatever arrives.
    EngineFixture quiet;
    EngineFixture loud;
    for(EngineFixture* fx : {&quiet, &loud})
    {
        fx->engine.SetMode(PedalDelayMode::kFreeze);
        fx->SetAllSlotsToDefault();
        fx->engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
        fx->engine.SetSlotNormalized(PedalSlot::kMotion, 0.0f);
        fx->SetSlotNative(PedalSlot::kTime, 200.0f);
        fx->engine.SetFreezeState(PedalFreezeState::kCapture);
        fx->Run(Sine(static_cast<std::size_t>(kSampleRate / 2), 220.0f, 0.7f));
        ASSERT_EQ(PedalFreezeState::kHold, fx->engine.GetFreezeState());
    }
    const std::size_t frames      = static_cast<std::size_t>(kSampleRate);
    const auto        tone        = Sine(frames, 900.0f, 0.9f);
    const auto        withSilence = quiet.Run(Silence(frames));
    const auto        withInput   = loud.Run(tone);
    // The residual is the wet/dry crossfade arithmetic, not a state leak: any
    // real admission of the 0.9 input would show far above -60 dB.
    EXPECT_LT(MaxDifference(withSilence, withInput), Peak(tone) * 1e-3f)
        << "hold must reject fresh input";
}

TEST(PedalDelayFreezeTest, AccumulateAdmitsFreshInputAndReplaceDiscardsCapture)
{
    const auto capture = [](PedalFreezeState after) {
        auto fixture = std::make_unique<EngineFixture>();
        fixture->engine.SetMode(PedalDelayMode::kFreeze);
        fixture->SetAllSlotsToDefault();
        fixture->engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
        fixture->engine.SetSlotNormalized(PedalSlot::kMotion, 0.0f);
        fixture->SetSlotNative(PedalSlot::kTime, 200.0f);
        fixture->engine.SetFreezeState(PedalFreezeState::kCapture);
        fixture->Run(
            Sine(static_cast<std::size_t>(kSampleRate / 2), 220.0f, 0.7f));
        fixture->engine.SetFreezeState(after);
        return fixture;
    };

    auto held        = capture(PedalFreezeState::kHold);
    auto accumulated = capture(PedalFreezeState::kAccumulate);
    // Injected material only reappears after one full loop (200 ms), so the
    // comparison window has to be longer than the loop.
    const auto tone = Sine(static_cast<std::size_t>(kSampleRate), 900.0f, 0.8f);
    const auto heldOutput = held->Run(tone);
    const auto accOutput  = accumulated->Run(tone);
    EXPECT_GT(MaxDifference(heldOutput, accOutput), 1e-3f)
        << "accumulate must admit fresh input";

    auto replaced = capture(PedalFreezeState::kHold);
    replaced->engine.SetFreezeState(PedalFreezeState::kReplace);
    replaced->Run(Silence(static_cast<std::size_t>(kSampleRate / 2)));
    EXPECT_EQ(PedalFreezeState::kHold, replaced->engine.GetFreezeState());
    const auto afterReplace
        = replaced->Run(Silence(static_cast<std::size_t>(kSampleRate)));
    EXPECT_LT(Peak(afterReplace), 1e-4f)
        << "replace must discard the previous capture";
}

TEST(PedalDelayFreezeTest, ClearReturnsToAKnownEmptyState)
{
    EngineFixture fixture;
    fixture.engine.SetMode(PedalDelayMode::kFreeze);
    fixture.SetAllSlotsToDefault();
    fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
    fixture.SetSlotNative(PedalSlot::kTime, 200.0f);
    fixture.engine.SetFreezeState(PedalFreezeState::kCapture);
    fixture.Run(Sine(static_cast<std::size_t>(kSampleRate / 2), 220.0f, 0.7f));

    fixture.engine.FreezeClear();
    EXPECT_EQ(PedalFreezeState::kIdle, fixture.engine.GetFreezeState());
    const auto output = fixture.Run(Silence(kBlockSize * 64));
    EXPECT_FLOAT_EQ(0.0f, Peak(output));
}

TEST(PedalDelayFreezeTest, RecirculationStaysFiniteAtMaximumDecay)
{
    EngineFixture fixture;
    fixture.engine.SetMode(PedalDelayMode::kFreeze);
    fixture.SetAllSlotsToDefault();
    fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
    fixture.engine.SetSlotNormalized(PedalSlot::kFeedback, 1.0f);
    fixture.engine.SetSlotNormalized(PedalSlot::kColor, 0.0f);
    fixture.engine.SetSlotNormalized(PedalSlot::kMotion, 1.0f);
    fixture.SetSlotNative(PedalSlot::kTime, 500.0f);

    fixture.engine.SetFreezeState(PedalFreezeState::kAccumulate);
    fixture.engine.SetFreezeState(PedalFreezeState::kCapture);
    fixture.Run(Sine(static_cast<std::size_t>(kSampleRate), 220.0f, 0.9f));
    fixture.engine.SetFreezeState(PedalFreezeState::kAccumulate);

    const auto output = fixture.Run(
        Sine(static_cast<std::size_t>(kSampleRate * 30), 220.0f, 0.9f));
    EXPECT_TRUE(AllFinite(output));
    EXPECT_LT(Peak(output), 10.0f);
}

TEST(PedalDelayFreezeTest, HostExposesEveryStateTransitionExplicitly)
{
    auto core = MakeCore();
    core->SetParameterValue("node0/param/mode", 1.0f); // FREEZE

    struct Case
    {
        const char*      item;
        PedalFreezeState expected;
    };
    static const std::array<Case, 4> kCases = {{
        {"capture", PedalFreezeState::kCapture},
        {"hold", PedalFreezeState::kHold},
        {"accumulate", PedalFreezeState::kAccumulate},
        {"replace", PedalFreezeState::kReplace},
    }};
    for(const auto& testCase : kCases)
    {
        core->SetMenuItemValue(
            std::string("node0/menu/switches/") + testCase.item, 1.0f);
        EXPECT_EQ(testCase.expected, core->GetEngine().GetFreezeState())
            << testCase.item;
    }
    core->SetMenuItemValue("node0/menu/switches/clear", 1.0f);
    EXPECT_EQ(PedalFreezeState::kIdle, core->GetEngine().GetFreezeState());
}

// ---------------------------------------------------------------------------
// Bypass / trails
// ---------------------------------------------------------------------------

TEST(PedalDelayBypassTest, TrailsKeepTheTailWhileBypassMutesTheInputSend)
{
    const auto run = [](bool trails) {
        EngineFixture fixture;
        fixture.SetAllSlotsToDefault();
        fixture.engine.SetTrails(trails);
        fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);
        fixture.SetSlotNative(PedalSlot::kTime, 300.0f);
        fixture.SetSlotNative(PedalSlot::kFeedback, 0.7f);
        fixture.Run(Sine(static_cast<std::size_t>(kSampleRate), 440.0f, 0.7f));
        fixture.engine.SetBypass(true);
        return fixture.Run(Silence(static_cast<std::size_t>(kSampleRate)));
    };

    EXPECT_GT(Rms(run(true)), 1e-4f) << "trails must survive bypass";
    EXPECT_FLOAT_EQ(0.0f, Peak(run(false)));
}

TEST(PedalDelayBypassTest, BypassWithoutTrailsPassesTheDrySignalUntouched)
{
    EngineFixture fixture;
    fixture.SetAllSlotsToDefault();
    fixture.engine.SetTrails(false);
    fixture.engine.SetBypass(true);
    fixture.engine.SetSlotNormalized(PedalSlot::kMix, 1.0f);

    const auto input
        = Sine(static_cast<std::size_t>(kSampleRate / 4), 440.0f, 0.6f);
    const auto output = fixture.Run(input);
    EXPECT_FLOAT_EQ(0.0f, MaxDifference(input, output));
}

// ---------------------------------------------------------------------------
// Host performance evidence (host only, never a target claim)
// ---------------------------------------------------------------------------

TEST(PedalDelayHostPerformanceTest, RecordsBlockProcessTiming)
{
    const auto         input = Sine(kBlockSize, 440.0f, 0.7f);
    std::vector<float> left(kBlockSize, 0.0f);
    std::vector<float> right(kBlockSize, 0.0f);

    for(std::size_t mode = 0; mode < kPedalDelayModeCount; ++mode)
    {
        EngineFixture fixture;
        fixture.engine.SetMode(ModeAt(mode));
        fixture.SetAllSlotsToDefault();
        if(ModeAt(mode) == PedalDelayMode::kFreeze)
        {
            fixture.engine.SetFreezeState(PedalFreezeState::kAccumulate);
        }

        // 10 s of audio at 64 frames per block.
        const std::size_t blocks
            = static_cast<std::size_t>(kSampleRate * 10) / kBlockSize;
        double totalMicroseconds = 0.0;
        double worstMicroseconds = 0.0;
        for(std::size_t block = 0; block < blocks; ++block)
        {
            const auto start = std::chrono::steady_clock::now();
            fixture.engine.Process(input.data(),
                                   input.data(),
                                   left.data(),
                                   right.data(),
                                   kBlockSize);
            const auto elapsed = std::chrono::duration<double, std::micro>(
                                     std::chrono::steady_clock::now() - start)
                                     .count();
            totalMicroseconds += elapsed;
            worstMicroseconds = std::max(worstMicroseconds, elapsed);
        }

        const double averageMicroseconds
            = totalMicroseconds / static_cast<double>(blocks);
        const double blockBudgetMicroseconds
            = static_cast<double>(kBlockSize) / kSampleRate * 1.0e6;
        const std::string name = daisyhost::PedalDelayModeName(ModeAt(mode));
        RecordProperty(name + "_avg_us", std::to_string(averageMicroseconds));
        RecordProperty(name + "_worst_us", std::to_string(worstMicroseconds));
        std::printf(
            "[host-perf] mode=%s sr=%.0f block=%zu channels=%zu avg=%.2fus "
            "worst=%.2fus budget=%.2fus\n",
            name.c_str(),
            kSampleRate,
            kBlockSize,
            kPedalChannelCount,
            averageMicroseconds,
            worstMicroseconds,
            blockBudgetMicroseconds);

        EXPECT_LT(averageMicroseconds, blockBudgetMicroseconds)
            << name << " does not run in real time on this host";
    }
}
