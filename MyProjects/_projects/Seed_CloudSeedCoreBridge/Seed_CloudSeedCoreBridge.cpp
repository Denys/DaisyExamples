#include "HardwareMapping.h"

#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace cloudseedbridge;

DaisySeed seed;
ControlModel controls;
SeedMapping  seed_mapping;

/**
 * If CloudSeedCore source is imported under third_party/CloudSeedCore and wired
 * into the Makefile, define USE_CLOUDSEEDCORE=1 to use the real engine.
 * Otherwise we run a ReverbSc fallback so the project still builds/tests.
 */
#if USE_CLOUDSEEDCORE
// TODO: add concrete CloudSeedCore includes and parameter forwarding here.
#endif

class ReverbEngineAdapter
{
  public:
    void Init(float sample_rate)
    {
        reverb_.Init(sample_rate);
        reverb_.SetFeedback(0.84f);
        reverb_.SetLpFreq(12000.0f);
    }

    void SetControlModel(const ControlModel &m)
    {
        // Approximate semantic mapping for fallback engine.
        mix_       = m.mix;
        feedback_  = 0.40f + (m.decay * 0.58f);
        lp_hz_     = 1500.0f + (1.0f - m.damping) * 17000.0f;
        reverb_.SetFeedback(feedback_);
        reverb_.SetLpFreq(lp_hz_);
    }

    inline float Process(float in)
    {
        float wet_l = 0.0f;
        float wet_r = 0.0f;
        reverb_.Process(in, in, &wet_l, &wet_r);
        const float wet = 0.5f * (wet_l + wet_r);
        return (in * (1.0f - mix_)) + (wet * mix_);
    }

  private:
    ReverbSc reverb_;
    float    mix_      = 0.5f;
    float    feedback_ = 0.84f;
    float    lp_hz_    = 12000.0f;
};

ReverbEngineAdapter engine;

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    seed.ProcessAllControls();
    seed_mapping.ReadInto(seed, controls);
    engine.SetControlModel(controls);

    for(size_t i = 0; i < size; i++)
    {
        float mono = 0.5f * (in[0][i] + in[1][i]);
        float y    = engine.Process(mono);
        out[0][i]  = y;
        out[1][i]  = y;
    }
}

int main(void)
{
    seed.Configure();
    seed.Init();
    seed.SetAudioBlockSize(48);

    seed_mapping.Init(seed);

    engine.Init(seed.AudioSampleRate());

    seed.StartAdc();
    seed.StartAudio(AudioCallback);

    while(true)
    {
        System::Delay(10);
    }
}
