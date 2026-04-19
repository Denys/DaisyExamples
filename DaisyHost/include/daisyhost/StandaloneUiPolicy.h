#pragma once

namespace daisyhost
{
struct UiRect
{
    int x      = 0;
    int y      = 0;
    int width  = 0;
    int height = 0;
};

bool IsStandaloneMuteBannerCandidate(const UiRect& editorBounds,
                                     const UiRect& siblingBounds);

UiRect AdjustEditorBoundsForHiddenMuteBanner(const UiRect& editorBounds,
                                             const UiRect& bannerBounds);
} // namespace daisyhost
