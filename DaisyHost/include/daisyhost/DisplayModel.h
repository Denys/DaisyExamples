#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace daisyhost
{
struct DisplayText
{
    int         x        = 0;
    int         y        = 0;
    std::string text;
    bool        inverted = false;
};

struct DisplayBar
{
    int   x          = 0;
    int   y          = 0;
    int   width      = 0;
    int   height     = 0;
    float normalized = 0.0f;
};

struct DisplayModel
{
    int                      width    = 128;
    int                      height   = 64;
    std::uint64_t            revision = 0;
    std::vector<DisplayText> texts;
    std::vector<DisplayBar>  bars;
};
} // namespace daisyhost
