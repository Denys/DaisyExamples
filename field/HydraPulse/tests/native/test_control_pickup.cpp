#include "src/core/ControlPickup.h"
#include "tests/native/test_support.h"

int main()
{
    using hydrapulse::core::ControlPickup;

    ControlPickup pickup(0.01f);
    pickup.Reset(0.70f, 0.20f);
    HPF_CHECK(!pickup.Captured());
    HPF_CHECK_NEAR(pickup.Value(), 0.70f, 1.0e-6f);

    HPF_CHECK(!pickup.Update(0.40f));
    HPF_CHECK(!pickup.Update(0.60f));
    HPF_CHECK(!pickup.Captured());

    HPF_CHECK(pickup.Update(0.695f));
    HPF_CHECK(pickup.Captured());
    HPF_CHECK_NEAR(pickup.Value(), 0.695f, 1.0e-6f);

    HPF_CHECK(pickup.Update(0.30f));
    HPF_CHECK_NEAR(pickup.Value(), 0.30f, 1.0e-6f);

    // Crossing the target between samples captures even if neither sample lands inside tolerance.
    pickup.Reset(0.50f, 0.20f);
    HPF_CHECK(!pickup.Update(0.48f));
    HPF_CHECK(pickup.Update(0.52f));
    HPF_CHECK(pickup.Captured());

    // Input and target are clamped to the normalized control domain.
    pickup.Reset(1.5f, 2.0f);
    HPF_CHECK(pickup.Captured());
    HPF_CHECK_NEAR(pickup.Value(), 1.0f, 1.0e-6f);

    return EXIT_SUCCESS;
}
