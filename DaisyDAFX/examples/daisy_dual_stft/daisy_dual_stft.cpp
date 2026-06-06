// Daisy Seed dual STFT comparison firmware.
//
// Compile-time options:
// - default: Daisy Seed, FastStftBackend, 512/128/64
// - STFT_BOARD_FIELD: use Daisy Field board initialization
// - STFT_BACKEND_FAST / STFT_BACKEND_DAFX: select backend
// - STFT_PROFILE_512 / STFT_PROFILE_1024: select FFT/hop profile

#if defined(STFT_BOARD_FIELD)
#include "daisy_field.h"
#else
#include "daisy_seed.h"
#endif
#include "spectral/dual_stft.h"
#include "util/CpuLoadMeter.h"

using namespace daisy;
using namespace daisysp;

#if defined(STFT_BACKEND_FAST) && defined(STFT_BACKEND_DAFX)
#error "Define only one backend: STFT_BACKEND_FAST or STFT_BACKEND_DAFX."
#endif

#if defined(STFT_PROFILE_512) && defined(STFT_PROFILE_1024)
#error "Define only one profile: STFT_PROFILE_512 or STFT_PROFILE_1024."
#endif

#if defined(STFT_PROFILE_1024)
constexpr size_t FFT_SIZE = 1024;
constexpr size_t HOP_SIZE = 256;
#else
constexpr size_t FFT_SIZE = 512;
constexpr size_t HOP_SIZE = 128;
#endif

constexpr size_t BLOCK_SIZE = 64;

#if defined(STFT_BACKEND_DAFX)
using ActiveStftBackend = DafxStftEnvBackend<FFT_SIZE, HOP_SIZE, BLOCK_SIZE>;
constexpr StftBackendKind BACKEND_KIND = StftBackendKind::DafxStftEnv;
#elif defined(STFT_BACKEND_FAST) || !defined(STFT_BACKEND_DAFX)
using ActiveStftBackend = FastStftBackend<FFT_SIZE, HOP_SIZE, BLOCK_SIZE>;
constexpr StftBackendKind BACKEND_KIND = StftBackendKind::FastStft;
#endif

#if defined(STFT_BOARD_FIELD)
static DaisyField hw;
#else
static DaisySeed hw;
#endif
static CpuLoadMeter cpu_meter;
static ActiveStftBackend stft_processor;
static float mono_in[BLOCK_SIZE];
static float mono_out[BLOCK_SIZE];

template <typename... Args> static void LogLine(const char *fmt, Args... args) {
#if defined(STFT_BOARD_FIELD)
  hw.seed.PrintLine(fmt, args...);
#else
  hw.PrintLine(fmt, args...);
#endif
}

static const char *BackendKindName() {
  return BACKEND_KIND == StftBackendKind::FastStft ? "FastStftBackend"
                                                   : "DafxStftEnvBackend";
}

#if defined(STFT_BOARD_FIELD)
static void AudioCallback(AudioHandle::InputBuffer in,
                          AudioHandle::OutputBuffer out, size_t size) {
  cpu_meter.OnBlockStart();

  for (size_t frame = 0; frame < BLOCK_SIZE; ++frame) {
    mono_in[frame] = frame < size ? in[0][frame] : 0.0f;
  }

  stft_processor.ProcessBlock(mono_in, mono_out);

  for (size_t frame = 0; frame < BLOCK_SIZE; ++frame) {
    if (frame < size) {
      out[0][frame] = mono_out[frame];
      out[1][frame] = mono_out[frame];
    }
  }

  cpu_meter.OnBlockEnd();
}
#else
static void AudioCallback(AudioHandle::InterleavingInputBuffer in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t size) {
  cpu_meter.OnBlockStart();

  for (size_t frame = 0; frame < BLOCK_SIZE; ++frame) {
    const size_t interleaved = frame * 2;
    mono_in[frame] = interleaved < size ? in[interleaved] : 0.0f;
  }

  stft_processor.ProcessBlock(mono_in, mono_out);

  for (size_t frame = 0; frame < BLOCK_SIZE; ++frame) {
    const size_t interleaved = frame * 2;
    if (interleaved + 1 < size) {
      out[interleaved] = mono_out[frame];
      out[interleaved + 1] = mono_out[frame];
    }
  }

  cpu_meter.OnBlockEnd();
}
#endif

int main(void) {
#if defined(STFT_BOARD_FIELD)
  hw.Init();
#else
  hw.Configure();
  hw.Init();
#endif
  hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
  hw.SetAudioBlockSize(BLOCK_SIZE);
#if defined(STFT_BOARD_FIELD)
  hw.seed.StartLog(false);
#else
  hw.StartLog(false);
#endif

  const float sample_rate = hw.AudioSampleRate();
  stft_processor.Init(sample_rate);
  cpu_meter.Init(sample_rate, static_cast<int>(BLOCK_SIZE));

  LogLine("DaisyDAFX dual STFT");
#if defined(STFT_BOARD_FIELD)
  LogLine("Board: Daisy Field");
#else
  LogLine("Board: Daisy Seed");
#endif
  LogLine("Backend: %s", BackendKindName());
  LogLine("FFT/Hop/Block: %u/%u/%u", static_cast<unsigned int>(FFT_SIZE),
          static_cast<unsigned int>(HOP_SIZE),
          static_cast<unsigned int>(BLOCK_SIZE));
  LogLine("Sample rate: " FLT_FMT3, FLT_VAR3(sample_rate));

#if defined(STFT_BOARD_FIELD)
  hw.StartAdc();
#endif
  hw.StartAudio(AudioCallback);

  while (true) {
    System::Delay(1000);
    LogLine("CPU max/avg/min %%: " FLT_FMT3 " / " FLT_FMT3 " / " FLT_FMT3,
            FLT_VAR3(cpu_meter.GetMaxCpuLoad() * 100.0f),
            FLT_VAR3(cpu_meter.GetAvgCpuLoad() * 100.0f),
            FLT_VAR3(cpu_meter.GetMinCpuLoad() * 100.0f));
  }
}
