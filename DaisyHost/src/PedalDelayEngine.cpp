#include "daisyhost/PedalDelayEngine.h"

#include <algorithm>
#include <cmath>
#include <string_view>

namespace daisyhost
{
namespace
{
    // Repository conventions reused here:
    //   * fractional reads use linear interpolation between the two
    //     neighbouring samples, same as DaisyDelayFxCore::DelayLine::Read;
    //   * dry/wet uses the linear crossfade law of DaisyDelayFxCore::Mix.
    float Mix(float a, float b, float amount)
    {
        return a + (b - a) * std::clamp(amount, 0.0f, 1.0f);
    }

    float Sanitize(float value)
    {
        return std::isfinite(value) ? value : 0.0f;
    }

    float FlushDenormal(float value)
    {
        return std::abs(value) < 1.0e-20f ? 0.0f : value;
    }

    // Normalized soft saturation: unity slope at low level, bounded at high
    // level, so TAPE stays finite at maximum feedback and drive.
    float SoftSaturate(float input, float drive)
    {
        const float gain = 1.0f + drive * 4.0f;
        return std::tanh(input * gain) / gain;
    }

    // Audio-rate variant: single precision, no double math in the callback.
    // ponytail: powf per sample is fine on the host but is a known target cost;
    // replace with a table or a per-block update if the M7 profile demands it.
    float ExponentialMap(float low, float high, float amount)
    {
        return low * std::pow(high / low, std::clamp(amount, 0.0f, 1.0f));
    }

    // Control-rate variant. float powf costs ~1e-4 relative accuracy, which on
    // a 200 ms delay is a full sample of timing error, so the UI-to-engineering
    // mapping uses double.
    float ExponentialMapPrecise(float low, float high, float amount)
    {
        return static_cast<float>(
            static_cast<double>(low)
            * std::pow(static_cast<double>(high) / static_cast<double>(low),
                       static_cast<double>(std::clamp(amount, 0.0f, 1.0f))));
    }

    float Fract(float value) { return value - std::floor(value); }

    // Tukey window. taper == 0.5 is exactly Hann; smaller tapers keep a
    // plateau. The reverse reader normalizes by the summed window, so any
    // taper stays gain-bounded.
    float TukeyWindow(float phase, float taper)
    {
        const float u = std::clamp(phase, 0.0f, 1.0f);
        const float t = std::clamp(taper, 0.01f, 0.5f);
        if(u < t)
        {
            return 0.5f - 0.5f * std::cos(3.14159265358979f * u / t);
        }
        if(u > 1.0f - t)
        {
            return 0.5f
                   - 0.5f * std::cos(3.14159265358979f * (1.0f - u) / t);
        }
        return 1.0f;
    }

    // Internal, documented constants. They are not user controls.
    constexpr float kDigiDriftRateHz    = 0.31f;
    constexpr float kTapeDriftRateHz    = 0.09f;
    constexpr float kTapeWowRateHz      = 1.10f;
    constexpr float kTapeFlutterRateHz  = 11.30f;
    constexpr float kTapeDriftDepthMs   = 8.0f;
    constexpr float kTapeWowDepthMs     = 3.0f;
    constexpr float kTapeFlutterDepthMs = 0.6f;
    constexpr float kFreezeInjectionGain = 0.5f;
    constexpr float kMinTapMs            = 100.0f;
    constexpr float kMaxTapMs            = 2000.0f;

    using Slots = std::array<PedalSlotDescriptor, kPedalSlotCount>;

    // Product/UI contract. Every range is provisional: current repository
    // sources do not freeze pedal-facing ranges, so these are host tuning
    // values, not product-frozen values.
    const std::array<Slots, kPedalDelayModeCount>& SlotTable()
    {
        static const std::array<Slots, kPedalDelayModeCount> kTable = {{
            // DIGI
            Slots{{
                {"TIME", "TIME", "Fractional forward-read delay time", "ms",
                 "log", 20.0f, 2000.0f, 400.0f, 60.0f, "longer delay",
                 "repeats move further apart", false, true,
                 {"history.read_delay_ms"}},
                {"FEEDBACK", "REPEATS", "Feedback-loop gain coefficient",
                 "ratio", "linear", 0.0f, 0.95f, 0.35f, 20.0f, "more repeats",
                 "repeats last longer", false, true,
                 {"feedback_conditioner.loop_gain"}},
                {"MIX", "MIX", "Wet/dry linear crossfade gain", "ratio",
                 "linear", 0.0f, 1.0f, 0.35f, 20.0f, "more wet",
                 "delay louder against the dry signal", false, true,
                 {"output.wet_dry_crossfade"}},
                {"COLOR", "BRIGHT", "Feedback-path high-cut LPF cutoff", "Hz",
                 "log", 500.0f, 12000.0f, 6000.0f, 30.0f, "brighter repeats",
                 "repeats keep more high frequency content", false, true,
                 {"feedback_conditioner.lpf_fc_hz"}},
                {"MOTION", "DRIFT", "Fractional delay-read modulation depth",
                 "ms", "linear", 0.0f, 5.0f, 0.0f, 50.0f, "more drift",
                 "repeats detune slowly; zero is a clean digital baseline",
                 false, true, {"history.read_modulation_depth_ms"}},
            }},
            // TAPE
            Slots{{
                {"TIME", "TIME", "Nominal transport read delay time", "ms",
                 "log", 40.0f, 1200.0f, 380.0f, 220.0f, "longer delay",
                 "transport slides to the new time, warping pitch on the way",
                 false, true, {"transport.nominal_delay_ms"}},
                {"FEEDBACK", "REPEATS",
                 "Feedback-loop gain after tape/replay conditioning", "ratio",
                 "linear", 0.0f, 0.95f, 0.45f, 20.0f, "more repeats",
                 "repeats last longer and age further", false, true,
                 {"feedback_conditioner.loop_gain_post_tape"}},
                {"MIX", "MIX", "Wet/dry linear crossfade gain", "ratio",
                 "linear", 0.0f, 1.0f, 0.40f, 20.0f, "more wet",
                 "delay louder against the dry signal", false, true,
                 {"output.wet_dry_crossfade"}},
                {"COLOR", "AGE",
                 "Repeat bandwidth loss and replay nonlinearity macro",
                 "normalized", "linear", 0.0f, 1.0f, 0.35f, 40.0f,
                 "older, darker, more saturated repeats",
                 "repeats lose treble and bass and soft-compress", true, true,
                 {"feedback_conditioner.lpf_fc_hz",
                  "feedback_conditioner.hpf_fc_hz",
                  "feedback_conditioner.saturation_drive"}},
                {"MOTION", "WARBLE",
                 "Transport read-position drift/wow/flutter macro",
                 "normalized", "linear", 0.0f, 1.0f, 0.25f, 60.0f,
                 "more transport instability",
                 "pitch wobbles slowly and quickly at once", true, true,
                 {"transport.drift_depth_ms", "transport.wow_depth_ms",
                  "transport.flutter_depth_ms"}},
            }},
            // MOD
            Slots{{
                {"TIME", "BASE TIME", "Base fractional comb delay time", "ms",
                 "log", 0.5f, 50.0f, 8.0f, 40.0f, "longer base delay",
                 "moves from flanger through chorus to slapback", false, true,
                 {"universal_comb.base_delay_ms"}},
                {"FEEDBACK", "RESONANCE",
                 "Signed universal-comb feedback coefficient", "ratio",
                 "bipolar", -0.95f, 0.95f, 0.0f, 20.0f,
                 "positive resonance; counter-clockwise is inverted",
                 "comb peaks get sharper, sign flips the notch pattern", false,
                 true, {"universal_comb.feedback_coefficient"}},
                {"MIX", "MIX", "Comb blend/feedforward gain pair", "ratio",
                 "linear", 0.0f, 1.0f, 0.50f, 20.0f, "more delayed path",
                 "moves from dry through comb notches to full vibrato", true,
                 true,
                 {"universal_comb.blend_gain",
                  "universal_comb.feedforward_gain"}},
                {"COLOR", "DEPTH", "Delay-read modulation depth", "ms",
                 "linear", 0.0f, 10.0f, 2.0f, 30.0f, "deeper sweep",
                 "wider pitch and comb-frequency sweep", false, true,
                 {"universal_comb.delay_modulation_depth_ms"}},
                {"MOTION", "RATE", "Modulation oscillator rate", "Hz", "log",
                 0.02f, 8.0f, 0.6f, 30.0f, "faster sweep",
                 "modulation speeds up from drift to vibrato", false, true,
                 {"mod_source.lfo_rate_hz"}},
            }},
            // REV
            Slots{{
                {"TIME", "SLICE", "Reverse grain/segment duration", "ms", "log",
                 50.0f, 1200.0f, 300.0f, 120.0f, "longer reversed slice",
                 "longer phrases are reversed at once", false, true,
                 {"reverse_reader.grain_duration_ms"}},
                {"FEEDBACK", "REPEATS",
                 "Post-overlap-add re-injection gain", "ratio", "linear", 0.0f,
                 0.85f, 0.25f, 20.0f, "more reversed repeats",
                 "reversed material re-enters the history and stacks", false,
                 true, {"reverse_reader.post_ola_reinjection_gain"}},
                {"MIX", "MIX", "Wet/dry linear crossfade gain", "ratio",
                 "linear", 0.0f, 1.0f, 0.50f, 20.0f, "more wet",
                 "reversed layer louder against the dry signal", false, true,
                 {"output.wet_dry_crossfade"}},
                {"COLOR", "REVERSE", "Forward/reverse branch crossfade",
                 "normalized", "linear", 0.0f, 1.0f, 1.0f, 30.0f,
                 "fully reversed", "blends a plain forward delay into reverse",
                 false, true, {"reverse_reader.forward_reverse_crossfade"}},
                {"MOTION", "GRAIN", "Reverse-grain window taper fraction",
                 "normalized", "linear", 0.0f, 1.0f, 0.5f, 40.0f,
                 "longer seam crossfade",
                 "grain seams go from abrupt to fully smoothed", false, true,
                 {"reverse_reader.window_taper_fraction"}},
            }},
            // FREEZE
            Slots{{
                {"TIME", "LOOP", "Capture/recirculating loop length", "ms",
                 "log", 50.0f, 2000.0f, 600.0f, 0.0f, "longer captured loop",
                 "the held texture repeats over a longer window", false, true,
                 {"freeze_loop.length_ms"}},
                {"FEEDBACK", "DECAY", "Loop recirculation gain", "ratio",
                 "linear", 0.50f, 0.999f, 0.97f, 30.0f, "longer hold",
                 "the frozen layer decays more slowly", false, true,
                 {"freeze_loop.recirculation_gain"}},
                {"MIX", "LEVEL", "Freeze-layer wet crossfade gain", "ratio",
                 "linear", 0.0f, 1.0f, 0.60f, 20.0f, "louder freeze layer",
                 "frozen layer louder against the dry signal", false, true,
                 {"freeze_loop.output_gain"}},
                {"COLOR", "DAMPING", "Loop LPF/HPF damping macro",
                 "normalized", "linear", 0.0f, 1.0f, 0.30f, 40.0f,
                 "more damping",
                 "the held texture loses treble and bass as it recirculates",
                 true, true,
                 {"freeze_loop.lpf_fc_hz", "freeze_loop.hpf_fc_hz"}},
                {"MOTION", "EVOLVE",
                 "Ultra-slow fractional-read drift macro", "normalized",
                 "linear", 0.0f, 1.0f, 0.10f, 60.0f, "more evolution",
                 "the loop slowly detunes and never sits still", true, true,
                 {"freeze_loop.drift_depth_ms", "freeze_loop.drift_rate_hz"}},
            }},
        }};
        return kTable;
    }
} // namespace

const char* PedalDelayModeName(PedalDelayMode mode)
{
    switch(mode)
    {
        case PedalDelayMode::kDigi: return "DIGI";
        case PedalDelayMode::kTape: return "TAPE";
        case PedalDelayMode::kMod: return "MOD";
        case PedalDelayMode::kRev: return "REV";
        case PedalDelayMode::kFreeze: return "FREEZE";
        default: return "DIGI";
    }
}

const char* PedalFreezeStateName(PedalFreezeState state)
{
    switch(state)
    {
        case PedalFreezeState::kIdle: return "idle";
        case PedalFreezeState::kCapture: return "capture";
        case PedalFreezeState::kHold: return "hold";
        case PedalFreezeState::kAccumulate: return "accumulate";
        case PedalFreezeState::kReplace: return "replace";
        default: return "idle";
    }
}

const PedalSlotDescriptor& GetPedalSlotDescriptor(PedalDelayMode mode,
                                                  PedalSlot      slot)
{
    const std::size_t modeIndex = std::min(static_cast<std::size_t>(mode),
                                           kPedalDelayModeCount - 1);
    const std::size_t slotIndex
        = std::min(static_cast<std::size_t>(slot), kPedalSlotCount - 1);
    return SlotTable()[modeIndex][slotIndex];
}

float PedalDelayEngine::OnePole::Lowpass(float input, float coefficient)
{
    state += (input - state) * coefficient;
    state = FlushDenormal(state);
    return state;
}

float PedalDelayEngine::OnePole::Highpass(float input, float coefficient)
{
    state += (input - state) * coefficient;
    state = FlushDenormal(state);
    return input - state;
}

float PedalDelayEngine::Lfo::Sine(float rateHz, float sampleRate)
{
    phase += rateHz / std::max(sampleRate, 1.0f);
    phase = Fract(phase);
    return std::sin(6.28318530717959f * phase);
}

PedalDelayEngine::PedalDelayEngine()
{
    ResetParameters();
}

std::size_t PedalDelayEngine::HistorySamplesForRate(double sampleRate)
{
    return static_cast<std::size_t>(sampleRate * kHistorySeconds) + 8;
}

std::size_t PedalDelayEngine::FreezeSamplesForRate(double sampleRate)
{
    return static_cast<std::size_t>(sampleRate * kFreezeLoopSeconds) + 8;
}

void PedalDelayEngine::AttachStorage(float*      history,
                                     std::size_t historySamplesPerChannel,
                                     float*      freezeLoop,
                                     std::size_t freezeSamplesPerChannel)
{
    history_        = history;
    historySamples_ = historySamplesPerChannel;
    freezeLoop_     = freezeLoop;
    freezeSamples_  = freezeSamplesPerChannel;
    writeIndex_     = 0;
}

void PedalDelayEngine::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    sampleRate_   = sampleRate > 0.0 ? sampleRate : 48000.0;
    maxBlockSize_ = maxBlockSize > 0 ? maxBlockSize : 1;
    prepared_     = history_ != nullptr && historySamples_ > 16
                && freezeLoop_ != nullptr && freezeSamples_ > 16;

    for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
    {
        const auto& descriptor
            = GetPedalSlotDescriptor(mode_, static_cast<PedalSlot>(slot));
        const float samples
            = descriptor.smoothingMs * 0.001f * static_cast<float>(sampleRate_);
        smoothingCoefficient_[slot]
            = samples > 1.0f ? 1.0f - std::exp(-1.0f / samples) : 1.0f;
    }
    Reset();
}

void PedalDelayEngine::Reset()
{
    if(history_ != nullptr)
    {
        std::fill(history_,
                  history_ + historySamples_ * kPedalChannelCount,
                  0.0f);
    }
    if(freezeLoop_ != nullptr)
    {
        std::fill(freezeLoop_,
                  freezeLoop_ + freezeSamples_ * kPedalChannelCount,
                  0.0f);
    }
    writeIndex_ = 0;
    for(auto& channel : channels_)
    {
        channel = ChannelState{};
    }
    modLfo_.Clear();
    driftLfo_.Clear();
    wowLfo_.Clear();
    flutterLfo_.Clear();
    evolveLfo_.Clear();
    revGrainPhase_ = 0.0f;

    freezeState_        = PedalFreezeState::kIdle;
    freezeLoopLength_   = 0;
    freezeCaptureCount_ = 0;
    freezeReadIndex_    = 0.0f;
    lastTapMs_          = -1.0;
    tapTempoMs_         = 0.0f;
    SnapSmoothingToTargets();
}

void PedalDelayEngine::ResetParameters()
{
    for(std::size_t mode = 0; mode < kPedalDelayModeCount; ++mode)
    {
        for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
        {
            slotNormalized_[mode][slot] = NativeToNormalized(
                static_cast<PedalDelayMode>(mode),
                static_cast<PedalSlot>(slot),
                GetPedalSlotDescriptor(static_cast<PedalDelayMode>(mode),
                                       static_cast<PedalSlot>(slot))
                    .defaultValue);
        }
    }
    bypassed_ = false;
    SnapSmoothingToTargets();
}

void PedalDelayEngine::SetMode(PedalDelayMode mode)
{
    if(mode == mode_ || static_cast<std::size_t>(mode) >= kPedalDelayModeCount)
    {
        return;
    }
    mode_ = mode;

    // Mode-change policy: the shared history and the freeze loop survive the
    // change (the existing tail keeps ringing), conditioning filters and
    // modulation phases are cleared, and the parameter smoothers snap to the
    // new mode's values because the units differ between modes.
    for(auto& channel : channels_)
    {
        channel.feedbackLpf.Clear();
        channel.ageHpf.Clear();
        channel.freezeLpf.Clear();
        channel.freezeHpf.Clear();
    }
    modLfo_.Clear();
    driftLfo_.Clear();
    wowLfo_.Clear();
    flutterLfo_.Clear();
    evolveLfo_.Clear();
    revGrainPhase_ = 0.0f;

    for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
    {
        const auto& descriptor
            = GetPedalSlotDescriptor(mode_, static_cast<PedalSlot>(slot));
        const float samples
            = descriptor.smoothingMs * 0.001f * static_cast<float>(sampleRate_);
        smoothingCoefficient_[slot]
            = samples > 1.0f ? 1.0f - std::exp(-1.0f / samples) : 1.0f;
    }
    SnapSmoothingToTargets();
}

bool PedalDelayEngine::SetSlotNormalized(PedalSlot slot, float normalizedValue)
{
    const std::size_t slotIndex = static_cast<std::size_t>(slot);
    if(slotIndex >= kPedalSlotCount || !std::isfinite(normalizedValue))
    {
        return false;
    }
    slotNormalized_[static_cast<std::size_t>(mode_)][slotIndex]
        = std::clamp(normalizedValue, 0.0f, 1.0f);
    return true;
}

float PedalDelayEngine::GetSlotNormalized(PedalSlot slot) const
{
    const std::size_t slotIndex = static_cast<std::size_t>(slot);
    if(slotIndex >= kPedalSlotCount)
    {
        return 0.0f;
    }
    return slotNormalized_[static_cast<std::size_t>(mode_)][slotIndex];
}

float PedalDelayEngine::GetSlotNative(PedalSlot slot) const
{
    return NormalizedToNative(mode_, slot, GetSlotNormalized(slot));
}

float PedalDelayEngine::NormalizedToNative(PedalDelayMode mode,
                                           PedalSlot      slot,
                                           float          normalizedValue) const
{
    const auto& descriptor = GetPedalSlotDescriptor(mode, slot);
    const float amount     = std::clamp(
        std::isfinite(normalizedValue) ? normalizedValue : 0.0f, 0.0f, 1.0f);
    if(std::string_view(descriptor.curve) == "log" && descriptor.minimum > 0.0f)
    {
        return ExponentialMapPrecise(
            descriptor.minimum, descriptor.maximum, amount);
    }
    return descriptor.minimum
           + amount * (descriptor.maximum - descriptor.minimum);
}

float PedalDelayEngine::NativeToNormalized(PedalDelayMode mode,
                                           PedalSlot      slot,
                                           float          nativeValue) const
{
    const auto& descriptor = GetPedalSlotDescriptor(mode, slot);
    const float value      = std::clamp(
        std::isfinite(nativeValue) ? nativeValue : descriptor.minimum,
        std::min(descriptor.minimum, descriptor.maximum),
        std::max(descriptor.minimum, descriptor.maximum));
    if(std::string_view(descriptor.curve) == "log" && descriptor.minimum > 0.0f)
    {
        const double ratio = static_cast<double>(value)
                             / static_cast<double>(descriptor.minimum);
        const double span = static_cast<double>(descriptor.maximum)
                            / static_cast<double>(descriptor.minimum);
        return static_cast<float>(
            std::clamp(std::log(ratio) / std::log(span), 0.0, 1.0));
    }
    const float span = descriptor.maximum - descriptor.minimum;
    return span != 0.0f ? std::clamp((value - descriptor.minimum) / span,
                                     0.0f,
                                     1.0f)
                        : 0.0f;
}

void PedalDelayEngine::SetBypass(bool bypassed)
{
    bypassed_ = bypassed;
}

void PedalDelayEngine::Tap(double nowMs)
{
    if(!std::isfinite(nowMs))
    {
        return;
    }
    const double intervalMs = nowMs - lastTapMs_;
    lastTapMs_              = nowMs;
    if(lastTapMs_ < 0.0 || intervalMs < kMinTapMs || intervalMs > kMaxTapMs)
    {
        return;
    }
    tapTempoMs_ = static_cast<float>(intervalMs);
    SetSlotNormalized(
        PedalSlot::kTime,
        NativeToNormalized(mode_, PedalSlot::kTime, tapTempoMs_));
}

void PedalDelayEngine::SetFreezeState(PedalFreezeState state)
{
    if(static_cast<std::size_t>(state)
       >= static_cast<std::size_t>(PedalFreezeState::kCount))
    {
        return;
    }
    if(state == PedalFreezeState::kReplace)
    {
        // Replace discards the previous capture completely: loop content and
        // the damping filter state that belongs to it.
        if(freezeLoop_ != nullptr)
        {
            std::fill(freezeLoop_,
                      freezeLoop_ + freezeSamples_ * kPedalChannelCount,
                      0.0f);
        }
        for(auto& channel : channels_)
        {
            channel.freezeLpf.Clear();
            channel.freezeHpf.Clear();
        }
    }
    if(state == PedalFreezeState::kCapture || state == PedalFreezeState::kReplace)
    {
        freezeCaptureCount_ = 0;
        freezeReadIndex_    = 0.0f;
    }
    freezeState_ = state;
}

void PedalDelayEngine::FreezeClear()
{
    if(freezeLoop_ != nullptr)
    {
        std::fill(freezeLoop_,
                  freezeLoop_ + freezeSamples_ * kPedalChannelCount,
                  0.0f);
    }
    for(auto& channel : channels_)
    {
        channel.freezeLpf.Clear();
        channel.freezeHpf.Clear();
    }
    freezeState_        = PedalFreezeState::kIdle;
    freezeLoopLength_   = 0;
    freezeCaptureCount_ = 0;
    freezeReadIndex_    = 0.0f;
}

void PedalDelayEngine::FreezeToggleHold()
{
    if(freezeState_ == PedalFreezeState::kIdle)
    {
        SetFreezeState(PedalFreezeState::kCapture);
        return;
    }
    SetFreezeState(PedalFreezeState::kIdle);
}

float PedalDelayEngine::GetAlgorithmicLatencySamples() const
{
    if(mode_ != PedalDelayMode::kRev)
    {
        return 0.0f;
    }
    return nativeSmoothed_[static_cast<std::size_t>(PedalSlot::kTime)] * 0.001f
           * static_cast<float>(sampleRate_);
}

float PedalDelayEngine::LowpassCoefficient(float cutoffHz) const
{
    const float clamped = std::clamp(cutoffHz,
                                     1.0f,
                                     static_cast<float>(sampleRate_) * 0.45f);
    const float coefficient
        = 1.0f
          - std::exp(-6.28318530717959f * clamped
                     / static_cast<float>(sampleRate_));
    return std::clamp(coefficient, 0.0f, 1.0f);
}

float PedalDelayEngine::SmoothedNative(PedalSlot slot) const
{
    return nativeSmoothed_[static_cast<std::size_t>(slot)];
}

void PedalDelayEngine::AdvanceSmoothing()
{
    // The state is the remaining error, not the running value. The textbook
    // form x += (t - x) * c stalls in float32 as soon as (t - x) * c drops
    // below the ULP of x, which leaves a systematic ~0.01% offset - about one
    // sample of delay time at 200 ms. Decaying the error instead keeps full
    // relative precision all the way down, and x = t - error reaches t
    // exactly. Same cost: one multiply and one subtract per slot.
    for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
    {
        nativeError_[slot] = FlushDenormal(
            nativeError_[slot] * (1.0f - smoothingCoefficient_[slot]));
        nativeSmoothed_[slot] = nativeTarget_[slot] - nativeError_[slot];
    }
}

void PedalDelayEngine::SnapSmoothingToTargets()
{
    for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
    {
        nativeTarget_[slot]
            = NormalizedToNative(mode_,
                                 static_cast<PedalSlot>(slot),
                                 GetSlotNormalized(static_cast<PedalSlot>(slot)));
        nativeSmoothed_[slot] = nativeTarget_[slot];
        nativeError_[slot]    = 0.0f;
    }
}

float PedalDelayEngine::HistoryRead(std::size_t channel,
                                    float       delaySamples) const
{
    if(history_ == nullptr || historySamples_ < 8)
    {
        return 0.0f;
    }
    const float bounded
        = std::clamp(std::isfinite(delaySamples) ? delaySamples : 1.0f,
                     1.0f,
                     static_cast<float>(historySamples_ - 3));
    const std::size_t whole = static_cast<std::size_t>(bounded);
    const float       frac  = bounded - static_cast<float>(whole);
    const float*      base  = history_ + channel * historySamples_;
    const std::size_t newest
        = (writeIndex_ + historySamples_ - whole) % historySamples_;
    const std::size_t older = (newest + historySamples_ - 1) % historySamples_;
    return base[newest] + (base[older] - base[newest]) * frac;
}

void PedalDelayEngine::HistoryWrite(std::size_t channel, float sample)
{
    if(history_ == nullptr || historySamples_ == 0)
    {
        return;
    }
    history_[channel * historySamples_ + writeIndex_] = Sanitize(sample);
}

float PedalDelayEngine::FreezeRead(std::size_t channel, float readIndex) const
{
    if(freezeLoop_ == nullptr || freezeLoopLength_ < 4)
    {
        return 0.0f;
    }
    const float length = static_cast<float>(freezeLoopLength_);
    float       wrapped
        = std::isfinite(readIndex) ? std::fmod(readIndex, length) : 0.0f;
    if(wrapped < 0.0f)
    {
        wrapped += length;
    }
    const std::size_t whole = static_cast<std::size_t>(wrapped)
                              % freezeLoopLength_;
    const float       frac = wrapped - std::floor(wrapped);
    const std::size_t next = (whole + 1) % freezeLoopLength_;
    const float*      base = freezeLoop_ + channel * freezeSamples_;
    return base[whole] + (base[next] - base[whole]) * frac;
}

void PedalDelayEngine::Process(const float* inputLeft,
                               const float* inputRight,
                               float*       outputLeft,
                               float*       outputRight,
                               std::size_t  frameCount)
{
    if(outputLeft == nullptr && outputRight == nullptr)
    {
        return;
    }

    // Coherent parameter snapshot at the block boundary. Everything below
    // reads nativeTarget_/nativeSmoothed_, never the UI-facing values.
    for(std::size_t slot = 0; slot < kPedalSlotCount; ++slot)
    {
        const float target
            = NormalizedToNative(mode_,
                                 static_cast<PedalSlot>(slot),
                                 GetSlotNormalized(static_cast<PedalSlot>(slot)));
        if(target != nativeTarget_[slot])
        {
            // Re-aim the smoother from wherever it currently is.
            nativeError_[slot]  = target - nativeSmoothed_[slot];
            nativeTarget_[slot] = target;
        }
    }

    const bool muteWet = bypassed_ && !trails_;

    for(std::size_t frame = 0; frame < frameCount; ++frame)
    {
        const float dryLeft
            = Sanitize(inputLeft != nullptr ? inputLeft[frame] : 0.0f);
        const float dryRight = Sanitize(
            inputRight != nullptr
                ? inputRight[frame]
                : (inputLeft != nullptr ? inputLeft[frame] : 0.0f));

        float wetLeft  = 0.0f;
        float wetRight = 0.0f;

        if(prepared_)
        {
            AdvanceSmoothing();

            // Bypass with trails: the wet path keeps running, only the new
            // input send is muted, so the existing tail is preserved.
            const float send      = bypassed_ ? 0.0f : 1.0f;
            const float sendLeft  = dryLeft * send;
            const float sendRight = dryRight * send;

            switch(mode_)
            {
                case PedalDelayMode::kDigi:
                    ProcessDigiOrTape(
                        sendLeft, sendRight, &wetLeft, &wetRight, false);
                    break;
                case PedalDelayMode::kTape:
                    ProcessDigiOrTape(
                        sendLeft, sendRight, &wetLeft, &wetRight, true);
                    break;
                case PedalDelayMode::kMod:
                    ProcessMod(sendLeft, sendRight, &wetLeft, &wetRight);
                    break;
                case PedalDelayMode::kRev:
                    ProcessRev(sendLeft, sendRight, &wetLeft, &wetRight);
                    break;
                case PedalDelayMode::kFreeze:
                    ProcessFreeze(sendLeft, sendRight, &wetLeft, &wetRight);
                    break;
                default: break;
            }

            writeIndex_ = (writeIndex_ + 1) % historySamples_;
        }

        if(muteWet)
        {
            wetLeft  = 0.0f;
            wetRight = 0.0f;
        }

        // MOD already carries its own blend/feedforward relationship, so the
        // output crossfade is bypassed for it.
        const float mixAmount
            = mode_ == PedalDelayMode::kMod
                  ? 1.0f
                  : SmoothedNative(PedalSlot::kMix);
        float outLeft  = Mix(dryLeft, wetLeft, mixAmount);
        float outRight = Mix(dryRight, wetRight, mixAmount);
        if(muteWet)
        {
            outLeft  = dryLeft;
            outRight = dryRight;
        }

        if(outputLeft != nullptr)
        {
            outputLeft[frame] = Sanitize(outLeft);
        }
        if(outputRight != nullptr)
        {
            outputRight[frame] = Sanitize(outRight);
        }
    }
}

void PedalDelayEngine::ProcessDigiOrTape(float  inputLeft,
                                         float  inputRight,
                                         float* outputLeft,
                                         float* outputRight,
                                         bool   tape)
{
    // The TIME smoother is the delay-time transition policy: a one-pole slew
    // of the read position (pitch warp), never an instantaneous pointer jump.
    const float delaySamples = SmoothedNative(PedalSlot::kTime) * 0.001f
                               * static_cast<float>(sampleRate_);
    const float feedbackGain
        = std::clamp(SmoothedNative(PedalSlot::kFeedback), 0.0f, 0.95f);

    float modulationSamples = 0.0f;
    float lpfCoefficient    = 1.0f;
    float hpfCoefficient    = 0.0f;
    float saturationDrive   = 0.0f;

    if(tape)
    {
        const float warble = std::clamp(SmoothedNative(PedalSlot::kMotion),
                                        0.0f,
                                        1.0f);
        const float age
            = std::clamp(SmoothedNative(PedalSlot::kColor), 0.0f, 1.0f);
        const float milliseconds
            = kTapeDriftDepthMs * warble
                  * driftLfo_.Sine(kTapeDriftRateHz,
                                   static_cast<float>(sampleRate_))
              + kTapeWowDepthMs * warble
                    * wowLfo_.Sine(kTapeWowRateHz,
                                   static_cast<float>(sampleRate_))
              + kTapeFlutterDepthMs * warble
                    * flutterLfo_.Sine(kTapeFlutterRateHz,
                                       static_cast<float>(sampleRate_));
        modulationSamples
            = milliseconds * 0.001f * static_cast<float>(sampleRate_);
        lpfCoefficient  = LowpassCoefficient(ExponentialMap(9000.0f, 1600.0f, age));
        hpfCoefficient  = LowpassCoefficient(ExponentialMap(40.0f, 120.0f, age));
        saturationDrive = 0.05f + 0.75f * age;
    }
    else
    {
        const float depthMs
            = std::max(SmoothedNative(PedalSlot::kMotion), 0.0f);
        modulationSamples = depthMs
                            * driftLfo_.Sine(kDigiDriftRateHz,
                                             static_cast<float>(sampleRate_))
                            * 0.001f * static_cast<float>(sampleRate_);
        lpfCoefficient = LowpassCoefficient(SmoothedNative(PedalSlot::kColor));
    }

    const float inputs[kPedalChannelCount]  = {inputLeft, inputRight};
    float*      outputs[kPedalChannelCount] = {outputLeft, outputRight};

    for(std::size_t channel = 0; channel < kPedalChannelCount; ++channel)
    {
        auto&       state = channels_[channel];
        const float read
            = HistoryRead(channel, delaySamples + modulationSamples);

        float conditioned = state.feedbackLpf.Lowpass(read, lpfCoefficient);
        if(tape)
        {
            conditioned = state.ageHpf.Highpass(conditioned, hpfCoefficient);
            conditioned = SoftSaturate(conditioned, saturationDrive);
        }
        HistoryWrite(channel, inputs[channel] + conditioned * feedbackGain);

        if(outputs[channel] != nullptr)
        {
            *outputs[channel] = read;
        }
    }
}

void PedalDelayEngine::ProcessMod(float  inputLeft,
                                  float  inputRight,
                                  float* outputLeft,
                                  float* outputRight)
{
    const float baseSamples = SmoothedNative(PedalSlot::kTime) * 0.001f
                              * static_cast<float>(sampleRate_);
    const float depthSamples = std::max(SmoothedNative(PedalSlot::kColor), 0.0f)
                               * 0.001f * static_cast<float>(sampleRate_);
    const float feedbackCoefficient
        = std::clamp(SmoothedNative(PedalSlot::kFeedback), -0.95f, 0.95f);
    const float blend
        = std::clamp(SmoothedNative(PedalSlot::kMix), 0.0f, 1.0f);
    const float lfo = modLfo_.Sine(
        std::max(SmoothedNative(PedalSlot::kMotion), 0.0f),
        static_cast<float>(sampleRate_));

    // Every read position stays inside the attached history.
    const float delaySamples
        = std::clamp(baseSamples + depthSamples * lfo,
                     1.0f,
                     static_cast<float>(historySamples_ - 3));

    const float inputs[kPedalChannelCount]  = {inputLeft, inputRight};
    float*      outputs[kPedalChannelCount] = {outputLeft, outputRight};

    for(std::size_t channel = 0; channel < kPedalChannelCount; ++channel)
    {
        // Universal comb (DAFX): xh[n] = x[n] + FB * xh[n-M],
        //                        y[n]  = FF * xh[n-M] + BL * xh[n]
        const float delayed = HistoryRead(channel, delaySamples);
        const float xh      = inputs[channel] + feedbackCoefficient * delayed;
        HistoryWrite(channel, xh);
        if(outputs[channel] != nullptr)
        {
            *outputs[channel] = blend * delayed + (1.0f - blend) * xh;
        }
    }
}

void PedalDelayEngine::ProcessRev(float  inputLeft,
                                  float  inputRight,
                                  float* outputLeft,
                                  float* outputRight)
{
    // A reverse read head walks away from the write head at twice the write
    // rate, so the absolute read position moves backwards at 1x speed. Two
    // grains half a period apart are summed and divided by the summed window,
    // which keeps the overlap-add gain bounded for any taper.
    const float maxGrain
        = std::max(32.0f, (static_cast<float>(historySamples_) - 8.0f) * 0.5f);
    const float grainSamples
        = std::clamp(SmoothedNative(PedalSlot::kTime) * 0.001f
                         * static_cast<float>(sampleRate_),
                     32.0f,
                     maxGrain);
    const float taper
        = 0.05f + 0.45f * std::clamp(SmoothedNative(PedalSlot::kMotion),
                                     0.0f,
                                     1.0f);
    const float reverseAmount
        = std::clamp(SmoothedNative(PedalSlot::kColor), 0.0f, 1.0f);
    const float reinjectionGain
        = std::clamp(SmoothedNative(PedalSlot::kFeedback), 0.0f, 0.85f);

    revGrainPhase_ = Fract(revGrainPhase_ + 1.0f / grainSamples);

    const float phases[2]
        = {revGrainPhase_, Fract(revGrainPhase_ + 0.5f)};
    float windows[2] = {TukeyWindow(phases[0], taper),
                        TukeyWindow(phases[1], taper)};
    const float windowSum = std::max(windows[0] + windows[1], 1.0e-6f);

    const float inputs[kPedalChannelCount]  = {inputLeft, inputRight};
    float*      outputs[kPedalChannelCount] = {outputLeft, outputRight};

    for(std::size_t channel = 0; channel < kPedalChannelCount; ++channel)
    {
        float reversed = 0.0f;
        for(std::size_t grain = 0; grain < 2; ++grain)
        {
            const float delay = 2.0f + 2.0f * phases[grain] * grainSamples;
            reversed += windows[grain] * HistoryRead(channel, delay);
        }
        reversed /= windowSum;

        const float forward  = HistoryRead(channel, grainSamples);
        const float branch   = Mix(forward, reversed, reverseAmount);
        HistoryWrite(channel, inputs[channel] + branch * reinjectionGain);
        if(outputs[channel] != nullptr)
        {
            *outputs[channel] = branch;
        }
    }
}

void PedalDelayEngine::ProcessFreeze(float  inputLeft,
                                     float  inputRight,
                                     float* outputLeft,
                                     float* outputRight)
{
    const std::size_t requestedLength = static_cast<std::size_t>(std::clamp(
        SmoothedNative(PedalSlot::kTime) * 0.001f
            * static_cast<float>(sampleRate_),
        64.0f,
        static_cast<float>(freezeSamples_ - 4)));
    const float decay
        = std::clamp(SmoothedNative(PedalSlot::kFeedback), 0.0f, 0.999f);
    const float damping
        = std::clamp(SmoothedNative(PedalSlot::kColor), 0.0f, 1.0f);
    const float evolve
        = std::clamp(SmoothedNative(PedalSlot::kMotion), 0.0f, 1.0f);
    const float lpfCoefficient
        = LowpassCoefficient(ExponentialMap(12000.0f, 800.0f, damping));
    const float hpfCoefficient
        = LowpassCoefficient(ExponentialMap(20.0f, 300.0f, damping));
    const float driftSamples
        = 6.0f * evolve
          * evolveLfo_.Sine(0.03f + 0.17f * evolve,
                            static_cast<float>(sampleRate_))
          * 0.001f * static_cast<float>(sampleRate_);

    const float inputs[kPedalChannelCount]  = {inputLeft, inputRight};
    float*      outputs[kPedalChannelCount] = {outputLeft, outputRight};

    // The history keeps running so switching back to a delay mode still has a
    // tail, but FREEZE itself never reads it.
    for(std::size_t channel = 0; channel < kPedalChannelCount; ++channel)
    {
        HistoryWrite(channel, inputs[channel]);
        if(outputs[channel] != nullptr)
        {
            *outputs[channel] = 0.0f;
        }
    }

    if(freezeState_ == PedalFreezeState::kCapture
       || freezeState_ == PedalFreezeState::kReplace)
    {
        for(std::size_t channel = 0; channel < kPedalChannelCount; ++channel)
        {
            freezeLoop_[channel * freezeSamples_ + freezeCaptureCount_]
                = inputs[channel];
        }
        ++freezeCaptureCount_;
        if(freezeCaptureCount_ >= requestedLength)
        {
            freezeLoopLength_   = requestedLength;
            freezeCaptureCount_ = 0;
            freezeReadIndex_    = 0.0f;
            freezeState_        = PedalFreezeState::kHold;
        }
        return;
    }

    if(freezeState_ == PedalFreezeState::kIdle || freezeLoopLength_ < 4)
    {
        return;
    }

    // LOOP can be shortened after capture; the read index simply re-wraps.
    if(requestedLength < freezeLoopLength_)
    {
        freezeLoopLength_ = requestedLength;
    }

    const std::size_t writeSlot = static_cast<std::size_t>(freezeReadIndex_)
                                  % freezeLoopLength_;
    const bool admitsInput = freezeState_ == PedalFreezeState::kAccumulate;

    for(std::size_t channel = 0; channel < kPedalChannelCount; ++channel)
    {
        auto&       state = channels_[channel];
        const float read
            = FreezeRead(channel, freezeReadIndex_ + driftSamples);
        float damped = state.freezeLpf.Lowpass(read, lpfCoefficient);
        damped       = state.freezeHpf.Highpass(damped, hpfCoefficient);

        const float injected
            = admitsInput ? inputs[channel] * kFreezeInjectionGain : 0.0f;
        // DECAY reaches 0.999 and ACCUMULATE keeps adding fresh material, so
        // the loop write-back goes through an explicit limiter. tanh is unity
        // for small signals and hard-bounds the stored loop to +/-1.
        freezeLoop_[channel * freezeSamples_ + writeSlot]
            = Sanitize(std::tanh(damped * decay + injected));

        if(outputs[channel] != nullptr)
        {
            *outputs[channel] = damped;
        }
    }

    freezeReadIndex_ += 1.0f;
    if(freezeReadIndex_ >= static_cast<float>(freezeLoopLength_))
    {
        freezeReadIndex_ -= static_cast<float>(freezeLoopLength_);
    }
}
} // namespace daisyhost
