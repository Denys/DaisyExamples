#include "src/app/Controller.h"
#include "tests/native/test_support.h"
int main()
{
    using namespace hydrapulse;
    Engine e;
    e.Init();
    Controller c;
    InputFrame f{};
    c.Init(e, f);
    // Tap a step: action happens on release.
    f.keys = 1;
    c.Tick(e, f);
    HPF_CHECK_EQ(e.Notes(Bank::A, 0), 0u);
    f.keys = 0;
    c.Tick(e, f);
    HPF_CHECK_EQ(e.Notes(Bank::A, 0), 1u);
    // Long press changes accent once and never toggles the step on release.
    f.keys = 1;
    for (unsigned i = 0; i < 450; ++i)
        c.Tick(e, f);
    f.keys = 0;
    c.Tick(e, f);
    HPF_CHECK_EQ(e.Accents(Bank::A, 0), 1u);
    HPF_CHECK_EQ(e.Notes(Bank::A, 0), 1u);
    // Modifier key held across Shift release must not leak a step.
    f.shift = true;
    c.Tick(e, f);
    f.keys = 2;
    c.Tick(e, f);
    HPF_CHECK_EQ(unsigned(c.Snapshot(e).selected), 1u);
    f.shift = false;
    c.Tick(e, f);
    f.keys = 0;
    c.Tick(e, f);
    HPF_CHECK_EQ(e.Notes(Bank::A, 1), 0u);
    // Play is a tap-release; panic chord must not become a Play release.
    f.play = true;
    c.Tick(e, f);
    f.play = false;
    c.Tick(e, f);
    HPF_CHECK(e.Running());
    f.play = true;
    f.shift = true;
    for (int i = 0; i < 510; ++i)
        c.Tick(e, f);
    HPF_CHECK(!e.Running());
    f.shift = false;
    c.Tick(e, f);
    f.play = false;
    c.Tick(e, f);
    HPF_CHECK(!e.Running());
    // Preview does not load. Confirm while stopped performs a fade transaction.
    f.shift = true;
    f.keys = std::uint16_t(1u << 14);
    c.Tick(e, f);
    HPF_CHECK(c.Snapshot(e).preset_armed);
    HPF_CHECK_EQ(unsigned(e.PresetId()), 0u);
    f.keys = 0;
    c.Tick(e, f);
    f.keys = std::uint16_t(1u << 15);
    c.Tick(e, f);
    HPF_CHECK(e.Loading());
    f.keys = 0;
    f.shift = false;
    for (int i = 0; i < 1000; ++i)
        (void)e.Process();
    c.Tick(e, f);
    HPF_CHECK_EQ(unsigned(e.PresetId()), 1u);
    // Selecting a voice with knobs at zero does not erase its stored parameters.
    auto before = e.Voice(2);
    f.shift = true;
    f.keys = 4;
    c.Tick(e, f);
    f.shift = false;
    c.Tick(e, f);
    f.keys = 0;
    c.Tick(e, f);
    HPF_CHECK_NEAR(e.Voice(2).tune, before.tune, 1e-6);
    HPF_CHECK_EQ(e.Notes(Bank::A, 2), 0u);
    return EXIT_SUCCESS;
}
