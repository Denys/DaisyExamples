#include <string>

#include <gtest/gtest.h>

#include "daisyhost/FieldOledTransient.h"

namespace
{
bool HasText(const daisyhost::DisplayModel& display, const std::string& text)
{
    for(const auto& item : display.texts)
    {
        if(item.text.find(text) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

TEST(FieldOledTransientTest, ShowsAndExpiresAfterTwoSeconds)
{
    daisyhost::FieldOledTransient transient;
    transient.Show(
        {"K8 Cutoff", "1.84 kHz", {"returns after 2.0s"}, false, true, 0.5f});

    daisyhost::DisplayModel display;
    transient.ApplyToDisplay(display);
    EXPECT_TRUE(HasText(display, "K8 Cutoff"));
    EXPECT_TRUE(HasText(display, "1.84 kHz"));
    ASSERT_EQ(display.bars.size(), 1u);
    EXPECT_FLOAT_EQ(display.bars[0].normalized, 0.5f);

    transient.Tick(1999.0);
    EXPECT_TRUE(transient.IsVisible());
    transient.Tick(2.0);
    EXPECT_FALSE(transient.IsVisible());
}

TEST(FieldOledTransientTest, RefreshingShowRestartsTimeout)
{
    daisyhost::FieldOledTransient transient;
    transient.Show({"K8 Cutoff", "1.0 kHz", {}, false, false, 0.0f});
    transient.Tick(1500.0);
    transient.Show({"K8 Cutoff", "2.0 kHz", {}, false, false, 0.0f});
    transient.Tick(1000.0);
    EXPECT_TRUE(transient.IsVisible());
    transient.Tick(1001.0);
    EXPECT_FALSE(transient.IsVisible());
}

TEST(FieldOledTransientTest, HeldTransientStaysVisibleUntilReleased)
{
    daisyhost::FieldOledTransient transient;
    transient.Show({"B7 Play", "Running", {}, true, false, 0.0f});
    transient.Tick(5000.0);
    EXPECT_TRUE(transient.IsVisible());
    transient.SetHeld(false);
    transient.Tick(1999.0);
    EXPECT_TRUE(transient.IsVisible());
    transient.Tick(2.0);
    EXPECT_FALSE(transient.IsVisible());
}

TEST(FieldOledTransientTest, RepeatedReleaseDoesNotRefreshTimeout)
{
    daisyhost::FieldOledTransient transient;
    transient.Show({"B7 Play", "Running", {}, true, false, 0.0f});

    transient.SetHeld(false);
    transient.Tick(1000.0);
    transient.SetHeld(false);
    transient.Tick(1001.0);

    EXPECT_FALSE(transient.IsVisible());
}

TEST(FieldOledTransientTest, ClearRemovesVisibleDisplayState)
{
    daisyhost::FieldOledTransient transient;
    transient.Show({"B5 Quantize", "12-note Just", {"detail"}, false, true, 2.0f});
    transient.Clear();

    daisyhost::DisplayModel display;
    display.texts.push_back({0, 0, "Existing", false});
    display.bars.push_back({0, 8, 10, 4, 0.25f});
    transient.ApplyToDisplay(display);

    EXPECT_FALSE(transient.IsVisible());
    ASSERT_EQ(display.texts.size(), 1u);
    EXPECT_EQ(display.texts[0].text, "Existing");
    ASSERT_EQ(display.bars.size(), 1u);
    EXPECT_FLOAT_EQ(display.bars[0].normalized, 0.25f);
}
} // namespace
