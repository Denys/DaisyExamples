#include "daisyhost/DaisyFieldRSPLayout.h"

#include <algorithm>

namespace daisyhost
{
namespace
{
UiRect Inset(const UiRect& rect, int dx, int dy)
{
    return {rect.x + dx,
            rect.y + dy,
            std::max(0, rect.width - dx * 2),
            std::max(0, rect.height - dy * 2)};
}

UiRect TakeTop(UiRect& rect, int height)
{
    const int taken = std::min(height, rect.height);
    UiRect result{rect.x, rect.y, rect.width, taken};
    rect.y += taken;
    rect.height -= taken;
    return result;
}

void DropTop(UiRect& rect, int height)
{
    const int dropped = std::min(height, rect.height);
    rect.y += dropped;
    rect.height -= dropped;
}
} // namespace

std::array<DaisyFieldCvGeneratorCardLayout, 4>
BuildDaisyFieldCvGeneratorCardLayout(const UiRect& area)
{
    std::array<DaisyFieldCvGeneratorCardLayout, 4> layouts{};
    constexpr int kGap = 10;
    const int columnWidth = std::max(0, (area.width - kGap) / 2);
    const int rowHeight = std::max(0, (area.height - kGap) / 2);

    for(std::size_t i = 0; i < layouts.size(); ++i)
    {
        const int col = static_cast<int>(i % 2);
        const int row = static_cast<int>(i / 2);
        auto& layout = layouts[i];
        layout.card = {area.x + col * (columnWidth + kGap),
                       area.y + row * (rowHeight + kGap),
                       columnWidth,
                       rowHeight};

        auto content = Inset(layout.card, 6, 5);
        layout.title = TakeTop(content, 18);
        DropTop(content, 2);

        auto modeRow = TakeTop(content, 24);
        const int modeWidth = std::max(74, (modeRow.width * 42) / 100);
        layout.mode = {modeRow.x, modeRow.y, std::min(modeWidth, modeRow.width), modeRow.height};
        layout.waveform = {layout.mode.x + layout.mode.width + 4,
                           modeRow.y,
                           std::max(0, modeRow.width - layout.mode.width - 4),
                           modeRow.height};

        DropTop(content, 4);
        layout.target = TakeTop(content, 24);
        DropTop(content, 6);

        auto sliderRow = TakeTop(content, std::max(0, content.height - 18));
        constexpr int kSliderGap = 6;
        const int sliderWidth = std::max(0, (sliderRow.width - kSliderGap * 2) / 3);
        for(std::size_t slider = 0; slider < layout.sliders.size(); ++slider)
        {
            layout.sliders[slider] = {
                sliderRow.x + static_cast<int>(slider) * (sliderWidth + kSliderGap),
                sliderRow.y,
                slider == layout.sliders.size() - 1
                    ? std::max(0,
                               sliderRow.width
                                   - static_cast<int>(slider) * (sliderWidth + kSliderGap))
                    : sliderWidth,
                sliderRow.height};
        }

        auto labelRow = TakeTop(content, 16);
        const int labelWidth = std::max(0, (labelRow.width - kSliderGap * 2) / 3);
        for(std::size_t label = 0; label < layout.sliderLabels.size(); ++label)
        {
            layout.sliderLabels[label] = {
                labelRow.x + static_cast<int>(label) * (labelWidth + kSliderGap),
                labelRow.y,
                label == layout.sliderLabels.size() - 1
                    ? std::max(0,
                               labelRow.width
                                   - static_cast<int>(label) * (labelWidth + kSliderGap))
                    : labelWidth,
                labelRow.height};
        }
    }
    return layouts;
}

DaisyFieldKeyMappingLegendLayout
BuildDaisyFieldKeyMappingLegendLayout(const UiRect& panelBounds)
{
    DaisyFieldKeyMappingLegendLayout layout;
    layout.mappingBox = {panelBounds.x + static_cast<int>(panelBounds.width * 0.018f),
                         panelBounds.y + static_cast<int>(panelBounds.height * 0.842f),
                         std::max(360, static_cast<int>(panelBounds.width * 0.435f)),
                         std::max(106, static_cast<int>(panelBounds.height * 0.125f))};
    layout.titleFontHeight = 11.5f;
    layout.rowFontHeight   = 9.8f;

    const int left = layout.mappingBox.x + 10;
    int       y    = layout.mappingBox.y + 7;
    const int width = std::max(0, layout.mappingBox.width - 20);
    layout.rows[0] = {left, y, width, 17};
    y += 18;
    for(std::size_t row = 1; row < layout.rows.size(); ++row)
    {
        layout.rows[row] = {left, y, width, 15};
        y += 16;
    }
    return layout;
}
} // namespace daisyhost
