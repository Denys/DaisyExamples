#include <gtest/gtest.h>

#include "daisyhost/MidiEventTracker.h"

namespace
{
TEST(MidiEventTrackerTest, FormatsIncomingMessagesAndLimitsHistory)
{
    daisyhost::MidiEventTracker tracker;
    tracker.SetInputDeviceCounts(2, 2);

    daisyhost::MidiMessageEvent noteOn;
    noteOn.status = 0x90;
    noteOn.data1  = 60;
    noteOn.data2  = 100;
    tracker.Record(noteOn);

    daisyhost::MidiMessageEvent cc;
    cc.status = 0xB0;
    cc.data1  = 74;
    cc.data2  = 96;
    tracker.Record(cc);

    const auto snapshot = tracker.GetSnapshot();
    ASSERT_EQ(snapshot.availableInputs, 2);
    ASSERT_EQ(snapshot.enabledInputs, 2);
    ASSERT_EQ(snapshot.recentMessages.size(), 2u);
    EXPECT_NE(snapshot.recentMessages[0].find("Note On"), std::string::npos);
    EXPECT_NE(snapshot.recentMessages[1].find("CC 74"), std::string::npos);
}
} // namespace
