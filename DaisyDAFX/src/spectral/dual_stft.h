// # Dual STFT Backends
// Portable comparison backends for Fast-STFT-style and DAFX stftenv-style
// analysis/synthesis.
//
// References:
// - Farmer2K5/daisy-fast-stft: Fast_STFT, Fast_RFFT, Fast_ISTFT, window and
//   spectral helper structure.
// - DAFX 2nd Ed. M_files_chap10/stftenv.m: zero-phase placement and envelope
//   compensation identity reference.
#pragma once
#ifndef DSY_DUAL_STFT_H
#define DSY_DUAL_STFT_H

#include "../utility/fft_handler.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>

#if defined(ARM_MATH_CM7) || defined(ARM_MATH_CM4) || defined(ARM_MATH_CM33)
#include "arm_math.h"
#include "arm_common_tables.h"
#define DAFX_DUAL_STFT_HAS_CMSIS_RFFT 1
#else
#define DAFX_DUAL_STFT_HAS_CMSIS_RFFT 0
#endif

namespace daisysp {

enum class StftBackendKind { FastStft, DafxStftEnv };

enum class StftProcessingMode { Complex, MagPhase, DafxIdentity };

namespace stft {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 6.28318530717958647692f;
constexpr float kEnvelopeEpsilon = 1.0e-12f;

inline void MakeHannWindow(float *window, size_t size) {
  if (window == nullptr || size == 0) {
    return;
  }

  const float scale = kTwoPi / static_cast<float>(size);
  for (size_t i = 0; i < size; ++i) {
    window[i] = 0.5f * (1.0f - std::cos(scale * static_cast<float>(i)));
  }
}

inline void NormalizeWindowBySum(float *window, size_t size) {
  if (window == nullptr || size == 0) {
    return;
  }

  float sum = 0.0f;
  for (size_t i = 0; i < size; ++i) {
    sum += window[i];
  }

  if (std::fabs(sum) <= kEnvelopeEpsilon) {
    return;
  }

  const float gain = 1.0f / sum;
  for (size_t i = 0; i < size; ++i) {
    window[i] *= gain;
  }
}

template <size_t FFT_SIZE, size_t HOP_SIZE>
float ComputeSquaredColaGain(const float (&window)[FFT_SIZE]) {
  static_assert(FFT_SIZE % HOP_SIZE == 0,
                "FFT_SIZE must be a multiple of HOP_SIZE");

  float mean = 0.0f;
  for (size_t i = 0; i < HOP_SIZE; ++i) {
    float sum = 0.0f;
    for (size_t offset = i; offset < FFT_SIZE; offset += HOP_SIZE) {
      sum += window[offset] * window[offset];
    }
    mean += sum;
  }

  mean /= static_cast<float>(HOP_SIZE);
  return mean > kEnvelopeEpsilon ? 1.0f / mean : 1.0f;
}

template <size_t WINDOW_SIZE, size_t HOP_SIZE>
float ComputeDafxEnvelope(const float (&window)[WINDOW_SIZE],
                          float (&envelope)[HOP_SIZE]) {
  for (size_t i = 0; i < HOP_SIZE; ++i) {
    envelope[i] = 0.0f;
  }

  for (size_t i = 0; i < WINDOW_SIZE; ++i) {
    envelope[i % HOP_SIZE] += window[i];
  }

  float max_envelope = 0.0f;
  for (size_t i = 0; i < HOP_SIZE; ++i) {
    max_envelope = std::max(max_envelope, envelope[i]);
  }

  const float threshold = std::max(max_envelope * 0.1f, kEnvelopeEpsilon);
  for (size_t i = 0; i < HOP_SIZE; ++i) {
    if (envelope[i] < threshold) {
      envelope[i] = threshold;
    }
  }

  return threshold;
}

template <size_t FFT_SIZE>
void ToMagPhase(const Complex (&spectrum)[FFT_SIZE], float (&magnitude)[FFT_SIZE],
                float (&phase)[FFT_SIZE]) {
  for (size_t i = 0; i < FFT_SIZE; ++i) {
    magnitude[i] = spectrum[i].Magnitude();
    phase[i] = spectrum[i].Phase();
  }
}

template <size_t FFT_SIZE>
void FromMagPhase(const float (&magnitude)[FFT_SIZE],
                  const float (&phase)[FFT_SIZE],
                  Complex (&spectrum)[FFT_SIZE]) {
  for (size_t i = 0; i < FFT_SIZE; ++i) {
    spectrum[i] = Complex::FromPolar(magnitude[i], phase[i]);
  }
}

template <size_t Size> void Clear(float (&buffer)[Size]) {
  std::memset(buffer, 0, sizeof(buffer));
}

template <size_t Size> void Clear(Complex (&buffer)[Size]) {
  for (size_t i = 0; i < Size; ++i) {
    buffer[i] = Complex();
  }
}

inline float Clamp(float value, float minimum, float maximum) {
  return std::max(minimum, std::min(maximum, value));
}

template <size_t FFT_SIZE>
void ComputeFrequencyBins(float (&frequencies)[FFT_SIZE / 2 + 1],
                          float sample_rate) {
  const float bin_hz = sample_rate / static_cast<float>(FFT_SIZE);
  for (size_t i = 0; i <= FFT_SIZE / 2; ++i) {
    frequencies[i] = static_cast<float>(i) * bin_hz;
  }
}

} // namespace stft

template <size_t FFT_SIZE> class Fast_RFFT {
public:
  static_assert((FFT_SIZE & (FFT_SIZE - 1)) == 0,
                "FFT_SIZE must be a power of two");
  static_assert(FFT_SIZE >= 32 && FFT_SIZE <= 4096,
                "FFT_SIZE must be in the CMSIS fast RFFT supported range");

  Fast_RFFT() : initialized_(false) {}

  void Init() {
#if DAFX_DUAL_STFT_HAS_CMSIS_RFFT
    initialized_ = InitCmsisInstance();
#else
    portable_fft_.Init();
    initialized_ = portable_fft_.IsInitialized();
#endif
  }

  void Forward(const float *input, Complex (&output)[FFT_SIZE]) {
#if DAFX_DUAL_STFT_HAS_CMSIS_RFFT
    arm_rfft_fast_f32(&cmsis_instance_, const_cast<float *>(input),
                      packed_spectrum_, 0);
    UnpackCmsisSpectrum(output);
#else
    portable_fft_.Forward(input, output);
#endif
  }

  void Inverse(const Complex (&input)[FFT_SIZE], float *output) {
#if DAFX_DUAL_STFT_HAS_CMSIS_RFFT
    PackCmsisSpectrum(input);
    arm_rfft_fast_f32(&cmsis_instance_, packed_spectrum_, output, 1);
#else
    portable_fft_.Inverse(input, output);
#endif
  }

  bool IsInitialized() const { return initialized_; }

private:
  void UnpackCmsisSpectrum(Complex (&output)[FFT_SIZE]) {
#if DAFX_DUAL_STFT_HAS_CMSIS_RFFT
    output[0] = Complex(packed_spectrum_[0], 0.0f);
    output[FFT_SIZE / 2] = Complex(packed_spectrum_[1], 0.0f);

    for (size_t bin = 1; bin < FFT_SIZE / 2; ++bin) {
      const float real = packed_spectrum_[2 * bin];
      const float imag = packed_spectrum_[2 * bin + 1];
      output[bin] = Complex(real, imag);
      output[FFT_SIZE - bin] = Complex(real, -imag);
    }
#else
    (void)output;
#endif
  }

  void PackCmsisSpectrum(const Complex (&input)[FFT_SIZE]) {
#if DAFX_DUAL_STFT_HAS_CMSIS_RFFT
    packed_spectrum_[0] = input[0].real;
    packed_spectrum_[1] = input[FFT_SIZE / 2].real;

    for (size_t bin = 1; bin < FFT_SIZE / 2; ++bin) {
      packed_spectrum_[2 * bin] = input[bin].real;
      packed_spectrum_[2 * bin + 1] = input[bin].imag;
    }
#else
    (void)input;
#endif
  }

  bool initialized_;
#if DAFX_DUAL_STFT_HAS_CMSIS_RFFT
  bool InitCmsisInstance() {
    arm_cfft_instance_f32 *cfft = &cmsis_instance_.Sint;
    cfft->fftLen = static_cast<uint16_t>(FFT_SIZE / 2);
    cmsis_instance_.fftLenRFFT = static_cast<uint16_t>(FFT_SIZE);

    switch (cfft->fftLen) {
    case 256u:
      cfft->bitRevLength = ARMBITREVINDEXTABLE_256_TABLE_LENGTH;
      cfft->pBitRevTable = (uint16_t *)armBitRevIndexTable256;
      cfft->pTwiddle = (float32_t *)twiddleCoef_256;
      cmsis_instance_.pTwiddleRFFT = (float32_t *)twiddleCoef_rfft_512;
      return true;
    case 512u:
      cfft->bitRevLength = ARMBITREVINDEXTABLE_512_TABLE_LENGTH;
      cfft->pBitRevTable = (uint16_t *)armBitRevIndexTable512;
      cfft->pTwiddle = (float32_t *)twiddleCoef_512;
      cmsis_instance_.pTwiddleRFFT = (float32_t *)twiddleCoef_rfft_1024;
      return true;
    case 1024u:
      cfft->bitRevLength = ARMBITREVINDEXTABLE_1024_TABLE_LENGTH;
      cfft->pBitRevTable = (uint16_t *)armBitRevIndexTable1024;
      cfft->pTwiddle = (float32_t *)twiddleCoef_1024;
      cmsis_instance_.pTwiddleRFFT = (float32_t *)twiddleCoef_rfft_2048;
      return true;
    case 2048u:
      cfft->bitRevLength = ARMBITREVINDEXTABLE_2048_TABLE_LENGTH;
      cfft->pBitRevTable = (uint16_t *)armBitRevIndexTable2048;
      cfft->pTwiddle = (float32_t *)twiddleCoef_2048;
      cmsis_instance_.pTwiddleRFFT = (float32_t *)twiddleCoef_rfft_4096;
      return true;
    default:
      return false;
    }
  }

  arm_rfft_fast_instance_f32 cmsis_instance_;
  float packed_spectrum_[FFT_SIZE];
#else
  FFTHandler<FFT_SIZE> portable_fft_;
#endif
};

template <size_t FFT_SIZE, size_t HOP_SIZE> class Fast_ISTFT {
public:
  static_assert((FFT_SIZE & (FFT_SIZE - 1)) == 0,
                "FFT_SIZE must be a power of two");
  static_assert(FFT_SIZE % HOP_SIZE == 0,
                "FFT_SIZE must be a multiple of HOP_SIZE");

  Fast_ISTFT() : read_pos_(0), cola_gain_(1.0f) {}

  void Init() {
    fft_.Init();
    stft::MakeHannWindow(window_, FFT_SIZE);
    cola_gain_ = stft::ComputeSquaredColaGain<FFT_SIZE, HOP_SIZE>(window_);
    Reset();
  }

  void Reset() {
    stft::Clear(overlap_ring_);
    stft::Clear(time_buffer_);
    read_pos_ = 0;
  }

  void ProcessFrame(const Complex (&spectrum)[FFT_SIZE], float *output) {
    fft_.Inverse(spectrum, time_buffer_);

    for (size_t i = 0; i < FFT_SIZE; ++i) {
      const size_t target = (read_pos_ + i) % FFT_SIZE;
      overlap_ring_[target] += time_buffer_[i] * window_[i] * cola_gain_;
    }

    for (size_t i = 0; i < HOP_SIZE; ++i) {
      const size_t source = (read_pos_ + i) % FFT_SIZE;
      output[i] = overlap_ring_[source];
      overlap_ring_[source] = 0.0f;
    }

    read_pos_ = (read_pos_ + HOP_SIZE) % FFT_SIZE;
  }

private:
  Fast_RFFT<FFT_SIZE> fft_;
  float window_[FFT_SIZE];
  float overlap_ring_[FFT_SIZE];
  float time_buffer_[FFT_SIZE];
  size_t read_pos_;
  float cola_gain_;
};

template <size_t FFT_SIZE, size_t HOP_SIZE, size_t BLOCK_SIZE,
          StftProcessingMode MODE = StftProcessingMode::Complex>
class FastStftBackend {
public:
  static_assert((FFT_SIZE & (FFT_SIZE - 1)) == 0,
                "FFT_SIZE must be a power of two");
  static_assert(FFT_SIZE % HOP_SIZE == 0,
                "FFT_SIZE must be a multiple of HOP_SIZE");
  static_assert(HOP_SIZE % BLOCK_SIZE == 0,
                "HOP_SIZE must be a multiple of BLOCK_SIZE");

  static constexpr size_t FftSize() { return FFT_SIZE; }
  static constexpr size_t HopSize() { return HOP_SIZE; }
  static constexpr size_t BlockSize() { return BLOCK_SIZE; }
  static constexpr size_t LatencySamples() { return FFT_SIZE; }

  FastStftBackend()
      : sample_rate_(48000.0f), input_write_pos_(0), output_read_pos_(0),
        output_write_pos_(LatencySamples()), samples_until_frame_(HOP_SIZE),
        cola_gain_(1.0f) {}

  void Init(float sample_rate) {
    sample_rate_ = sample_rate;
    fft_.Init();

    stft::MakeHannWindow(window_, FFT_SIZE);
    cola_gain_ = stft::ComputeSquaredColaGain<FFT_SIZE, HOP_SIZE>(window_);

    stft::Clear(input_ring_);
    stft::Clear(output_ring_);
    stft::Clear(frame_);
    stft::Clear(time_buffer_);
    stft::Clear(spectrum_);
    stft::Clear(magnitude_);
    stft::Clear(phase_);

    input_write_pos_ = 0;
    output_read_pos_ = 0;
    output_write_pos_ = LatencySamples();
    samples_until_frame_ = HOP_SIZE;
  }

  void ProcessBlock(const float *input, float *output) {
    for (size_t i = 0; i < BLOCK_SIZE; ++i) {
      PushInput(input[i]);
      output[i] = PopOutput();

      --samples_until_frame_;
      if (samples_until_frame_ == 0) {
        ProcessFrame();
        samples_until_frame_ = HOP_SIZE;
      }
    }
  }

  const char *Name() const { return "FastStftBackend"; }

  float SampleRate() const { return sample_rate_; }

protected:
  void ProcessFrameComplex(Complex *spectrum, size_t size) {
    (void)spectrum;
    (void)size;
  }

  void ProcessFrameMagPhase(float *magnitude, float *phase, size_t size) {
    (void)magnitude;
    (void)phase;
    (void)size;
  }

private:
  static constexpr size_t kOutputRingSize = FFT_SIZE * 4;

  void PushInput(float sample) {
    input_ring_[input_write_pos_] = sample;
    input_write_pos_ = (input_write_pos_ + 1) % FFT_SIZE;
  }

  float PopOutput() {
    const float sample = output_ring_[output_read_pos_];
    output_ring_[output_read_pos_] = 0.0f;
    output_read_pos_ = (output_read_pos_ + 1) % kOutputRingSize;
    return sample;
  }

  void ProcessFrame() {
    const size_t start = input_write_pos_;
    for (size_t i = 0; i < FFT_SIZE; ++i) {
      const size_t source = (start + i) % FFT_SIZE;
      frame_[i] = input_ring_[source] * window_[i];
    }

    fft_.Forward(frame_, spectrum_);

    if (MODE == StftProcessingMode::MagPhase) {
      stft::ToMagPhase<FFT_SIZE>(spectrum_, magnitude_, phase_);
      ProcessFrameMagPhase(magnitude_, phase_, FFT_SIZE);
      stft::FromMagPhase<FFT_SIZE>(magnitude_, phase_, spectrum_);
    } else {
      ProcessFrameComplex(spectrum_, FFT_SIZE);
    }

    fft_.Inverse(spectrum_, time_buffer_);

    for (size_t i = 0; i < FFT_SIZE; ++i) {
      const size_t target = (output_write_pos_ + i) % kOutputRingSize;
      output_ring_[target] += time_buffer_[i] * window_[i] * cola_gain_;
    }

    output_write_pos_ = (output_write_pos_ + HOP_SIZE) % kOutputRingSize;
  }

  Fast_RFFT<FFT_SIZE> fft_;
  float sample_rate_;
  float input_ring_[FFT_SIZE];
  float output_ring_[kOutputRingSize];
  float window_[FFT_SIZE];
  float frame_[FFT_SIZE];
  float time_buffer_[FFT_SIZE];
  Complex spectrum_[FFT_SIZE];
  float magnitude_[FFT_SIZE];
  float phase_[FFT_SIZE];
  size_t input_write_pos_;
  size_t output_read_pos_;
  size_t output_write_pos_;
  size_t samples_until_frame_;
  float cola_gain_;
};

template <size_t FFT_SIZE, size_t HOP_SIZE, size_t BLOCK_SIZE>
class DafxStftEnvBackend {
public:
  static_assert((FFT_SIZE & (FFT_SIZE - 1)) == 0,
                "FFT_SIZE must be a power of two");
  static_assert(HOP_SIZE % BLOCK_SIZE == 0,
                "HOP_SIZE must be a multiple of BLOCK_SIZE");
  static_assert(FFT_SIZE > 2, "FFT_SIZE must leave room for an odd window");

  static constexpr size_t WindowSize() { return FFT_SIZE - 1; }
  static constexpr size_t FftSize() { return FFT_SIZE; }
  static constexpr size_t HopSize() { return HOP_SIZE; }
  static constexpr size_t BlockSize() { return BLOCK_SIZE; }
  static constexpr size_t LatencySamples() { return FFT_SIZE; }

  DafxStftEnvBackend()
      : sample_rate_(48000.0f), input_write_pos_(0), output_read_pos_(0),
        output_write_pos_(LatencySamples()), samples_until_frame_(HOP_SIZE),
        envelope_threshold_(stft::kEnvelopeEpsilon) {}

  void Init(float sample_rate) {
    sample_rate_ = sample_rate;
    fft_.Init();

    stft::MakeHannWindow(window_, WindowSize());
    stft::NormalizeWindowBySum(window_, WindowSize());
    envelope_threshold_ =
        stft::ComputeDafxEnvelope<WindowSize(), HOP_SIZE>(window_,
                                                          envelope_period_);

    stft::Clear(input_ring_);
    stft::Clear(output_ring_);
    stft::Clear(envelope_ring_);
    stft::Clear(windowed_frame_);
    stft::Clear(fft_input_);
    stft::Clear(time_buffer_);
    stft::Clear(output_frame_);
    stft::Clear(spectrum_);

    input_write_pos_ = 0;
    output_read_pos_ = 0;
    output_write_pos_ = LatencySamples();
    samples_until_frame_ = HOP_SIZE;
  }

  void ProcessBlock(const float *input, float *output) {
    for (size_t i = 0; i < BLOCK_SIZE; ++i) {
      PushInput(input[i]);
      output[i] = PopOutput();

      --samples_until_frame_;
      if (samples_until_frame_ == 0) {
        ProcessFrame();
        samples_until_frame_ = HOP_SIZE;
      }
    }
  }

  const char *Name() const { return "DafxStftEnvBackend"; }

  float SampleRate() const { return sample_rate_; }

  float EnvelopeThreshold() const { return envelope_threshold_; }

private:
  static constexpr size_t kWindowSize = FFT_SIZE - 1;
  static constexpr size_t kHalfWindow = (kWindowSize - 1) / 2;
  static constexpr size_t kRightHalfSize = kHalfWindow + 1;
  static constexpr size_t kOutputRingSize = FFT_SIZE * 4;

  void PushInput(float sample) {
    input_ring_[input_write_pos_] = sample;
    input_write_pos_ = (input_write_pos_ + 1) % kWindowSize;
  }

  float PopOutput() {
    const float envelope =
        std::max(envelope_ring_[output_read_pos_], envelope_threshold_);
    const float sample = output_ring_[output_read_pos_] / envelope;
    output_ring_[output_read_pos_] = 0.0f;
    envelope_ring_[output_read_pos_] = 0.0f;
    output_read_pos_ = (output_read_pos_ + 1) % kOutputRingSize;
    return sample;
  }

  void ProcessFrame() {
    const size_t start = input_write_pos_;
    for (size_t i = 0; i < kWindowSize; ++i) {
      const size_t source = (start + i) % kWindowSize;
      windowed_frame_[i] = input_ring_[source] * window_[i];
    }

    stft::Clear(fft_input_);

    for (size_t i = 0; i < kRightHalfSize; ++i) {
      fft_input_[i] = windowed_frame_[kHalfWindow + i];
    }

    for (size_t i = 0; i < kHalfWindow; ++i) {
      fft_input_[FFT_SIZE - kHalfWindow + i] = windowed_frame_[i];
    }

    fft_.Forward(fft_input_, spectrum_);
    fft_.Inverse(spectrum_, time_buffer_);

    for (size_t i = 0; i < kHalfWindow; ++i) {
      output_frame_[i] = time_buffer_[FFT_SIZE - kHalfWindow + i];
    }

    for (size_t i = 0; i < kRightHalfSize; ++i) {
      output_frame_[kHalfWindow + i] = time_buffer_[i];
    }

    for (size_t i = 0; i < kWindowSize; ++i) {
      const size_t target = (output_write_pos_ + i) % kOutputRingSize;
      output_ring_[target] += output_frame_[i];
      envelope_ring_[target] += window_[i];
    }

    output_write_pos_ = (output_write_pos_ + HOP_SIZE) % kOutputRingSize;
  }

  Fast_RFFT<FFT_SIZE> fft_;
  float sample_rate_;
  float input_ring_[kWindowSize];
  float output_ring_[kOutputRingSize];
  float envelope_ring_[kOutputRingSize];
  float window_[kWindowSize];
  float envelope_period_[HOP_SIZE];
  float windowed_frame_[kWindowSize];
  float fft_input_[FFT_SIZE];
  float time_buffer_[FFT_SIZE];
  float output_frame_[kWindowSize];
  Complex spectrum_[FFT_SIZE];
  size_t input_write_pos_;
  size_t output_read_pos_;
  size_t output_write_pos_;
  size_t samples_until_frame_;
  float envelope_threshold_;
};

template <size_t FFT_SIZE, size_t HOP_SIZE, size_t BLOCK_SIZE>
class DualStftProcessor {
public:
  static constexpr size_t FftSize() { return FFT_SIZE; }
  static constexpr size_t HopSize() { return HOP_SIZE; }
  static constexpr size_t BlockSize() { return BLOCK_SIZE; }

  DualStftProcessor() : backend_kind_(StftBackendKind::FastStft) {}

  void Init(float sample_rate) {
    fast_.Init(sample_rate);
    dafx_.Init(sample_rate);
    backend_kind_ = StftBackendKind::FastStft;
  }

  void SetBackendKind(StftBackendKind kind) { backend_kind_ = kind; }

  StftBackendKind GetBackendKind() const { return backend_kind_; }

  void ProcessBlock(const float *input, float *output) {
    if (backend_kind_ == StftBackendKind::FastStft) {
      fast_.ProcessBlock(input, output);
    } else {
      dafx_.ProcessBlock(input, output);
    }
  }

  const char *Name() const {
    return backend_kind_ == StftBackendKind::FastStft ? fast_.Name()
                                                      : dafx_.Name();
  }

private:
  FastStftBackend<FFT_SIZE, HOP_SIZE, BLOCK_SIZE> fast_;
  DafxStftEnvBackend<FFT_SIZE, HOP_SIZE, BLOCK_SIZE> dafx_;
  StftBackendKind backend_kind_;
};

} // namespace daisysp

#endif // DSY_DUAL_STFT_H
