#include "daisyhost/DaisyDelayFxCore.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace daisyhost
{
namespace
{
constexpr float kPi = 3.14159265358979323846f;

float Clamp(float value, float minValue, float maxValue)
{
    return value < minValue ? minValue : (value > maxValue ? maxValue : value);
}

float Clamp01(float value)
{
    return Clamp(value, 0.0f, 1.0f);
}

float Mix(float a, float b, float amount)
{
    return a + (b - a) * Clamp01(amount);
}

float DbToLinear(float db)
{
    return std::pow(10.0f, db / 20.0f);
}

float MidiNoteToHz(int note)
{
    return 440.0f * std::pow(2.0f, (static_cast<float>(note) - 69.0f) / 12.0f);
}

float FastTanh(float value)
{
    const float x = Clamp(value, -4.0f, 4.0f);
    return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
}

float OnePoleCoeff(float hz, float sampleRate)
{
    const float clampedHz = Clamp(hz, 5.0f, sampleRate * 0.45f);
    return Clamp(1.0f - std::exp((-2.0f * kPi * clampedHz) / sampleRate),
                 0.0001f,
                 1.0f);
}

const std::array<DaisyDelayFxProfile, 4> kProfiles = {{
    {DaisyDelayFxSource::kMultiFxPedal,
     "field_delay_multifx_pedal",
     "Field Delay MultiFX Pedal",
     "balazsbencs/daisy-multifx-pedal",
     "Tape-style SDRAM delay with feedback color, modulation, grit, and "
     "product-style mode controls."},
    {DaisyDelayFxSource::kReverbPlayground,
     "field_delay_reverb_playground",
     "Field Delay Reverb Playground",
     "Farmer2K5/daisy-reverb-playground",
     "Early reflection, diffusion, and damped FDN/tank behavior adapted into "
     "a Field-safe delay network."},
    {DaisyDelayFxSource::kFunBox,
     "field_delay_funbox",
     "Field Delay FunBox",
     "GuitarML/FunBox",
     "FunBox-inspired Mars/Saturn/Uranus/Pluto family: two-tap delay, spectral "
     "smear, granular motion, freeze, reverse, and looper-style hold."},
    {DaisyDelayFxSource::kSdramDelaylines,
     "field_delay_sdram_delaylines",
     "Field Delay SDRAM Delaylines",
     "Farmer2K5/daisy-sdram-delaylines",
     "Focused external-buffer delay-line primitive with long fractional stereo "
     "delay and ping-pong feedback."},
}};

const std::array<DaisyDelayFxAlgorithmDescriptor, 4> kAlgorithmDescriptors = {{
    {DaisyDelayFxSource::kMultiFxPedal, "Tape [multifx]", "Tape", "tape"},
    {DaisyDelayFxSource::kReverbPlayground, "Tank [reverb]", "Tank", "tank"},
    {DaisyDelayFxSource::kFunBox, "Texture [FunBox]", "Texture", "texture"},
    {DaisyDelayFxSource::kSdramDelaylines, "Long [sdram]", "Long", "long"},
}};

const DaisyDelayFxProfile& ProfileForSource(DaisyDelayFxSource source)
{
    for(const auto& profile : kProfiles)
    {
        if(profile.source == source)
        {
            return profile;
        }
    }
    return kProfiles.front();
}

int QuantizedState(float normalized, int stepCount)
{
    if(stepCount <= 1)
    {
        return 0;
    }
    return static_cast<int>(
        std::round(Clamp01(normalized) * static_cast<float>(stepCount - 1)));
}

enum ParameterSlot : std::size_t
{
    kParamMix = 0,
    kParamTime,
    kParamFeedback,
    kParamTone,
    kParamTexture,
    kParamMod,
    kParamDrive,
    kParamOutput,
    kParamPreDelay,
    kParamWidth,
    kParamDiffusion,
    kParamDamping,
    kParamTapRatio,
    kParamFreeze,
    kParamMidiLevel,
    kParamTempo,
    kParamSize,
    kParamDensity,
    kParamLowCut,
    kParamHighCut,
    kParamSmear,
    kParamWarp,
    kParamAttack,
    kParamRelease,
};
} // namespace

const std::array<DaisyDelayFxAlgorithmDescriptor, 4>&
GetDaisyDelayFxAlgorithmDescriptors()
{
    return kAlgorithmDescriptors;
}

const DaisyDelayFxAlgorithmDescriptor& GetDaisyDelayFxAlgorithmDescriptor(
    DaisyDelayFxSource source)
{
    for(const auto& descriptor : kAlgorithmDescriptors)
    {
        if(descriptor.source == source)
        {
            return descriptor;
        }
    }
    return kAlgorithmDescriptors.front();
}

DaisyDelayFxSource DaisyDelayFxSourceForAlgorithmIndex(std::size_t index)
{
    if(index >= kAlgorithmDescriptors.size())
    {
        index = kAlgorithmDescriptors.size() - 1;
    }
    return kAlgorithmDescriptors[index].source;
}

std::size_t DaisyDelayFxAlgorithmIndex(DaisyDelayFxSource source)
{
    for(std::size_t i = 0; i < kAlgorithmDescriptors.size(); ++i)
    {
        if(kAlgorithmDescriptors[i].source == source)
        {
            return i;
        }
    }
    return 0;
}

void DaisyDelayFxCore::DelayLine::Init(float* externalBuffer,
                                       std::size_t externalSize)
{
    buffer = externalBuffer;
    size = externalSize;
    writeIndex = 0;
    Clear();
}

void DaisyDelayFxCore::DelayLine::Clear()
{
    if(buffer == nullptr)
    {
        return;
    }
    for(std::size_t i = 0; i < size; ++i)
    {
        buffer[i] = 0.0f;
    }
    writeIndex = 0;
}

void DaisyDelayFxCore::DelayLine::Write(float sample)
{
    if(buffer == nullptr || size == 0)
    {
        return;
    }
    buffer[writeIndex] = sample;
    writeIndex = (writeIndex + 1) % size;
}

float DaisyDelayFxCore::DelayLine::Read(float delaySamples) const
{
    if(buffer == nullptr || size < 4)
    {
        return 0.0f;
    }

    const float boundedDelay = Clamp(delaySamples, 1.0f, static_cast<float>(size - 3));
    const int   wholeDelay = static_cast<int>(boundedDelay);
    const float frac = boundedDelay - static_cast<float>(wholeDelay);
    const std::size_t newest
        = (writeIndex + size - static_cast<std::size_t>(wholeDelay)) % size;
    const std::size_t older = (newest + size - 1) % size;
    return Mix(buffer[newest], buffer[older], frac);
}

DaisyDelayFxCore::DaisyDelayFxCore(DaisyDelayFxSource source)
: source_(source)
{
    RebuildParameters();
}

void DaisyDelayFxCore::SetSource(DaisyDelayFxSource source)
{
    if(source_ == source)
    {
        return;
    }
    source_ = source;
    RebuildParameters();
    ResetToDefaultState(seed_);
}

DaisyDelayFxSource DaisyDelayFxCore::GetSource() const
{
    return source_;
}

const DaisyDelayFxProfile& DaisyDelayFxCore::GetProfile() const
{
    return ProfileForSource(source_);
}

void DaisyDelayFxCore::SetBundleMode(bool enabled)
{
    if(bundleMode_ == enabled)
    {
        return;
    }
    bundleMode_ = enabled;
    RebuildParameters();
    ResetToDefaultState(seed_);
}

void DaisyDelayFxCore::AttachDelayStorage(float* storage,
                                          std::size_t lineCount,
                                          std::size_t samplesPerLine)
{
    for(std::size_t i = 0; i < delays_.size(); ++i)
    {
        float* line = (storage != nullptr && i < lineCount)
                          ? storage + (i * samplesPerLine)
                          : nullptr;
        delays_[i].Init(line, line != nullptr ? samplesPerLine : 0);
    }
}

void DaisyDelayFxCore::Prepare(double sampleRate, std::size_t maxBlockSize)
{
    sampleRate_ = sampleRate > 1000.0 ? sampleRate : 48000.0;
    maxBlockSize_ = maxBlockSize > 0 ? maxBlockSize : kPreferredBlockSize;
    prepared_ = true;
    ResetToDefaultState(seed_);
}

void DaisyDelayFxCore::ResetToDefaultState(std::uint32_t seed)
{
    seed_ = seed;
    for(auto& parameter : parameters_)
    {
        parameter.normalizedValue = parameter.defaultNormalizedValue;
        parameter.effectiveNormalizedValue = parameter.defaultNormalizedValue;
    }
    for(auto& delay : delays_)
    {
        delay.Clear();
    }
    buttonStates_.fill(0);
    noteActive_.fill(false);
    synthVoices_ = {};
    internalSynthMode_ = 1;
    internalSynthHoldMode_ = 0;
    activeMidiNote_ = 60;
    activeMidiFrequencyHz_ = MidiNoteToHz(activeMidiNote_);
    keyboardOctaveOffset_ = 0;
    notePhase_ = 0.0f;
    noteEnvelope_ = 0.0f;
    lfoPhase_ = 0.0f;
    slowLfoPhase_ = 0.0f;
    for(float& value : delaySmooth_)
    {
        value = 2400.0f;
    }
    for(float& value : dampingState_)
    {
        value = 0.0f;
    }
    toneState_[0] = 0.0f;
    toneState_[1] = 0.0f;
    rngState_ = 0.37f + static_cast<float>((seed % 97u)) * 0.001f;
    UpdateParameterCache();
}

void DaisyDelayFxCore::Process(const float* inputLeft,
                               const float* inputRight,
                               float*       outputLeft,
                               float*       outputRight,
                               std::size_t  frameCount)
{
    if(outputLeft == nullptr || outputRight == nullptr)
    {
        return;
    }

    if(!prepared_)
    {
        Prepare(sampleRate_, maxBlockSize_);
    }

    const float synthBrightness = bundleMode_
                                      ? EffectiveParameterValueAt(kParamFreeze)
                                      : EffectiveParameterValueAt(kParamTexture);
    const float synthDecay = bundleMode_ ? EffectiveParameterValueAt(kParamMidiLevel)
                                         : EffectiveParameterValueAt(kParamRelease);
    const float synthLevel = bundleMode_ ? EffectiveParameterValueAt(kParamTempo)
                                         : EffectiveParameterValueAt(kParamMidiLevel);
    const float mix = EffectiveParameterValueAt(kParamMix);
    const float outputGain = DbToLinear(NativeValueAt(kParamOutput));
    const float attackMs = std::max(0.5f, NativeValueAt(kParamAttack));
    const float releaseMs = std::max(10.0f, NativeValueAt(kParamRelease));

    for(std::size_t i = 0; i < frameCount; ++i)
    {
        const float left = inputLeft != nullptr ? inputLeft[i] : 0.0f;
        const float right = inputRight != nullptr ? inputRight[i] : left;
        const float internalSynth = ProcessInternalSynth(
            synthBrightness, synthDecay, synthLevel, attackMs, releaseMs);
        const float inL = left + internalSynth;
        const float inR = right + internalSynth;
        float wetL = 0.0f;
        float wetR = 0.0f;

        switch(source_)
        {
            case DaisyDelayFxSource::kReverbPlayground:
                ProcessReverbPlayground(inL, inR, &wetL, &wetR);
                break;
            case DaisyDelayFxSource::kFunBox:
                ProcessFunBox(inL, inR, &wetL, &wetR);
                break;
            case DaisyDelayFxSource::kSdramDelaylines:
                ProcessSdramDelaylines(inL, inR, &wetL, &wetR);
                break;
            case DaisyDelayFxSource::kMultiFxPedal:
            default:
                ProcessMultiFx(inL, inR, &wetL, &wetR);
                break;
        }

        const bool  bypass = buttonStates_[0] == 2;
        const bool  wetOnly = buttonStates_[0] == 1;

        float outL = bypass ? inL : Mix(wetOnly ? 0.0f : inL, wetL, mix);
        float outR = bypass ? inR : Mix(wetOnly ? 0.0f : inR, wetR, mix);
        outL = Clamp(outL * outputGain, -1.2f, 1.2f);
        outR = Clamp(outR * outputGain, -1.2f, 1.2f);
        if(!std::isfinite(outL))
        {
            outL = 0.0f;
        }
        if(!std::isfinite(outR))
        {
            outR = 0.0f;
        }
        outputLeft[i] = outL;
        outputRight[i] = outR;
    }
}

bool DaisyDelayFxCore::SetParameterValue(const std::string& parameterId,
                                         float normalizedValue)
{
    return SetParameterValue(parameterId.c_str(), normalizedValue);
}

bool DaisyDelayFxCore::SetParameterValue(const char* parameterId,
                                         float       normalizedValue)
{
    const std::size_t index = ParameterIndexById(parameterId);
    if(index >= parameters_.size())
    {
        return false;
    }
    parameters_[index].normalizedValue = Clamp01(normalizedValue);
    parameters_[index].effectiveNormalizedValue = parameters_[index].normalizedValue;
    return true;
}

bool DaisyDelayFxCore::GetParameterValue(const std::string& parameterId,
                                         float* normalizedValue) const
{
    return GetParameterValue(parameterId.c_str(), normalizedValue);
}

bool DaisyDelayFxCore::GetParameterValue(const char* parameterId,
                                         float*      normalizedValue) const
{
    const std::size_t index = ParameterIndexById(parameterId);
    if(index >= parameters_.size() || normalizedValue == nullptr)
    {
        return false;
    }
    *normalizedValue = parameters_[index].normalizedValue;
    return true;
}

bool DaisyDelayFxCore::SetEffectiveParameterValue(
    const std::string& parameterId, float normalizedValue)
{
    return SetEffectiveParameterValue(parameterId.c_str(), normalizedValue);
}

bool DaisyDelayFxCore::SetEffectiveParameterValue(const char* parameterId,
                                                  float       normalizedValue)
{
    const std::size_t index = ParameterIndexById(parameterId);
    if(index >= parameters_.size())
    {
        return false;
    }
    parameters_[index].effectiveNormalizedValue = Clamp01(normalizedValue);
    return true;
}

bool DaisyDelayFxCore::GetEffectiveParameterValue(
    const std::string& parameterId, float* normalizedValue) const
{
    return GetEffectiveParameterValue(parameterId.c_str(), normalizedValue);
}

bool DaisyDelayFxCore::GetEffectiveParameterValue(const char* parameterId,
                                                  float* normalizedValue) const
{
    const std::size_t index = ParameterIndexById(parameterId);
    if(index >= parameters_.size() || normalizedValue == nullptr)
    {
        return false;
    }
    *normalizedValue = parameters_[index].effectiveNormalizedValue;
    return true;
}

void DaisyDelayFxCore::ClearEffectiveParameterOverrides()
{
    for(auto& parameter : parameters_)
    {
        parameter.effectiveNormalizedValue = parameter.normalizedValue;
    }
}

const std::vector<DaisyDelayFxParameter>& DaisyDelayFxCore::GetParameters() const
{
    return parameters_;
}

const DaisyDelayFxParameter* DaisyDelayFxCore::FindParameter(
    const std::string& parameterId) const
{
    return FindParameter(parameterId.c_str());
}

const DaisyDelayFxParameter* DaisyDelayFxCore::FindParameter(
    const char* parameterId) const
{
    return FindParameterById(parameterId);
}

const char* DaisyDelayFxCore::GetParameterForLayerKnob(std::size_t layer,
                                                       std::size_t knob) const
{
    if(layer >= kLayerCount || knob >= kKnobCount)
    {
        return "";
    }
    return layerKnobParameterIds_[layer][knob].c_str();
}

std::string DaisyDelayFxCore::FormatParameterValue(
    const std::string& parameterId) const
{
    char text[32];
    FormatParameterValue(parameterId.c_str(), text, sizeof(text));
    return text;
}

void DaisyDelayFxCore::FormatParameterValue(const char* parameterId,
                                            char*       destination,
                                            std::size_t destinationSize) const
{
    if(destination == nullptr || destinationSize == 0)
    {
        return;
    }
    destination[0] = '\0';

    const auto* parameter = FindParameterById(parameterId);
    if(parameter == nullptr)
    {
        return;
    }

    if(parameter->stepCount > 1)
    {
        std::snprintf(destination,
                      destinationSize,
                      "%d/%d",
                      QuantizedState(parameter->effectiveNormalizedValue,
                                     parameter->stepCount)
                          + 1,
                      parameter->stepCount);
        return;
    }

    if(parameter->unitLabel.empty())
    {
        std::snprintf(destination,
                      destinationSize,
                      "%d%%",
                      static_cast<int>(std::round(
                          Clamp01(parameter->effectiveNormalizedValue)
                          * 100.0f)));
        return;
    }

    const float native = NativeValue(*parameter);
    if(parameter->nativePrecision <= 0)
    {
        std::snprintf(destination,
                      destinationSize,
                      "%d%s",
                      static_cast<int>(std::round(native)),
                      parameter->unitLabel.c_str());
    }
    else
    {
        std::snprintf(destination,
                      destinationSize,
                      "%.*f%s",
                      parameter->nativePrecision,
                      native,
                      parameter->unitLabel.c_str());
    }
}

bool DaisyDelayFxCore::TriggerMomentaryAction(const std::string& actionId)
{
    if(actionId == "clear")
    {
        for(auto& delay : delays_)
        {
            delay.Clear();
        }
        return true;
    }
    if(actionId == "octave_down")
    {
        keyboardOctaveOffset_ = std::max(-2, keyboardOctaveOffset_ - 1);
        return true;
    }
    if(actionId == "octave_up")
    {
        keyboardOctaveOffset_ = std::min(2, keyboardOctaveOffset_ + 1);
        return true;
    }
    return false;
}

bool DaisyDelayFxCore::TriggerFieldKeyAction(std::size_t zeroBasedIndex,
                                             bool pressed)
{
    if(zeroBasedIndex >= kFieldKeyCount)
    {
        return false;
    }

    if(zeroBasedIndex < 6)
    {
        if(pressed)
        {
            SetButtonState(zeroBasedIndex, (buttonStates_[zeroBasedIndex] + 1) % 3);
        }
        return true;
    }

    if(zeroBasedIndex == 6)
    {
        if(pressed)
        {
            TriggerMomentaryAction("octave_down");
        }
        return true;
    }
    if(zeroBasedIndex == 7)
    {
        if(pressed)
        {
            TriggerMomentaryAction("octave_up");
        }
        return true;
    }

    static constexpr std::array<int, 8> kWhiteNotes = {{60, 62, 64, 65, 67, 69, 71, 72}};
    const std::size_t whiteIndex = zeroBasedIndex - 8;
    const int note = kWhiteNotes[whiteIndex] + (keyboardOctaveOffset_ * 12);
    HandleMidiEvent(pressed ? 0x90 : 0x80,
                    static_cast<std::uint8_t>(Clamp(static_cast<float>(note), 0.0f, 127.0f)),
                    pressed ? 100 : 0);
    return true;
}

void DaisyDelayFxCore::SetButtonState(std::size_t zeroBasedIndex, int state)
{
    if(zeroBasedIndex >= buttonStates_.size())
    {
        return;
    }
    buttonStates_[zeroBasedIndex] = std::max(0, std::min(2, state));
}

int DaisyDelayFxCore::GetButtonState(std::size_t zeroBasedIndex) const
{
    return zeroBasedIndex < buttonStates_.size() ? buttonStates_[zeroBasedIndex] : 0;
}

void DaisyDelayFxCore::SetInternalSynthMode(int mode)
{
    const int clamped = std::max(0, std::min(2, mode));
    if(clamped == internalSynthMode_)
    {
        return;
    }
    internalSynthMode_ = clamped;
    if(internalSynthMode_ == 0)
    {
        ReleaseAllSynthVoices(true);
    }
}

int DaisyDelayFxCore::GetInternalSynthMode() const
{
    return internalSynthMode_;
}

void DaisyDelayFxCore::SetInternalSynthHoldMode(int mode)
{
    const int clamped = std::max(0, std::min(2, mode));
    if(clamped == internalSynthHoldMode_)
    {
        return;
    }
    internalSynthHoldMode_ = clamped;
    ReleaseAllSynthVoices(clamped == 0);
}

int DaisyDelayFxCore::GetInternalSynthHoldMode() const
{
    return internalSynthHoldMode_;
}

std::array<float, DaisyDelayFxCore::kFieldKeyCount>
DaisyDelayFxCore::GetFieldKeyLedValues() const
{
    std::array<float, kFieldKeyCount> values{};
    for(std::size_t i = 0; i < 6; ++i)
    {
        values[i] = buttonStates_[i] == 0 ? 0.0f
                                          : (buttonStates_[i] == 1 ? 0.32f : 0.85f);
    }
    values[6] = keyboardOctaveOffset_ < 0 ? 0.85f : 0.08f;
    values[7] = keyboardOctaveOffset_ > 0 ? 0.85f : 0.08f;

    static constexpr std::array<int, 8> kWhiteNotes = {{60, 62, 64, 65, 67, 69, 71, 72}};
    for(std::size_t i = 0; i < kWhiteNotes.size(); ++i)
    {
        const int note = kWhiteNotes[i] + (keyboardOctaveOffset_ * 12);
        values[8 + i] = note >= 0 && note < 128 && noteActive_[note] ? 0.85f : 0.03f;
    }
    return values;
}

void DaisyDelayFxCore::HandleMidiEvent(std::uint8_t status,
                                       std::uint8_t data1,
                                       std::uint8_t data2)
{
    const std::uint8_t messageType = status & 0xF0;
    if(messageType == 0x90 && data2 > 0)
    {
        activeMidiNote_ = static_cast<int>(data1);
        if(activeMidiNote_ < 0
           || activeMidiNote_ >= static_cast<int>(noteActive_.size()))
        {
            return;
        }
        activeMidiFrequencyHz_ = MidiNoteToHz(activeMidiNote_);
        if(internalSynthHoldMode_ == 1 && noteActive_[activeMidiNote_])
        {
            noteActive_[activeMidiNote_] = false;
            ReleaseSynthVoice(activeMidiNote_);
            return;
        }
        noteActive_[activeMidiNote_] = true;
        StartSynthVoice(activeMidiNote_, static_cast<float>(data2) / 127.0f);
    }
    else if(messageType == 0x80 || (messageType == 0x90 && data2 == 0))
    {
        const int note = static_cast<int>(data1);
        if(note < 0 || note >= static_cast<int>(noteActive_.size()))
        {
            return;
        }
        if(internalSynthHoldMode_ == 0)
        {
            noteActive_[note] = false;
            ReleaseSynthVoice(note);
        }
    }
}

int DaisyDelayFxCore::GetKeyboardOctaveOffset() const
{
    return keyboardOctaveOffset_;
}

std::unordered_map<std::string, float>
DaisyDelayFxCore::CaptureStatefulParameterValues() const
{
    std::unordered_map<std::string, float> values;
    for(const auto& parameter : parameters_)
    {
        if(parameter.stateful)
        {
            values.emplace(parameter.id, parameter.normalizedValue);
        }
    }
    return values;
}

void DaisyDelayFxCore::RestoreStatefulParameterValues(
    const std::unordered_map<std::string, float>& values)
{
    for(const auto& pair : values)
    {
        SetParameterValue(pair.first, pair.second);
    }
}

void DaisyDelayFxCore::RebuildParameters()
{
    // NOTE: every source emits exactly the same 24 parameters in the same
    // order (only labels/defaults/ranges differ). The audio ISR reads
    // parameters_ concurrently, so we must NOT clear()/push_back here: that
    // frees and reallocates the backing buffer under the callback -> use-
    // after-free -> hard fault on every algorithm switch. Instead, reserve
    // once and overwrite each slot in place so the storage pointer is stable.
    parameters_.reserve(24);
    layerKnobParameterIds_ = {};

    std::size_t writeIndex = 0;
    auto add = [this, &writeIndex](const char* id,
                                   const char* label,
                                   const char* unit,
                                   float       defaultNormalized,
                                   float       nativeMin,
                                   float       nativeMax,
                                   int         precision,
                                   int         layer,
                                   int         knob,
                                   int         rank,
                                   int         stepCount = 0) {
        DaisyDelayFxParameter parameter;
        parameter.id = id;
        parameter.label = label;
        parameter.unitLabel = unit != nullptr ? unit : "";
        parameter.defaultNormalizedValue = Clamp01(defaultNormalized);
        parameter.normalizedValue = parameter.defaultNormalizedValue;
        parameter.effectiveNormalizedValue = parameter.defaultNormalizedValue;
        parameter.nativeMinimum = nativeMin;
        parameter.nativeMaximum = nativeMax;
        parameter.nativeDefault = nativeMin + (nativeMax - nativeMin) * parameter.defaultNormalizedValue;
        parameter.nativePrecision = precision;
        parameter.layer = layer;
        parameter.knob = knob;
        parameter.importanceRank = rank;
        parameter.stepCount = stepCount;
        if(writeIndex < parameters_.size())
        {
            parameters_[writeIndex] = parameter; // overwrite in place, no realloc
        }
        else
        {
            parameters_.push_back(parameter); // first build only
        }
        ++writeIndex;
        if(layer >= 0 && layer < static_cast<int>(kLayerCount)
           && knob >= 0 && knob < static_cast<int>(kKnobCount))
        {
            layerKnobParameterIds_[static_cast<std::size_t>(layer)]
                                  [static_cast<std::size_t>(knob)]
                = id;
        }
    };

    const bool funbox = source_ == DaisyDelayFxSource::kFunBox;
    const bool reverb = source_ == DaisyDelayFxSource::kReverbPlayground;
    const bool sdram = source_ == DaisyDelayFxSource::kSdramDelaylines;
    const bool bundle = bundleMode_;

    add("mix", "Mix", "", 0.48f, 0.0f, 100.0f, 0, 0, 0, 1);
    add("time", sdram ? "Long Time" : "Delay Time", "ms", sdram ? 0.36f : 0.30f,
        sdram ? 80.0f : 40.0f,
        sdram ? 8000.0f : (funbox ? 3000.0f : 2200.0f), 0, 0, 1, 2);
    add("feedback", reverb ? "Decay" : "Feedback", "", reverb ? 0.62f : 0.45f,
        0.0f, 92.0f, 0, 0, 2, 3);
    add("tone", reverb ? "HF Damp" : "Tone", "", 0.55f, 0.0f, 100.0f, 0, 0, 3, 4);
    add("texture", funbox ? "Texture" : (reverb ? "Tank Color" : "Grit"),
        "", funbox ? 0.42f : 0.30f, 0.0f, 100.0f, 0, 0, 4, 5);
    add("mod", funbox ? "Drift" : "Mod", "", funbox ? 0.36f : 0.22f,
        0.0f, 100.0f, 0, 0, 5, 6);
    add("drive", "Input Drive", "dB", 0.38f, -12.0f, 18.0f, 1, 0, 6, 7);
    add("output", "Output", "dB", 0.63f, -18.0f, 6.0f, 1, 0, 7, 8);

    add("pre_delay", "Pre Delay", "ms", reverb ? 0.26f : 0.12f, 0.0f, 500.0f,
        0, 1, 0, 9);
    add("width", "Width", "", 0.65f, 0.0f, 100.0f, 0, 1, 1, 10);
    add("diffusion", reverb ? "Diffusion" : "Spread", "", reverb ? 0.72f : 0.36f,
        0.0f, 100.0f, 0, 1, 2, 11);
    add("damping", "Damping", "", 0.45f, 0.0f, 100.0f, 0, 1, 3, 12);
    add("tap_ratio", funbox ? "Tap Mode" : "Rhythm", "", 0.34f, 0.0f, 100.0f,
        0, 1, 4, 13, 4);
    add("freeze", bundle ? "Synth Bright" : "Freeze Amt", "", bundle ? 0.32f : 0.0f,
        0.0f, 100.0f, 0, 1, 5, 14);
    add("midi_level", bundle ? "Synth Decay" : "MIDI Level", "",
        bundle ? 0.20f : 0.35f, 0.0f, 100.0f, 0, 1, 6, 15);
    add("tempo", bundle ? "Synth Level" : "Tempo", bundle ? "" : "BPM",
        bundle ? 0.32f : 0.42f, bundle ? 0.0f : 40.0f, bundle ? 100.0f : 220.0f,
        0, 1, 7, 16);

    add("size", reverb ? "Tank Size" : "Range", "", reverb ? 0.70f : 0.48f,
        0.0f, 100.0f, 0, 2, 0, 17);
    add("density", funbox ? "Grain Density" : "Density", "", funbox ? 0.55f : 0.40f,
        0.0f, 100.0f, 0, 2, 1, 18);
    add("low_cut", "Low Cut", "Hz", 0.14f, 20.0f, 600.0f, 0, 2, 2, 19);
    add("high_cut", "High Cut", "Hz", 0.80f, 1200.0f, 16000.0f, 0, 2, 3, 20);
    add("smear", funbox ? "Spectral Smear" : "Smear", "", funbox ? 0.52f : 0.25f,
        0.0f, 100.0f, 0, 2, 4, 21);
    add("warp", sdram ? "Interp Warp" : "Warp", "", sdram ? 0.20f : 0.34f,
        0.0f, 100.0f, 0, 2, 5, 22);
    add("attack", "MIDI Attack", "ms", 0.08f, 0.5f, 100.0f, 1, 2, 6, 23);
    add("release", "MIDI Release", "ms", 0.36f, 10.0f, 2000.0f, 0, 2, 7, 24);
}

void DaisyDelayFxCore::UpdateParameterCache()
{
    for(auto& parameter : parameters_)
    {
        parameter.effectiveNormalizedValue = parameter.normalizedValue;
    }
}

float DaisyDelayFxCore::ParameterValue(const char* id) const
{
    const auto* parameter = FindParameterById(id);
    return parameter != nullptr ? parameter->normalizedValue : 0.0f;
}

float DaisyDelayFxCore::EffectiveParameterValue(const char* id) const
{
    const auto* parameter = FindParameterById(id);
    return parameter != nullptr ? parameter->effectiveNormalizedValue : 0.0f;
}

float DaisyDelayFxCore::NativeValue(const char* id) const
{
    const auto* parameter = FindParameterById(id);
    return parameter != nullptr ? NativeValue(*parameter) : 0.0f;
}

float DaisyDelayFxCore::ParameterValueAt(std::size_t index) const
{
    return index < parameters_.size() ? parameters_[index].normalizedValue : 0.0f;
}

float DaisyDelayFxCore::EffectiveParameterValueAt(std::size_t index) const
{
    return index < parameters_.size()
               ? parameters_[index].effectiveNormalizedValue
               : 0.0f;
}

float DaisyDelayFxCore::NativeValueAt(std::size_t index) const
{
    return index < parameters_.size() ? NativeValue(parameters_[index]) : 0.0f;
}

float DaisyDelayFxCore::NativeValue(const DaisyDelayFxParameter& parameter) const
{
    return parameter.nativeMinimum
           + (parameter.nativeMaximum - parameter.nativeMinimum)
                 * Clamp01(parameter.effectiveNormalizedValue);
}

const DaisyDelayFxParameter* DaisyDelayFxCore::FindParameterById(
    const char* id) const
{
    if(id == nullptr)
    {
        return nullptr;
    }
    for(const auto& parameter : parameters_)
    {
        if(std::strcmp(parameter.id.c_str(), id) == 0)
        {
            return &parameter;
        }
    }
    return nullptr;
}

std::size_t DaisyDelayFxCore::ParameterIndex(
    const std::string& parameterId) const
{
    return ParameterIndexById(parameterId.c_str());
}

std::size_t DaisyDelayFxCore::ParameterIndexById(const char* parameterId) const
{
    if(parameterId == nullptr)
    {
        return parameters_.size();
    }
    for(std::size_t i = 0; i < parameters_.size(); ++i)
    {
        if(std::strcmp(parameters_[i].id.c_str(), parameterId) == 0)
        {
            return i;
        }
    }
    return parameters_.size();
}

void DaisyDelayFxCore::StartSynthVoice(int note, float velocity)
{
    if(note < 0 || note >= static_cast<int>(noteActive_.size()))
    {
        return;
    }
    if(internalSynthMode_ == 0)
    {
        noteActive_[static_cast<std::size_t>(note)] = false;
        return;
    }

    SynthVoice* voice = FindVoiceForNote(note);
    if(voice == nullptr)
    {
        voice = AllocateVoice();
    }
    if(voice == nullptr)
    {
        return;
    }

    rngState_ = std::fmod((rngState_ * 3.9871f) + 0.137f, 1.0f);
    const float exciter = (rngState_ * 2.0f) - 1.0f;

    voice->note = note;
    voice->active = true;
    voice->held = true;
    voice->frequency = MidiNoteToHz(note);
    voice->velocity = Clamp01(velocity);
    voice->phase = std::fmod(voice->phase + 0.013f + rngState_ * 0.07f, 1.0f);
    voice->envelope = std::max(voice->envelope,
                               internalSynthMode_ == 2 ? 0.02f
                                                       : voice->velocity);
    voice->body = 0.0f;
    voice->exciter = exciter * voice->velocity;
}

void DaisyDelayFxCore::ReleaseSynthVoice(int note)
{
    for(auto& voice : synthVoices_)
    {
        if(voice.active && voice.note == note)
        {
            voice.held = false;
        }
    }
}

void DaisyDelayFxCore::ReleaseAllSynthVoices(bool immediate)
{
    noteActive_.fill(false);
    for(auto& voice : synthVoices_)
    {
        voice.held = false;
        if(immediate)
        {
            voice = {};
        }
    }
}

DaisyDelayFxCore::SynthVoice* DaisyDelayFxCore::FindVoiceForNote(int note)
{
    for(auto& voice : synthVoices_)
    {
        if(voice.active && voice.note == note)
        {
            return &voice;
        }
    }
    return nullptr;
}

DaisyDelayFxCore::SynthVoice* DaisyDelayFxCore::AllocateVoice()
{
    SynthVoice* quietest = &synthVoices_[0];
    for(auto& voice : synthVoices_)
    {
        if(!voice.active)
        {
            return &voice;
        }
        if(voice.envelope < quietest->envelope)
        {
            quietest = &voice;
        }
    }
    return quietest;
}

float DaisyDelayFxCore::ProcessInternalSynth(float brightness,
                                             float decay,
                                             float level,
                                             float attackMs,
                                             float releaseMs)
{
    if(internalSynthMode_ == 0 || level <= 0.0001f)
    {
        return 0.0f;
    }

    const float sampleRate = static_cast<float>(sampleRate_);
    const float bright = Clamp01(brightness);
    const float decaySeconds = 0.12f + 5.8f * decay * decay;
    const float pluckDecay = std::exp(-1.0f / (decaySeconds * sampleRate));
    const float attackCoeff
        = 1.0f - std::exp(-1.0f / (0.001f * std::max(0.5f, attackMs) * sampleRate));
    const float releaseCoeff = 1.0f - std::exp(
        -1.0f
        / (0.001f
           * std::max(30.0f, releaseMs + (decay * 1800.0f))
           * sampleRate));

    float sum = 0.0f;
    for(auto& voice : synthVoices_)
    {
        if(!voice.active)
        {
            continue;
        }

        voice.phase += voice.frequency / sampleRate;
        if(voice.phase >= 1.0f)
        {
            voice.phase -= std::floor(voice.phase);
        }

        if(internalSynthMode_ == 1)
        {
            const bool noteHeld = voice.note >= 0
                                  && voice.note < static_cast<int>(noteActive_.size())
                                  && noteActive_[static_cast<std::size_t>(voice.note)];
            const float releasedDecay = std::exp(-1.0f / (0.08f * sampleRate));
            voice.envelope *= (noteHeld || voice.held) ? pluckDecay
                                                       : releasedDecay;
        }
        else
        {
            const bool noteHeld = voice.note >= 0
                                  && voice.note < static_cast<int>(noteActive_.size())
                                  && noteActive_[static_cast<std::size_t>(voice.note)];
            const float target = noteHeld || voice.held ? voice.velocity : 0.0f;
            const float coeff = target > voice.envelope ? attackCoeff
                                                        : releaseCoeff;
            voice.envelope += coeff * (target - voice.envelope);
        }

        voice.exciter *= 0.988f - (bright * 0.018f);
        const float phase = 2.0f * kPi * voice.phase;
        const float fundamental = std::sin(phase);
        const float second = std::sin(phase * 2.01f);
        const float third = std::sin(phase * 3.0f);
        const float partials = fundamental
                               + bright * ((0.42f * second) + (0.22f * third));
        const float resonant = partials + voice.exciter * (0.30f + 0.45f * bright);
        voice.body += (0.02f + bright * 0.18f) * (resonant - voice.body);
        sum += Mix(voice.body, resonant, 0.35f + bright * 0.45f)
               * voice.envelope;

        if(voice.envelope < 0.00008f && !voice.held)
        {
            voice = {};
        }
    }

    return Clamp(sum * (0.18f + 0.62f * Clamp01(level)), -0.9f, 0.9f);
}

void DaisyDelayFxCore::ProcessMultiFx(float inputLeft,
                                      float inputRight,
                                      float* outputLeft,
                                      float* outputRight)
{
    const float sr = static_cast<float>(sampleRate_);
    const float drive = DbToLinear(NativeValueAt(kParamDrive));
    const float input = (inputLeft + inputRight) * 0.5f * drive;
    const float timeMs = NativeValueAt(kParamTime);
    const float feedback = Clamp(EffectiveParameterValueAt(kParamFeedback) * 0.92f, 0.0f, 0.92f);
    const float width = EffectiveParameterValueAt(kParamWidth);
    const float mod = EffectiveParameterValueAt(kParamMod);
    const float grit = EffectiveParameterValueAt(kParamTexture);
    const float freezeParam = bundleMode_ ? 0.0f
                                          : EffectiveParameterValueAt(kParamFreeze);
    const float freeze = std::max(freezeParam,
                                  buttonStates_[1] > 0 ? (buttonStates_[1] == 2 ? 1.0f : 0.65f) : 0.0f);
    const float tapRatio = buttonStates_[3] == 1 ? 0.75f
                         : (buttonStates_[3] == 2 ? 0.6666667f
                                                   : Mix(0.5f, 1.0f, ParameterValueAt(kParamTapRatio)));

    lfoPhase_ += (0.05f + 7.0f * mod * mod) / sr;
    if(lfoPhase_ >= 1.0f)
    {
        lfoPhase_ -= 1.0f;
    }
    const float lfo = std::sin(2.0f * kPi * lfoPhase_);
    const float targetDelay = Clamp(timeMs * 0.001f * sr * tapRatio, 24.0f, delays_[0].size > 0 ? static_cast<float>(delays_[0].size - 8) : 24.0f);
    delaySmooth_[0] += 0.0006f * (targetDelay - delaySmooth_[0]);
    delaySmooth_[1] += 0.0006f * ((targetDelay * (1.0f + 0.012f + width * 0.04f)) - delaySmooth_[1]);
    const float flutter = (8.0f + 400.0f * mod * mod) * lfo;
    const float readL = delays_[0].Read(delaySmooth_[0] + flutter);
    const float readR = delays_[1].Read(delaySmooth_[1] - flutter);
    const float monoWet = (readL + readR) * 0.5f;
    const float toneHz = 900.0f + 9000.0f * (1.0f - EffectiveParameterValueAt(kParamTone));
    const float toneCoeff = OnePoleCoeff(toneHz, sr);
    toneState_[0] += toneCoeff * (monoWet - toneState_[0]);
    const float saturated = FastTanh((toneState_[0] * (1.0f + grit * 4.0f)) + input);
    const float write = Mix(input + (toneState_[0] * feedback), saturated, grit);
    const float writeBlend = 1.0f - freeze;
    delays_[0].Write((write * writeBlend) + (readL * feedback * freeze));
    delays_[1].Write((write * writeBlend) + (readR * feedback * freeze));
    *outputLeft = readL;
    *outputRight = readR;
}

void DaisyDelayFxCore::ProcessReverbPlayground(float inputLeft,
                                               float inputRight,
                                               float* outputLeft,
                                               float* outputRight)
{
    const float sr = static_cast<float>(sampleRate_);
    const float input = (inputLeft + inputRight) * 0.5f
                        * DbToLinear(NativeValueAt(kParamDrive)) * 0.55f;
    const float decay = Clamp(0.15f + EffectiveParameterValueAt(kParamFeedback) * 0.82f,
                              0.0f,
                              0.92f);
    const float size = 0.55f + EffectiveParameterValueAt(kParamSize) * 1.8f;
    const float diffusion = Clamp01(EffectiveParameterValueAt(kParamDiffusion)
                                    + (buttonStates_[4] * 0.18f));
    const float dampingHz = 1200.0f + (1.0f - EffectiveParameterValueAt(kParamDamping))
                                        * 12000.0f;
    const float dampCoeff = OnePoleCoeff(dampingHz, sr);
    static constexpr std::array<float, 4> kBaseDelaysMs = {{37.0f, 53.0f, 71.0f, 89.0f}};

    float taps[4] = {};
    for(std::size_t i = 0; i < 4; ++i)
    {
        const float delaySamples = Clamp(kBaseDelaysMs[i] * size * 0.001f * sr,
                                         24.0f,
                                         delays_[i].size > 0 ? static_cast<float>(delays_[i].size - 8) : 24.0f);
        taps[i] = delays_[i].Read(delaySamples);
        dampingState_[i] += dampCoeff * (taps[i] - dampingState_[i]);
    }

    const float a = dampingState_[0];
    const float b = dampingState_[1];
    const float c = dampingState_[2];
    const float d = dampingState_[3];
    const float h0 = (a + b + c + d) * 0.5f;
    const float h1 = (a - b + c - d) * 0.5f;
    const float h2 = (a + b - c - d) * 0.5f;
    const float h3 = (a - b - c + d) * 0.5f;
    const float diffuseInput = input * (0.18f + diffusion * 0.52f);
    delays_[0].Write(diffuseInput + h0 * decay);
    delays_[1].Write(diffuseInput + h1 * decay);
    delays_[2].Write(diffuseInput + h2 * decay);
    delays_[3].Write(diffuseInput + h3 * decay);

    const float width = EffectiveParameterValueAt(kParamWidth);
    const float preDelay = EffectiveParameterValueAt(kParamPreDelay);
    const float left = (a + c) * 0.42f + input * preDelay * 0.25f;
    const float right = (b + d) * 0.42f + input * preDelay * 0.25f;
    const float mono = (left + right) * 0.5f;
    *outputLeft = Mix(mono, left, width);
    *outputRight = Mix(mono, right, width);
}

void DaisyDelayFxCore::ProcessFunBox(float inputLeft,
                                     float inputRight,
                                     float* outputLeft,
                                     float* outputRight)
{
    const float sr = static_cast<float>(sampleRate_);
    const float drive = DbToLinear(NativeValueAt(kParamDrive));
    const float inL = inputLeft * drive;
    const float inR = inputRight * drive;
    const float timeMs = NativeValueAt(kParamTime);
    const float feedback = Clamp(EffectiveParameterValueAt(kParamFeedback) * 0.88f,
                                 0.0f,
                                 0.88f);
    const float texture = EffectiveParameterValueAt(kParamTexture);
    const float density = EffectiveParameterValueAt(kParamDensity);
    const float smear = EffectiveParameterValueAt(kParamSmear);
    const float mod = EffectiveParameterValueAt(kParamMod);
    const float width = EffectiveParameterValueAt(kParamWidth);
    const float reverseAmount = buttonStates_[2] == 0 ? 0.0f
                              : (buttonStates_[2] == 1 ? 0.45f : 0.85f);
    const float freezeParam = bundleMode_ ? 0.0f
                                          : EffectiveParameterValueAt(kParamFreeze);
    const float freeze = std::max(freezeParam,
                                  buttonStates_[1] > 0 ? 0.75f : 0.0f);
    const float ratio = buttonStates_[3] == 2 ? 0.6666667f
                       : (buttonStates_[3] == 1 ? 0.75f
                                                 : Mix(0.5f, 1.0f, ParameterValueAt(kParamTapRatio)));

    slowLfoPhase_ += (0.03f + mod * 4.0f) / sr;
    if(slowLfoPhase_ >= 1.0f)
    {
        slowLfoPhase_ -= 1.0f;
    }
    const float drift = std::sin(2.0f * kPi * slowLfoPhase_);
    const float baseDelay = Clamp(timeMs * 0.001f * sr, 24.0f,
                                  delays_[0].size > 0 ? static_cast<float>(delays_[0].size - 8) : 24.0f);
    delaySmooth_[0] += 0.0008f * ((baseDelay + drift * baseDelay * 0.04f * texture) - delaySmooth_[0]);
    delaySmooth_[1] += 0.0008f * ((baseDelay * ratio - drift * baseDelay * 0.03f * texture) - delaySmooth_[1]);
    const float normalL = delays_[0].Read(delaySmooth_[0]);
    const float normalR = delays_[1].Read(delaySmooth_[0] * (1.0f + 0.04f * width));
    const float grainDelay = Clamp(baseDelay * Mix(0.12f, 0.85f, density)
                                       + drift * baseDelay * 0.18f * smear,
                                   24.0f,
                                   delays_[2].size > 0 ? static_cast<float>(delays_[2].size - 8) : 24.0f);
    const float grain = delays_[2].Read(grainDelay);
    const float reverseTap = delays_[3].Read(Clamp(baseDelay * (1.0f - 0.65f * drift),
                                                  24.0f,
                                                  delays_[3].size > 0 ? static_cast<float>(delays_[3].size - 8) : 24.0f));
    const float wetL = Mix(normalL, grain, texture);
    const float wetR = Mix(normalR, reverseTap, Clamp01(texture + reverseAmount));
    const float writeL = (inL * (1.0f - freeze)) + wetR * feedback;
    const float writeR = (inR * (1.0f - freeze)) + wetL * feedback;
    delays_[0].Write(writeL);
    delays_[1].Write(writeR);
    delays_[2].Write((inL + inR) * 0.5f * (1.0f - freeze) + grain * feedback);
    delays_[3].Write((inL - inR) * 0.5f * (1.0f - freeze) + reverseTap * feedback);
    *outputLeft = wetL;
    *outputRight = wetR;
}

void DaisyDelayFxCore::ProcessSdramDelaylines(float inputLeft,
                                              float inputRight,
                                              float* outputLeft,
                                              float* outputRight)
{
    const float sr = static_cast<float>(sampleRate_);
    const float timeMs = NativeValueAt(kParamTime);
    const float baseDelay = Clamp(timeMs * 0.001f * sr,
                                  24.0f,
                                  delays_[0].size > 0 ? static_cast<float>(delays_[0].size - 8) : 24.0f);
    const float warp = EffectiveParameterValueAt(kParamWarp);
    const float mod = EffectiveParameterValueAt(kParamMod);
    const float modDepth = mod * mod;
    const float width = EffectiveParameterValueAt(kParamWidth);
    lfoPhase_ += (0.02f + 3.0f * mod) / sr;
    if(lfoPhase_ >= 1.0f)
    {
        lfoPhase_ -= 1.0f;
    }
    const float lfo = std::sin(2.0f * kPi * lfoPhase_);
    delaySmooth_[0] += 0.00035f * (baseDelay - delaySmooth_[0]);
    delaySmooth_[1] += 0.00035f * ((baseDelay * (1.0f + width * 0.2f)) - delaySmooth_[1]);
    const float readL = delays_[0].Read(delaySmooth_[0] + lfo * baseDelay * 0.04f * modDepth);
    const float readR = delays_[1].Read(delaySmooth_[1] - lfo * baseDelay * 0.04f * modDepth);
    const float tapL = delays_[2].Read(Clamp(baseDelay * Mix(0.25f, 0.95f, warp), 24.0f,
                                           delays_[2].size > 0 ? static_cast<float>(delays_[2].size - 8) : 24.0f));
    const float tapR = delays_[3].Read(Clamp(baseDelay * Mix(0.35f, 1.15f, warp), 24.0f,
                                           delays_[3].size > 0 ? static_cast<float>(delays_[3].size - 8) : 24.0f));
    const float feedback = Clamp(EffectiveParameterValueAt(kParamFeedback) * 0.90f,
                                 0.0f,
                                 0.90f);
    const float cross = EffectiveParameterValueAt(kParamTexture);
    const float drive = DbToLinear(NativeValueAt(kParamDrive));
    delays_[0].Write((inputLeft * drive) + Mix(readL, readR, cross) * feedback);
    delays_[1].Write((inputRight * drive) + Mix(readR, readL, cross) * feedback);
    delays_[2].Write(inputLeft + tapR * feedback * 0.65f);
    delays_[3].Write(inputRight + tapL * feedback * 0.65f);
    const float smear = EffectiveParameterValueAt(kParamSmear);
    *outputLeft = Mix(readL, tapL, smear);
    *outputRight = Mix(readR, tapR, smear);
}
} // namespace daisyhost
