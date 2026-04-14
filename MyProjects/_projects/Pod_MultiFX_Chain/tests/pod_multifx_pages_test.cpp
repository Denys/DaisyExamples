#include "../pod_multifx_pages.h"

#include <cassert>
#include <cmath>

namespace
{
bool NearlyEqual(float a, float b, float epsilon = 1.0e-6f)
{
    return std::fabs(a - b) <= epsilon;
}
}

int main()
{
    using namespace pod_multifx;

    {
        ControlState state = MakeDefaultControlState();
        assert(state.selected_page == FX_OVERDRIVE);

        SelectPageDelta(state, 1);
        assert(state.selected_page == FX_DELAY);

        SelectPageDelta(state, 3);
        assert(state.selected_page == FX_OVERDRIVE);

        SelectPageDelta(state, -1);
        assert(state.selected_page == FX_WAHWAH);
    }

    {
        ControlState state = MakeDefaultControlState();
        const float   saved_delay_time = state.pages[FX_DELAY].knob_values[0];
        const float   saved_delay_fb   = state.pages[FX_DELAY].knob_values[1];

        SelectPageDelta(state, 1);
        assert(state.selected_page == FX_DELAY);
        assert(!state.pages[FX_DELAY].captured[0]);
        assert(!state.pages[FX_DELAY].captured[1]);

        UpdateCurrentPageKnobs(state, 0.90f, 0.10f);
        assert(NearlyEqual(state.pages[FX_DELAY].knob_values[0], saved_delay_time));
        assert(NearlyEqual(state.pages[FX_DELAY].knob_values[1], saved_delay_fb));
        assert(!state.pages[FX_DELAY].captured[0]);
        assert(!state.pages[FX_DELAY].captured[1]);

        UpdateCurrentPageKnobs(state, saved_delay_time + 0.01f, saved_delay_fb - 0.01f);
        assert(state.pages[FX_DELAY].captured[0]);
        assert(state.pages[FX_DELAY].captured[1]);
        assert(NearlyEqual(state.pages[FX_DELAY].knob_values[0], saved_delay_time + 0.01f));
        assert(NearlyEqual(state.pages[FX_DELAY].knob_values[1], saved_delay_fb - 0.01f));
    }

    {
        ControlState state = MakeDefaultControlState();

        UpdateCurrentPageKnobs(state, 0.62f, 0.41f);
        assert(NearlyEqual(state.pages[FX_OVERDRIVE].knob_values[0], 0.62f));
        assert(NearlyEqual(state.pages[FX_OVERDRIVE].knob_values[1], 0.41f));

        SelectPageDelta(state, 1);
        UpdateCurrentPageKnobs(state,
                               state.pages[FX_DELAY].knob_values[0],
                               state.pages[FX_DELAY].knob_values[1]);
        UpdateCurrentPageKnobs(state, 0.33f, 0.77f);
        assert(NearlyEqual(state.pages[FX_DELAY].knob_values[0], 0.33f));
        assert(NearlyEqual(state.pages[FX_DELAY].knob_values[1], 0.77f));

        SelectPageDelta(state, -1);
        assert(state.selected_page == FX_OVERDRIVE);
        assert(NearlyEqual(state.pages[FX_OVERDRIVE].knob_values[0], 0.62f));
        assert(NearlyEqual(state.pages[FX_OVERDRIVE].knob_values[1], 0.41f));
        assert(!state.pages[FX_OVERDRIVE].captured[0]);
        assert(!state.pages[FX_OVERDRIVE].captured[1]);
    }

    {
        ControlState state = MakeDefaultControlState();
        assert(!state.pages[FX_OVERDRIVE].bypassed);
        assert(!state.global_bypass);

        ToggleCurrentPageBypass(state);
        assert(state.pages[FX_OVERDRIVE].bypassed);

        ToggleGlobalBypass(state);
        assert(state.global_bypass);

        SelectPageDelta(state, 1);
        ToggleCurrentPageBypass(state);
        assert(state.pages[FX_DELAY].bypassed);
        assert(state.pages[FX_OVERDRIVE].bypassed);
        assert(state.global_bypass);
    }

    return 0;
}
