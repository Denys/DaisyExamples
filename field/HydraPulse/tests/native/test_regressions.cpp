#include "src/core/ControlPickup.h"
#include "src/core/OutputTestArm.h"
#include "src/core/SampleAccurateStepClock.h"
#include "tests/native/test_support.h"
#include <limits>
int main()
{
    using namespace hydrapulse::core;
    SampleAccurateStepClock c;
    for (unsigned i = 0; i < 5999; ++i)
    {
        HPF_CHECK(c.SetTempoBpm(120));
        HPF_CHECK(!c.ProcessSample());
    }
    HPF_CHECK(c.SetTempoBpm(120));
    HPF_CHECK(c.ProcessSample());
    SampleAccurateStepClock invalid(0, 0, 0);
    HPF_CHECK_EQ(invalid.SampleRate(), 48000u);
    HPF_CHECK_EQ(invalid.StepsPerBeat(), 4u);
    HPF_CHECK_EQ(invalid.SamplesUntilNextStep(), 6000u);
    HPF_CHECK(!c.SetTempoBpm(0.0000001));
    HPF_CHECK(!c.SetTempoBpm(std::numeric_limits<double>::infinity()));
    HPF_CHECK(!c.SetTempoBpm(std::numeric_limits<double>::quiet_NaN()));
    HPF_CHECK_NEAR(c.TempoBpm(), 120, 1e-6);
    ControlPickup p;
    p.Reset(.6f, .1f);
    HPF_CHECK(!p.Update(std::numeric_limits<float>::quiet_NaN()));
    HPF_CHECK_NEAR(p.Value(), .6f, 1e-6);
    HPF_CHECK(p.Update(.7f));
    HPF_CHECK(!p.Update(std::numeric_limits<float>::infinity()));
    HPF_CHECK_NEAR(p.Value(), .7f, 1e-6);
    OutputTestArm arm;
    HPF_CHECK(!arm.Update(true, false));
    HPF_CHECK(!arm.Update(true, false));
    HPF_CHECK(!arm.Update(false, false));
    HPF_CHECK(arm.Update(true, false));
    HPF_CHECK(!arm.Update(true, true));
    HPF_CHECK(!arm.Update(false, false));
    return EXIT_SUCCESS;
}
