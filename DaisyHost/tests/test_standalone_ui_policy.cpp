#include <gtest/gtest.h>

#include "daisyhost/StandaloneUiPolicy.h"

namespace
{
TEST(StandaloneUiPolicyTest, DetectsTopBannerAboveEditor)
{
    const daisyhost::UiRect editor{0, 56, 1360, 900};
    const daisyhost::UiRect banner{0, 28, 1360, 28};

    EXPECT_TRUE(
        daisyhost::IsStandaloneMuteBannerCandidate(editor, banner));

    const auto adjusted
        = daisyhost::AdjustEditorBoundsForHiddenMuteBanner(editor, banner);
    EXPECT_EQ(adjusted.x, 0);
    EXPECT_EQ(adjusted.y, 28);
    EXPECT_EQ(adjusted.width, 1360);
    EXPECT_EQ(adjusted.height, 928);
}

TEST(StandaloneUiPolicyTest, RejectsLargeContentSiblingThatWouldMeltLayout)
{
    const daisyhost::UiRect editor{0, 56, 1360, 900};
    const daisyhost::UiRect contentPeer{0, 0, 1360, 980};

    EXPECT_FALSE(
        daisyhost::IsStandaloneMuteBannerCandidate(editor, contentPeer));
    EXPECT_EQ(daisyhost::AdjustEditorBoundsForHiddenMuteBanner(editor, contentPeer).y,
              editor.y);
    EXPECT_EQ(
        daisyhost::AdjustEditorBoundsForHiddenMuteBanner(editor, contentPeer).height,
        editor.height);
}

TEST(StandaloneUiPolicyTest, RejectsNarrowSideDrawerSibling)
{
    const daisyhost::UiRect editor{0, 56, 1360, 900};
    const daisyhost::UiRect sidePeer{1160, 28, 200, 28};

    EXPECT_FALSE(
        daisyhost::IsStandaloneMuteBannerCandidate(editor, sidePeer));
}
} // namespace
