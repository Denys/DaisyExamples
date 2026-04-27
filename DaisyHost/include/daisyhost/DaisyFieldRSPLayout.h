#pragma once

#include <array>

#include "daisyhost/StandaloneUiPolicy.h"

namespace daisyhost
{
struct DaisyFieldCvGeneratorCardLayout
{
    UiRect card;
    UiRect title;
    UiRect mode;
    UiRect waveform;
    UiRect target;
    std::array<UiRect, 3> sliders{};
    std::array<UiRect, 3> sliderLabels{};
};

struct DaisyFieldKeyMappingLegendLayout
{
    UiRect mappingBox;
    std::array<UiRect, 5> rows{};
    float titleFontHeight = 11.0f;
    float rowFontHeight   = 9.5f;
};

std::array<DaisyFieldCvGeneratorCardLayout, 4>
BuildDaisyFieldCvGeneratorCardLayout(const UiRect& area);
DaisyFieldKeyMappingLegendLayout
BuildDaisyFieldKeyMappingLegendLayout(const UiRect& panelBounds);
} // namespace daisyhost
