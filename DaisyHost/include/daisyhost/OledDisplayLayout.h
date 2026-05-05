#pragma once

#include <string>
#include <vector>

#include "daisyhost/DisplayModel.h"

namespace daisyhost
{
struct OledDisplayTextLayout
{
    int         x        = 0;
    int         y        = 0;
    int         width    = 0;
    int         height   = 0;
    std::string text;
    bool        inverted = false;
};

struct OledDisplayBarLayout
{
    int   x          = 0;
    int   y          = 0;
    int   width      = 0;
    int   height     = 0;
    float normalized = 0.0f;
};

struct OledDisplayLayout
{
    int                                width  = 128;
    int                                height = 64;
    std::vector<OledDisplayTextLayout> texts;
    std::vector<OledDisplayBarLayout>  bars;
};

OledDisplayLayout BuildOledDisplayLayout(const DisplayModel& display,
                                         int                 width = 128,
                                         int                 height = 64);
} // namespace daisyhost
