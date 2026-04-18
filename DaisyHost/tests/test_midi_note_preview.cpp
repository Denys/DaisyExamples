#include <algorithm>
#include <array>

#include <gtest/gtest.h>

#include "daisyhost/MidiNotePreview.h"

namespace
{
TEST(MidiNotePreviewTest, NoteOnProducesAudiblePreviewSignal)
{
    daisyhost::MidiNotePreview preview;
    preview.Prepare(48000.0);

    daisyhost::MidiMessageEvent noteOn;
    noteOn.status = 0x90;
    noteOn.data1  = 69;
    noteOn.data2  = 100;
    preview.HandleMidiEvent(noteOn);

    std::array<float, 64> buffer = {};
    preview.RenderAdd(buffer.data(), buffer.size(), 0.5f);

    const auto peakIt = std::max_element(buffer.begin(), buffer.end());
    EXPECT_GT(*peakIt, 0.01f);
    EXPECT_TRUE(preview.IsActive());
}

TEST(MidiNotePreviewTest, NoteOffStopsSignalGeneration)
{
    daisyhost::MidiNotePreview preview;
    preview.Prepare(48000.0);

    daisyhost::MidiMessageEvent noteOn;
    noteOn.status = 0x90;
    noteOn.data1  = 60;
    noteOn.data2  = 127;
    preview.HandleMidiEvent(noteOn);
    std::array<float, 64> warmup = {};
    preview.RenderAdd(warmup.data(), warmup.size(), 0.5f);

    daisyhost::MidiMessageEvent noteOff;
    noteOff.status = 0x80;
    noteOff.data1  = 60;
    noteOff.data2  = 0;
    preview.HandleMidiEvent(noteOff);

    std::array<float, 32> buffer = {};
    preview.RenderAdd(buffer.data(), buffer.size(), 0.5f);

    const auto releasePeak = std::max_element(buffer.begin(), buffer.end());
    EXPECT_GT(*releasePeak, 0.0f);

    std::array<float, 4096> tail = {};
    preview.RenderAdd(tail.data(), tail.size(), 0.5f);
    EXPECT_FALSE(preview.IsActive());
}
} // namespace
