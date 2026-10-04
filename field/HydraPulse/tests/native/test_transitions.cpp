#include "src/app/Engine.h"
#include "src/dsp/Voices.h"
#include "tests/native/test_support.h"
#include <algorithm>
#include <cmath>

int main()
{
    using namespace hydrapulse;
    dsp::SineTable table;
    table.Init();
    float max_retrigger_error = 0, max_parameter_error = 0;
    for (unsigned id = 0; id < 4; ++id)
    {
        for (int offset : {64, 127, 313, 511, 997})
        {
            dsp::PercussionVoice a;
            a.Init(static_cast<dsp::VoiceId>(id), 48000, table);
            a.SetParameters({.35f, .85f, .95f, 1}, true);
            a.Trigger(1);
            for (int i = 0; i < offset; ++i)
                (void)a.Process();
            auto reference = a;
            a.Trigger(1);
            max_retrigger_error = std::max(max_retrigger_error, std::fabs(a.Process() - reference.Process()));
            a = reference;
            reference = a;
            a.SetParameters({.8f, .1f, 0, .1f});
            max_parameter_error = std::max(max_parameter_error, std::fabs(a.Process() - reference.Process()));
        }
    }
    // Compare to a matched no-event trajectory, not raw adjacent samples of a noisy drum.
    std::cout << "retrigger first-sample deviation " << max_retrigger_error
              << "; parameter first-sample deviation " << max_parameter_error << '\n';
    HPF_CHECK(max_retrigger_error < .05f);
    HPF_CHECK(max_parameter_error < .05f);
    // Repeated Stop must not restart release and extend a five-millisecond mute indefinitely.
    dsp::PercussionVoice v;
    v.Init(dsp::VoiceId::Arc, 48000, table);
    v.Trigger(1);
    for (int i = 0; i < 300; ++i)
        (void)v.Process();
    for (int i = 0; i < 300; ++i)
    {
        v.Release();
        (void)v.Process();
    }
    HPF_CHECK_EQ(v.Process(), 0.0f);
    // Loading during an audible stopped tail must stay bounded and end in the new preset.
    Engine e;
    e.Init();
    e.Trigger(3, 1);
    for (int i = 0; i < 300; ++i)
        (void)e.Process();
    HPF_CHECK(e.RequestPreset(7));
    for (int i = 0; i < 600; ++i)
    {
        const auto s = e.Process();
        HPF_CHECK(std::isfinite(s.left) && std::fabs(s.left) < .737f);
    }
    HPF_CHECK_EQ(unsigned(e.PresetId()), 7u);
    return EXIT_SUCCESS;
}
