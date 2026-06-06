#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace daisyhost
{
enum class DaisyDelayFxSource
{
    kMultiFxPedal,
    kReverbPlayground,
    kFunBox,
    kSdramDelaylines,
};

struct DaisyDelayFxProfile
{
    DaisyDelayFxSource source;
    const char*        appId;
    const char*        displayName;
    const char*        sourceRepo;
    const char*        sourceSummary;
};

struct DaisyDelayFxAlgorithmDescriptor
{
    DaisyDelayFxSource source;
    const char*        label;
    const char*        shortLabel;
    const char*        statePrefix;
};

const std::array<DaisyDelayFxAlgorithmDescriptor, 4>&
GetDaisyDelayFxAlgorithmDescriptors();
const DaisyDelayFxAlgorithmDescriptor& GetDaisyDelayFxAlgorithmDescriptor(
    DaisyDelayFxSource source);
DaisyDelayFxSource DaisyDelayFxSourceForAlgorithmIndex(std::size_t index);
std::size_t DaisyDelayFxAlgorithmIndex(DaisyDelayFxSource source);

struct DaisyDelayFxParameter
{
    std::string id;
    std::string label;
    std::string unitLabel;
    float       normalizedValue = 0.0f;
    float       defaultNormalizedValue = 0.0f;
    float       effectiveNormalizedValue = 0.0f;
    float       nativeMinimum = 0.0f;
    float       nativeMaximum = 1.0f;
    float       nativeDefault = 0.0f;
    int         nativePrecision = 0;
    int         stepCount = 0;
    int         importanceRank = 0;
    int         layer = 0;
    int         knob = 0;
    bool        automatable = true;
    bool        stateful = true;
    bool        menuEditable = true;
};

class DaisyDelayFxCore
{
  public:
    static constexpr std::size_t kPreferredBlockSize = 48;
    static constexpr std::size_t kDelayLineCount = 4;
    static constexpr std::size_t kMaxDelaySamples = 384000;
    static constexpr std::size_t kLayerCount = 3;
    static constexpr std::size_t kKnobCount = 8;
    static constexpr std::size_t kFieldKeyCount = 16;

    explicit DaisyDelayFxCore(
        DaisyDelayFxSource source = DaisyDelayFxSource::kMultiFxPedal);

    void SetSource(DaisyDelayFxSource source);
    DaisyDelayFxSource GetSource() const;
    const DaisyDelayFxProfile& GetProfile() const;
    void SetBundleMode(bool enabled);

    void AttachDelayStorage(float* storage,
                            std::size_t lineCount,
                            std::size_t samplesPerLine);
    void Prepare(double sampleRate, std::size_t maxBlockSize);
    void ResetToDefaultState(std::uint32_t seed = 0);

    void Process(const float* inputLeft,
                 const float* inputRight,
                 float*       outputLeft,
                 float*       outputRight,
                 std::size_t  frameCount);

    bool SetParameterValue(const std::string& parameterId,
                           float              normalizedValue);
    bool SetParameterValue(const char* parameterId, float normalizedValue);
    bool GetParameterValue(const std::string& parameterId,
                           float*             normalizedValue) const;
    bool GetParameterValue(const char* parameterId,
                           float*      normalizedValue) const;
    bool SetEffectiveParameterValue(const std::string& parameterId,
                                    float normalizedValue);
    bool SetEffectiveParameterValue(const char* parameterId,
                                    float       normalizedValue);
    bool GetEffectiveParameterValue(const std::string& parameterId,
                                    float*             normalizedValue) const;
    bool GetEffectiveParameterValue(const char* parameterId,
                                    float*      normalizedValue) const;
    void ClearEffectiveParameterOverrides();

    const std::vector<DaisyDelayFxParameter>& GetParameters() const;
    const DaisyDelayFxParameter* FindParameter(
        const std::string& parameterId) const;
    const DaisyDelayFxParameter* FindParameter(const char* parameterId) const;
    const char* GetParameterForLayerKnob(std::size_t layer,
                                         std::size_t knob) const;
    std::string FormatParameterValue(const std::string& parameterId) const;
    void FormatParameterValue(const char* parameterId,
                              char*       destination,
                              std::size_t destinationSize) const;

    bool TriggerMomentaryAction(const std::string& actionId);
    bool TriggerFieldKeyAction(std::size_t zeroBasedIndex, bool pressed);
    void SetButtonState(std::size_t zeroBasedIndex, int state);
    int GetButtonState(std::size_t zeroBasedIndex) const;
    void SetInternalSynthMode(int mode);
    int GetInternalSynthMode() const;
    void SetInternalSynthHoldMode(int mode);
    int GetInternalSynthHoldMode() const;
    std::array<float, kFieldKeyCount> GetFieldKeyLedValues() const;

    void HandleMidiEvent(std::uint8_t status,
                         std::uint8_t data1,
                         std::uint8_t data2);
    int GetKeyboardOctaveOffset() const;

    std::unordered_map<std::string, float> CaptureStatefulParameterValues()
        const;
    void RestoreStatefulParameterValues(
        const std::unordered_map<std::string, float>& values);

  private:
    struct DelayLine
    {
        float*      buffer = nullptr;
        std::size_t size = 0;
        std::size_t writeIndex = 0;

        void Init(float* externalBuffer, std::size_t externalSize);
        void Clear();
        void Write(float sample);
        float Read(float delaySamples) const;
    };

    struct SynthVoice
    {
        int   note = -1;
        bool  active = false;
        bool  held = false;
        float frequency = 261.625565f;
        float velocity = 0.0f;
        float phase = 0.0f;
        float envelope = 0.0f;
        float body = 0.0f;
        float exciter = 0.0f;
    };

    void RebuildParameters();
    void UpdateParameterCache();
    const DaisyDelayFxParameter* FindParameterById(const char* id) const;
    float ParameterValue(const char* id) const;
    float EffectiveParameterValue(const char* id) const;
    float NativeValue(const char* id) const;
    float ParameterValueAt(std::size_t index) const;
    float EffectiveParameterValueAt(std::size_t index) const;
    float NativeValueAt(std::size_t index) const;
    float NativeValue(const DaisyDelayFxParameter& parameter) const;
    std::size_t ParameterIndex(const std::string& parameterId) const;
    std::size_t ParameterIndexById(const char* parameterId) const;

    void StartSynthVoice(int note, float velocity);
    void ReleaseSynthVoice(int note);
    void ReleaseAllSynthVoices(bool immediate);
    SynthVoice* FindVoiceForNote(int note);
    SynthVoice* AllocateVoice();
    float ProcessInternalSynth(float brightness,
                               float decay,
                               float level,
                               float attackMs,
                               float releaseMs);
    void ProcessMultiFx(float inputLeft,
                        float inputRight,
                        float* outputLeft,
                        float* outputRight);
    void ProcessReverbPlayground(float inputLeft,
                                 float inputRight,
                                 float* outputLeft,
                                 float* outputRight);
    void ProcessFunBox(float inputLeft,
                       float inputRight,
                       float* outputLeft,
                       float* outputRight);
    void ProcessSdramDelaylines(float inputLeft,
                                float inputRight,
                                float* outputLeft,
                                float* outputRight);

    DaisyDelayFxSource source_;
    double             sampleRate_ = 48000.0;
    std::size_t        maxBlockSize_ = kPreferredBlockSize;
    bool               prepared_ = false;
    bool               bundleMode_ = false;

    std::array<DelayLine, kDelayLineCount> delays_;
    std::vector<DaisyDelayFxParameter> parameters_;
    std::array<std::array<std::string, kKnobCount>, kLayerCount>
        layerKnobParameterIds_{};

    std::array<int, 8>  buttonStates_{};
    std::array<bool, 128> noteActive_{};
    static constexpr std::size_t kSynthVoiceCount = 8;
    std::array<SynthVoice, kSynthVoiceCount> synthVoices_{};
    int                 internalSynthMode_ = 1;
    int                 internalSynthHoldMode_ = 0;
    int                 activeMidiNote_ = 60;
    float               activeMidiFrequencyHz_ = 261.625565f;
    int                 keyboardOctaveOffset_ = 0;
    float               notePhase_ = 0.0f;
    float               noteEnvelope_ = 0.0f;
    float               lfoPhase_ = 0.0f;
    float               slowLfoPhase_ = 0.0f;
    float               delaySmooth_[4] = {};
    float               dampingState_[4] = {};
    float               toneState_[2] = {};
    float               rngState_ = 0.37f;
    std::uint32_t       seed_ = 0;
};
} // namespace daisyhost
