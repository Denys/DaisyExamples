// Unit tests for dual STFT backends.
// Covers window helpers, identity reconstruction, and callback contract.

#include <gtest/gtest.h>

#include "spectral/dual_stft.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace daisysp;

namespace {

constexpr float kSampleRate = 48000.0f;
constexpr size_t kFftSize = 512;
constexpr size_t kHopSize = 128;
constexpr size_t kBlockSize = 64;

float GenerateSine(size_t index, float frequency) {
  const float phase =
      2.0f * static_cast<float>(M_PI) * frequency *
      static_cast<float>(index) / kSampleRate;
  return 0.25f * std::sin(phase);
}

template <typename Backend, size_t BlockSize>
std::vector<float> ProcessSine(float frequency, size_t sample_count) {
  Backend backend;
  backend.Init(kSampleRate);

  std::vector<float> input(sample_count, 0.0f);
  std::vector<float> output(sample_count, 0.0f);

  float block_in[BlockSize] = {};
  float block_out[BlockSize] = {};

  for (size_t offset = 0; offset < sample_count; offset += BlockSize) {
    for (size_t i = 0; i < BlockSize; ++i) {
      input[offset + i] = GenerateSine(offset + i, frequency);
      block_in[i] = input[offset + i];
    }

    backend.ProcessBlock(block_in, block_out);

    for (size_t i = 0; i < BlockSize; ++i) {
      output[offset + i] = block_out[i];
    }
  }

  return output;
}

float RmsErrorAtLag(const std::vector<float> &reference,
                   const std::vector<float> &actual, size_t lag,
                   size_t fft_size) {
  const size_t start = std::max<size_t>(lag + 4 * fft_size, 6 * fft_size);
  const size_t end = actual.size() - fft_size;

  double sum = 0.0;
  size_t count = 0;
  for (size_t i = start; i < end; ++i) {
    const double error =
        static_cast<double>(actual[i]) - static_cast<double>(reference[i - lag]);
    sum += error * error;
    ++count;
  }

  return count == 0 ? std::numeric_limits<float>::infinity()
                    : static_cast<float>(std::sqrt(sum / count));
}

float BestAlignedRmsError(const std::vector<float> &reference,
                          const std::vector<float> &actual, size_t fft_size) {
  float best = std::numeric_limits<float>::infinity();
  for (size_t lag = 0; lag <= 2 * fft_size; ++lag) {
    best = std::min(best, RmsErrorAtLag(reference, actual, lag, fft_size));
  }
  return best;
}

std::string FormatFloat(float value) {
  std::ostringstream stream;
  stream << std::scientific << std::setprecision(9) << value;
  return stream.str();
}

template <typename Backend, size_t FftSize, size_t BlockSize>
float SineIdentityRms() {
  constexpr size_t kSampleCount = FftSize * 32;
  std::vector<float> reference(kSampleCount, 0.0f);
  for (size_t i = 0; i < kSampleCount; ++i) {
    reference[i] = GenerateSine(i, 440.0f);
  }

  const std::vector<float> actual =
      ProcessSine<Backend, BlockSize>(440.0f, kSampleCount);
  return BestAlignedRmsError(reference, actual, FftSize);
}

template <typename Backend, size_t BlockSize> float ProcessImpulseMaxAbs() {
  Backend backend;
  backend.Init(kSampleRate);

  float block_in[BlockSize] = {};
  float block_out[BlockSize] = {};
  float max_abs = 0.0f;

  for (size_t block = 0; block < 64; ++block) {
    std::fill(std::begin(block_in), std::end(block_in), 0.0f);
    if (block == 0) {
      block_in[0] = 1.0f;
    }

    backend.ProcessBlock(block_in, block_out);

    for (float sample : block_out) {
      EXPECT_TRUE(std::isfinite(sample));
      max_abs = std::max(max_abs, std::fabs(sample));
    }
  }

  return max_abs;
}

} // namespace

TEST(DualStftTest, HannWindowIsDeterministic) {
  float window[8] = {};
  stft::MakeHannWindow(window, 8);

  EXPECT_NEAR(window[0], 0.0f, 1e-6f);
  EXPECT_NEAR(window[2], 0.5f, 1e-6f);
  EXPECT_NEAR(window[4], 1.0f, 1e-6f);
  EXPECT_NEAR(window[6], 0.5f, 1e-6f);
}

TEST(DualStftTest, FastColaGainStabilizesHannOverlap) {
  float window[kFftSize] = {};
  stft::MakeHannWindow(window, kFftSize);

  const float gain = stft::ComputeSquaredColaGain<kFftSize, kHopSize>(window);

  for (size_t i = 0; i < kHopSize; ++i) {
    float sum = 0.0f;
    for (size_t offset = i; offset < kFftSize; offset += kHopSize) {
      sum += window[offset] * window[offset];
    }
    EXPECT_NEAR(sum * gain, 1.0f, 1e-5f);
  }
}

TEST(DualStftTest, DafxEnvelopeCompensationIsStable) {
  float window[kFftSize - 1] = {};
  stft::MakeHannWindow(window, kFftSize - 1);
  stft::NormalizeWindowBySum(window, kFftSize - 1);

  float envelope[kHopSize] = {};
  const float threshold =
      stft::ComputeDafxEnvelope<kFftSize - 1, kHopSize>(window, envelope);

  EXPECT_GT(threshold, 0.0f);
  for (float sample : envelope) {
    EXPECT_GE(sample, threshold);
    EXPECT_TRUE(std::isfinite(sample));
  }
}

TEST(DualStftTest, FastRfftRoundTripsRealSignal) {
  Fast_RFFT<kFftSize> fft;
  fft.Init();

  float input[kFftSize] = {};
  float output[kFftSize] = {};
  Complex spectrum[kFftSize] = {};

  for (size_t i = 0; i < kFftSize; ++i) {
    input[i] = GenerateSine(i, 440.0f);
  }

  fft.Forward(input, spectrum);
  fft.Inverse(spectrum, output);

  double sum = 0.0;
  for (size_t i = 0; i < kFftSize; ++i) {
    const double error =
        static_cast<double>(output[i]) - static_cast<double>(input[i]);
    sum += error * error;
  }

  EXPECT_LT(std::sqrt(sum / static_cast<double>(kFftSize)), 1e-5);
}

TEST(DualStftTest, FastIstftSilenceFrameProducesSilenceHop) {
  Fast_ISTFT<kFftSize, kHopSize> istft;
  istft.Init();

  Complex spectrum[kFftSize] = {};
  float output[kHopSize] = {};

  istft.ProcessFrame(spectrum, output);

  for (float sample : output) {
    EXPECT_NEAR(sample, 0.0f, 1e-7f);
  }
}

TEST(DualStftTest, FastBackendSilenceRemainsSilence) {
  FastStftBackend<kFftSize, kHopSize, kBlockSize> backend;
  backend.Init(kSampleRate);

  float block_in[kBlockSize] = {};
  float block_out[kBlockSize] = {};

  for (size_t block = 0; block < 32; ++block) {
    backend.ProcessBlock(block_in, block_out);
    for (float sample : block_out) {
      EXPECT_NEAR(sample, 0.0f, 1e-7f);
    }
  }
}

TEST(DualStftTest, DafxBackendSilenceRemainsSilence) {
  DafxStftEnvBackend<kFftSize, kHopSize, kBlockSize> backend;
  backend.Init(kSampleRate);

  float block_in[kBlockSize] = {};
  float block_out[kBlockSize] = {};

  for (size_t block = 0; block < 32; ++block) {
    backend.ProcessBlock(block_in, block_out);
    for (float sample : block_out) {
      EXPECT_NEAR(sample, 0.0f, 1e-7f);
    }
  }
}

TEST(DualStftTest, FastBackendReconstructsSineIdentity) {
  using Backend = FastStftBackend<kFftSize, kHopSize, kBlockSize>;
  const float rms = SineIdentityRms<Backend, kFftSize, kBlockSize>();
  RecordProperty("fast_512_128_64_sine_rms", FormatFloat(rms));
  EXPECT_LT(rms, 0.02f);
}

TEST(DualStftTest, DafxBackendReconstructsSineIdentity) {
  using Backend = DafxStftEnvBackend<kFftSize, kHopSize, kBlockSize>;
  const float rms = SineIdentityRms<Backend, kFftSize, kBlockSize>();
  RecordProperty("dafx_512_128_64_sine_rms", FormatFloat(rms));
  EXPECT_LT(rms, 0.02f);
}

TEST(DualStftTest, FastBackend1024ReconstructsSineIdentity) {
  using Backend = FastStftBackend<1024, 256, kBlockSize>;
  const float rms = SineIdentityRms<Backend, 1024, kBlockSize>();
  RecordProperty("fast_1024_256_64_sine_rms", FormatFloat(rms));
  EXPECT_LT(rms, 0.02f);
}

TEST(DualStftTest, DafxBackend1024ReconstructsSineIdentity) {
  using Backend = DafxStftEnvBackend<1024, 256, kBlockSize>;
  const float rms = SineIdentityRms<Backend, 1024, kBlockSize>();
  RecordProperty("dafx_1024_256_64_sine_rms", FormatFloat(rms));
  EXPECT_LT(rms, 0.02f);
}

TEST(DualStftTest, ImpulseOutputStaysBounded) {
  using FastBackend = FastStftBackend<kFftSize, kHopSize, kBlockSize>;
  using DafxBackend = DafxStftEnvBackend<kFftSize, kHopSize, kBlockSize>;

  const float fast_max = ProcessImpulseMaxAbs<FastBackend, kBlockSize>();
  const float dafx_max = ProcessImpulseMaxAbs<DafxBackend, kBlockSize>();

  EXPECT_LT(fast_max, 2.0f);
  EXPECT_LT(dafx_max, 2.0f);
}

TEST(DualStftTest, BackendSwitchPreservesBlockContract) {
  DualStftProcessor<kFftSize, kHopSize, kBlockSize> processor;
  processor.Init(kSampleRate);

  float block_in[kBlockSize] = {};
  float block_out[kBlockSize] = {};

  EXPECT_EQ(processor.GetBackendKind(), StftBackendKind::FastStft);
  EXPECT_STREQ(processor.Name(), "FastStftBackend");
  processor.ProcessBlock(block_in, block_out);

  processor.SetBackendKind(StftBackendKind::DafxStftEnv);
  EXPECT_EQ(processor.GetBackendKind(), StftBackendKind::DafxStftEnv);
  EXPECT_STREQ(processor.Name(), "DafxStftEnvBackend");
  processor.ProcessBlock(block_in, block_out);

  EXPECT_EQ(processor.BlockSize(), kBlockSize);
}
