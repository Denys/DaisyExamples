#include <gtest/gtest.h>

#include "daisyhost/DaisyFieldRSPLayout.h"

namespace
{
int RightOf(const daisyhost::UiRect& rect)
{
    return rect.x + rect.width;
}

int BottomOf(const daisyhost::UiRect& rect)
{
    return rect.y + rect.height;
}

bool Overlaps(const daisyhost::UiRect& left, const daisyhost::UiRect& right)
{
    return left.x < RightOf(right) && RightOf(left) > right.x
           && left.y < BottomOf(right) && BottomOf(left) > right.y;
}
} // namespace

TEST(DaisyFieldRSPLayoutTest, CvGeneratorCardsUseReadableTwoByTwoGrid)
{
    const daisyhost::UiRect area{0, 0, 410, 360};
    const auto cards = daisyhost::BuildDaisyFieldCvGeneratorCardLayout(area);

    EXPECT_GE(cards[0].card.width, 190);
    EXPECT_GE(cards[0].card.height, 165);
    EXPECT_EQ(cards[0].card.y, cards[1].card.y);
    EXPECT_EQ(cards[2].card.y, cards[3].card.y);
    EXPECT_EQ(cards[0].card.x, cards[2].card.x);
    EXPECT_EQ(cards[1].card.x, cards[3].card.x);
    EXPECT_GT(cards[2].card.y, cards[0].card.y);
    EXPECT_GT(cards[1].card.x, cards[0].card.x);

    for(const auto& card : cards)
    {
        EXPECT_GE(card.mode.width, 74);
        EXPECT_GE(card.waveform.width, 96);
        EXPECT_GE(card.target.width, 170);
        for(const auto& slider : card.sliders)
        {
            EXPECT_GE(slider.width, 48);
            EXPECT_GT(slider.height, 62);
        }
        EXPECT_LE(RightOf(card.card), RightOf(area));
        EXPECT_LE(BottomOf(card.card), BottomOf(area));
    }
}

TEST(DaisyFieldRSPLayoutTest, KeyMappingLegendRowsStayReadableInsideBox)
{
    const daisyhost::UiRect panel{0, 0, 854, 471};
    const auto layout = daisyhost::BuildDaisyFieldKeyMappingLegendLayout(panel);

    EXPECT_GE(layout.titleFontHeight, 11.0f);
    EXPECT_GE(layout.rowFontHeight, 9.5f);
    EXPECT_GE(layout.mappingBox.width, 350);
    EXPECT_GE(layout.mappingBox.height, 86);

    for(const auto& row : layout.rows)
    {
        EXPECT_GE(row.height, 14);
        EXPECT_GE(row.x, layout.mappingBox.x + 8);
        EXPECT_GE(row.y, layout.mappingBox.y + 6);
        EXPECT_LE(RightOf(row), RightOf(layout.mappingBox) - 8);
        EXPECT_LE(BottomOf(row), BottomOf(layout.mappingBox) - 6);
    }

    for(std::size_t i = 1; i < layout.rows.size(); ++i)
    {
        EXPECT_FALSE(Overlaps(layout.rows[i - 1], layout.rows[i]));
        EXPECT_LE(BottomOf(layout.rows[i - 1]), layout.rows[i].y);
    }
}
