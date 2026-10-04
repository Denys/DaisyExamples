#include "src/app/Engine.h"
#include "tests/native/test_support.h"
#include <cmath>
int main()
{
    using namespace hydrapulse;
    Engine e;
    e.Init();
    HPF_CHECK(!e.Running());
    for (int i = 0; i < 100; ++i)
    {
        auto a = e.Process();
        HPF_CHECK(a.left == 0 && a.right == 0);
    }
    HPF_CHECK(e.RequestPreset(2));
    HPF_CHECK(!e.RequestPreset(3));
    for (int i = 0; i < 600; ++i)
        (void)e.Process();
    HPF_CHECK_EQ(unsigned(e.PresetId()), 2u);
    e.SetTempo(120);
    e.Start();
    (void)e.Process();
    HPF_CHECK_EQ(e.StepEvents(), 1u);
    HPF_CHECK_EQ(unsigned(e.CurrentStep()), 0u);
    e.QueueBank(Bank::B);
    HPF_CHECK(e.ActiveBank() == Bank::A);
    for (unsigned i = 1; i <= 16u * 6000u; ++i)
        (void)e.Process();
    HPF_CHECK(e.ActiveBank() == Bank::B);
    HPF_CHECK_EQ(unsigned(e.CurrentStep()), 0u);
    e.SetFill(true);
    for (int i = 0; i < 6000; ++i)
        (void)e.Process();
    HPF_CHECK(e.PlayingBank() == Bank::Fill);
    e.SetFill(false);
    for (int i = 0; i < 6000; ++i)
        (void)e.Process();
    HPF_CHECK(e.PlayingBank() == Bank::B);
    HPF_CHECK(!e.RequestPreset(0));
    HPF_CHECK(!e.ClearTrack(Bank::A, 0));
    e.Panic();
    HPF_CHECK(!e.Running());
    float peak = 0;
    for (int i = 0; i < 48000; ++i)
    {
        auto a = e.Process();
        if (i > 40000)
            peak = std::max(peak, std::fabs(a.left) + std::fabs(a.right));
    }
    HPF_CHECK(peak < 1e-7f);
    HPF_CHECK(!e.ToggleStep(static_cast<Bank>(9), 0, 0));
    HPF_CHECK(!e.ToggleStep(Bank::A, 4, 0));
    HPF_CHECK(!e.ToggleAccent(Bank::A, 0, 16));
    HPF_CHECK(!e.RequestPreset(8));
    e.SetVoice(0, {NAN, INFINITY, -INFINITY, NAN});
    e.Trigger(0, NAN);
    for (int i = 0; i < 100; ++i)
        HPF_CHECK(std::isfinite(e.Process().left));
    HPF_CHECK_EQ(e.Faults(), 0u);
    return EXIT_SUCCESS;
}
