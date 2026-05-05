#include "daisyhost/OledDisplayLayout.h"

#include <algorithm>

namespace daisyhost
{
namespace
{
int ClampInt(int value, int minValue, int maxValue)
{
    return std::max(minValue, std::min(maxValue, value));
}

float Clamp01(float value)
{
    return std::max(0.0f, std::min(1.0f, value));
}
} // namespace

OledDisplayLayout BuildOledDisplayLayout(const DisplayModel& display,
                                         int                 width,
                                         int                 height)
{
    OledDisplayLayout layout;
    layout.width  = std::max(1, width);
    layout.height = std::max(1, height);
    layout.texts.reserve(display.texts.size());
    layout.bars.reserve(display.bars.size());

    for(const auto& bar : display.bars)
    {
        const int x = ClampInt(bar.x, 0, layout.width - 1);
        const int y = ClampInt(bar.y, 0, layout.height - 1);
        layout.bars.push_back({x,
                               y,
                               ClampInt(bar.width, 0, layout.width - x),
                               ClampInt(bar.height, 0, layout.height - y),
                               Clamp01(bar.normalized)});
    }

    for(std::size_t index = 0; index < display.texts.size(); ++index)
    {
        const auto& text = display.texts[index];
        const int   x    = ClampInt(text.x, 0, layout.width - 1);
        const int   y    = ClampInt(text.y, 0, layout.height - 1);

        int nextDistinctY = layout.height;
        for(std::size_t next = index + 1; next < display.texts.size(); ++next)
        {
            if(display.texts[next].y > text.y)
            {
                nextDistinctY = display.texts[next].y;
                break;
            }
        }

        const int availableHeight = ClampInt(nextDistinctY - y, 8, 12);
        layout.texts.push_back({x,
                                y,
                                layout.width - x,
                                std::min(availableHeight, layout.height - y),
                                text.text,
                                text.inverted});
    }

    return layout;
}
} // namespace daisyhost
