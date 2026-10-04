// Exercise the actual application router/callback with an explicit fake BSP.
// This is not a UART parser, HAL, ARM, or hardware test.
#define main hydrapulse_firmware_entry_not_run
#include "firmware/HydraPulse.cpp"
#undef main
#include "tests/native/test_support.h"
#include <cmath>

int main()
{
    engine.Init();
    controller.Init(engine, hydrapulse::field::ReadControls(hw));
    meter.Init();
    float l[48]{}, r[48]{};
    float *output[2] = {l, r};
    daisy::MidiEvent event{};
    event.type = daisy::NoteOn;
    event.channel = 0;
    event.data[0] = 36;
    event.data[1] = 100;
    Midi(event);
    AudioCallback(nullptr, output, 48);
    for (float x : l)
        HPF_CHECK_EQ(x, 0.0f);
    event.channel = 9;
    event.data[1] = 0;
    Midi(event);
    AudioCallback(nullptr, output, 48);
    for (float x : l)
        HPF_CHECK_EQ(x, 0.0f);
    event.data[1] = 100;
    Midi(event);
    AudioCallback(nullptr, output, 48);
    float peak = 0;
    for (float x : l)
        peak = std::max(peak, std::fabs(x));
    HPF_CHECK(peak > 1e-6f);
    for (int i = 0; i < 80; ++i)
        Midi(event);
    HPF_CHECK(midi_drops.load() > 0);
    event.type = daisy::SystemRealTime;
    event.srt_type = daisy::Stop;
    Midi(event);
    AudioCallback(nullptr, output, 48);
    HPF_CHECK(!engine.Running());
    hydrapulse::MidiCommand pending{};
    HPF_CHECK(!midi_commands.Pop(pending));
    for (int i = 0; i < 100; ++i)
        AudioCallback(nullptr, output, 48);
    for (float x : l)
        HPF_CHECK(std::fabs(x) < 1e-6f);
    event.srt_type = daisy::TimingClock;
    Midi(event);
    AudioCallback(nullptr, output, 48);
    HPF_CHECK(!engine.Running());
    event.srt_type = daisy::Start;
    Midi(event);
    AudioCallback(nullptr, output, 48);
    HPF_CHECK(engine.Running());
    event.type = daisy::ChannelMode;
    event.channel = 9;
    event.data[0] = 123;
    Midi(event);
    AudioCallback(nullptr, output, 48);
    HPF_CHECK(!engine.Running());
    return EXIT_SUCCESS;
}
