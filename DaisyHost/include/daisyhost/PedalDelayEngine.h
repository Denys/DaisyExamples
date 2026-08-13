#pragma once

// Portable multi-delay pedal engine.
//
// Five modes (DIGI / TAPE / MOD / REV / FREEZE), five performance slots
// (TIME / FEEDBACK / MIX / COLOR / MOTION), one active mode at a time.
//
// Real-time contract, mirrored from DaisyDelayFxCore:
//   * no allocation, no locks, no I/O in Process();
//   * all sample memory is attached externally before audio starts;
//   * slot values are snapshotted once per block and smoothed per sample;
//   * every read index is clamped inside the attached history;
//   * non-finite parameter values are rejected, non-finite audio is zeroed.
//
// Host memory is plain RAM here. On target the same pointers are expected to
// come from SDRAM; the engine never assumes where the storage lives.

#include <array>
#include <cstddef>
#include <cstdint>

namespace daisyhost
{
enum class PedalDelayMode : std::size_t
{
    kDigi = 0,
    kTape,
    kMod,
    kRev,
    kFreeze,
    kCount,
};

enum class PedalSlot : std::size_t
{
    kTime = 0,
    kFeedback,
    kMix,
    kColor,
    kMotion,
    kCount,
};

// Deterministic state operations for the FREEZE layer. The performance UI maps
// these onto TAP/HOLD; the host/debug UI drives them explicitly.
enum class PedalFreezeState : std::size_t
{
    kIdle = 0,
    kCapture,
    kHold,
    kAccumulate,
    kReplace,
    kCount,
};

static constexpr std::size_t kPedalDelayModeCount
    = static_cast<std::size_t>(PedalDelayMode::kCount);
static constexpr std::size_t kPedalSlotCount
    = static_cast<std::size_t>(PedalSlot::kCount);
static constexpr std::size_t kPedalChannelCount  = 2;
static constexpr std::size_t kPedalMaxDspTargets = 4;

// One row of the product/UI contract: musician label plus the engineering
// meaning it actually writes.
struct PedalSlotDescriptor
{
    const char*                                  slotId           = "";
    const char*                                  musicianLabel    = "";
    const char*                                  engineeringLabel = "";
    const char*                                  unit             = "";
    const char*                                  curve            = "linear";
    float                                        minimum          = 0.0f;
    float                                        maximum          = 1.0f;
    float                                        defaultValue     = 0.0f;
    float                                        smoothingMs      = 0.0f;
    const char*                                  clockwiseMeaning = "";
    const char*                                  audibleEffect    = "";
    bool                                         isMacro          = false;
    bool                                         provisional      = true;
    std::array<const char*, kPedalMaxDspTargets> dspTargets{};
};

const char*                PedalDelayModeName(PedalDelayMode mode);
const char*                PedalFreezeStateName(PedalFreezeState state);
const PedalSlotDescriptor& GetPedalSlotDescriptor(PedalDelayMode mode,
                                                  PedalSlot      slot);

class PedalDelayEngine
{
  public:
    // History is sized for the longest musical delay plus the reverse-grain
    // read span (2 x grain). 2.5 s covers TIME up to 2 s and SLICE up to 1.2 s.
    static constexpr float kHistorySeconds    = 2.5f;
    static constexpr float kFreezeLoopSeconds = 2.0f;

    PedalDelayEngine();

    static std::size_t HistorySamplesForRate(double sampleRate);
    static std::size_t FreezeSamplesForRate(double sampleRate);

    // `history` and `freezeLoop` must each hold
    // kPedalChannelCount * samplesPerChannel floats and outlive the engine.
    void AttachStorage(float*      history,
                       std::size_t historySamplesPerChannel,
                       float*      freezeLoop,
                       std::size_t freezeSamplesPerChannel);
    void Prepare(double sampleRate, std::size_t maxBlockSize);
    // Clears audio state (history, freeze loop, filters, LFOs). Leaves slot
    // values alone so a sample-rate change does not discard the patch.
    void Reset();
    // Restores every mode's five slots to their descriptor defaults.
    void ResetParameters();

    void           SetMode(PedalDelayMode mode);
    PedalDelayMode GetMode() const { return mode_; }

    // Returns false (and keeps the previous value) for non-finite input.
    bool  SetSlotNormalized(PedalSlot slot, float normalizedValue);
    float GetSlotNormalized(PedalSlot slot) const;
    // Engineering value in the descriptor's unit.
    float GetSlotNative(PedalSlot slot) const;
    float NormalizedToNative(PedalDelayMode mode,
                             PedalSlot      slot,
                             float          normalizedValue) const;
    float NativeToNormalized(PedalDelayMode mode,
                             PedalSlot      slot,
                             float          nativeValue) const;

    void SetBypass(bool bypassed);
    bool GetBypass() const { return bypassed_; }
    // Trails on: bypass mutes the new input send but keeps the wet tail alive.
    void SetTrails(bool enabled) { trails_ = enabled; }
    bool GetTrails() const { return trails_; }

    // Tap tempo. `nowMs` is a monotonic host clock. Two taps 100..2000 ms
    // apart rewrite the TIME slot; anything else only restarts the measurement.
    void  Tap(double nowMs);
    float GetTapTempoMs() const { return tapTempoMs_; }

    void             SetFreezeState(PedalFreezeState state);
    PedalFreezeState GetFreezeState() const { return freezeState_; }
    void             FreezeClear();
    // TAP/HOLD mapping used by the performance UI.
    void FreezeToggleHold();

    void Process(const float* inputLeft,
                 const float* inputRight,
                 float*       outputLeft,
                 float*       outputRight,
                 std::size_t  frameCount);

    // Reported REV algorithmic latency (one grain), 0 for the other modes.
    float GetAlgorithmicLatencySamples() const;

  private:
    struct OnePole
    {
        float state = 0.0f;
        float Lowpass(float input, float coefficient);
        float Highpass(float input, float coefficient);
        void  Clear() { state = 0.0f; }
    };

    struct Lfo
    {
        float phase = 0.0f;
        float Sine(float rateHz, float sampleRate);
        void  Clear() { phase = 0.0f; }
    };

    struct ChannelState
    {
        OnePole feedbackLpf;
        OnePole ageHpf;
        OnePole freezeLpf;
        OnePole freezeHpf;
    };

    float HistoryRead(std::size_t channel, float delaySamples) const;
    void  HistoryWrite(std::size_t channel, float sample);
    float FreezeRead(std::size_t channel, float readIndex) const;

    void ProcessDigiOrTape(float  inputLeft,
                           float  inputRight,
                           float* outputLeft,
                           float* outputRight,
                           bool   tape);
    void ProcessMod(float  inputLeft,
                    float  inputRight,
                    float* outputLeft,
                    float* outputRight);
    void ProcessRev(float  inputLeft,
                    float  inputRight,
                    float* outputLeft,
                    float* outputRight);
    void ProcessFreeze(float  inputLeft,
                       float  inputRight,
                       float* outputLeft,
                       float* outputRight);

    float SmoothedNative(PedalSlot slot) const;
    void  AdvanceSmoothing();
    void  SnapSmoothingToTargets();
    float LowpassCoefficient(float cutoffHz) const;

    float*      history_        = nullptr;
    std::size_t historySamples_ = 0;
    float*      freezeLoop_     = nullptr;
    std::size_t freezeSamples_  = 0;
    std::size_t writeIndex_     = 0;

    double      sampleRate_   = 48000.0;
    std::size_t maxBlockSize_ = 48;

    PedalDelayMode mode_ = PedalDelayMode::kDigi;
    std::array<std::array<float, kPedalSlotCount>, kPedalDelayModeCount>
        slotNormalized_{};
    // Native (engineering-unit) targets snapshotted at block start, plus the
    // per-sample smoothed values actually handed to the DSP.
    std::array<float, kPedalSlotCount> nativeTarget_{};
    std::array<float, kPedalSlotCount> nativeSmoothed_{};
    // Remaining smoothing error; see AdvanceSmoothing().
    std::array<float, kPedalSlotCount> nativeError_{};
    std::array<float, kPedalSlotCount> smoothingCoefficient_{};

    std::array<ChannelState, kPedalChannelCount> channels_{};
    Lfo                                          modLfo_;
    Lfo                                          driftLfo_;
    Lfo                                          wowLfo_;
    Lfo                                          flutterLfo_;
    Lfo                                          evolveLfo_;
    float                                        revGrainPhase_ = 0.0f;

    PedalFreezeState freezeState_        = PedalFreezeState::kIdle;
    std::size_t      freezeLoopLength_   = 0;
    std::size_t      freezeCaptureCount_ = 0;
    float            freezeReadIndex_    = 0.0f;

    bool   bypassed_   = false;
    bool   trails_     = true;
    double lastTapMs_  = -1.0;
    float  tapTempoMs_ = 0.0f;
    bool   prepared_   = false;
};
} // namespace daisyhost
