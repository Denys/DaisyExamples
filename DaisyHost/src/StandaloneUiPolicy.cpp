#include "daisyhost/StandaloneUiPolicy.h"

#include <algorithm>

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

bool IsEmpty(const daisyhost::UiRect& rect)
{
    return rect.width <= 0 || rect.height <= 0;
}
} // namespace

namespace daisyhost
{
bool IsStandaloneMuteBannerCandidate(const UiRect& editorBounds,
                                     const UiRect& siblingBounds)
{
    if(IsEmpty(editorBounds) || IsEmpty(siblingBounds))
    {
        return false;
    }

    if(siblingBounds.height > 96 || siblingBounds.height >= editorBounds.height)
    {
        return false;
    }

    if(siblingBounds.y > editorBounds.y)
    {
        return false;
    }

    if(BottomOf(siblingBounds) > editorBounds.y + 8)
    {
        return false;
    }

    const int overlapLeft  = std::max(editorBounds.x, siblingBounds.x);
    const int overlapRight = std::min(RightOf(editorBounds), RightOf(siblingBounds));
    const int overlapWidth = std::max(0, overlapRight - overlapLeft);

    return overlapWidth >= (editorBounds.width * 3) / 5;
}

UiRect AdjustEditorBoundsForHiddenMuteBanner(const UiRect& editorBounds,
                                             const UiRect& bannerBounds)
{
    if(!IsStandaloneMuteBannerCandidate(editorBounds, bannerBounds))
    {
        return editorBounds;
    }

    UiRect adjusted = editorBounds;
    const int shift = std::max(0, bannerBounds.height);
    adjusted.y      = std::max(0, adjusted.y - shift);
    adjusted.height = editorBounds.height + (editorBounds.y - adjusted.y);
    return adjusted;
}
} // namespace daisyhost
