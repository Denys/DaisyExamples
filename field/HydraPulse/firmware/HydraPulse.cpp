#include "daisy_field.h"
#include "firmware/FieldAdapter.h"
#include "src/app/Controller.h"
#include "src/app/Presets.generated.h"
#include "src/core/SpscQueue.h"
#include <atomic>
#include <cstdio>

namespace
{
daisy::DaisyField hw;
hydrapulse::Engine engine;
hydrapulse::Controller controller;
hydrapulse::field::CallbackMeter meter;
hydrapulse::field::BootRecord boot;
hydrapulse::core::SpscQueue<hydrapulse::UiSnapshot, 8> snapshots;
hydrapulse::core::SpscQueue<hydrapulse::MidiCommand, 32> midi_commands;
std::atomic<std::uint32_t> urgent{0}, midi_drops{0};
std::uint32_t snapshot_drops = 0, blocks = 0;

void AudioCallback(daisy::AudioHandle::InputBuffer, daisy::AudioHandle::OutputBuffer output, std::size_t size)
{
    meter.Begin(hw);
    const auto frame = hydrapulse::field::ReadControls(hw);
    controller.Tick(engine, frame);
    const auto urgent_now = urgent.exchange(0, std::memory_order_acq_rel);
    hydrapulse::MidiCommand command{};
    if (urgent_now)
    {
        engine.Panic();
        // Flush pre-stop commands so an old Note On cannot restart a silenced voice.
        for (unsigned i = 0; i < 32 && midi_commands.Pop(command); ++i)
        {
        }
    }
    else
    {
        for (unsigned i = 0; i < 8 && midi_commands.Pop(command); ++i)
            if (!controller.SuppressMidi())
                engine.HandleMidi(command);
    }
    hydrapulse::field::OutputsOff(hw); // No live CV/Gate integration before P0 qualification.
    for (std::size_t i = 0; i < size; ++i)
    {
        const auto audio = engine.Process();
        output[0][i] = audio.left;
        output[1][i] = audio.right;
    }
    if (++blocks % 40u == 0)
    {
        auto s = controller.Snapshot(engine);
        s.callback_count = meter.Count();
        s.load_max_per_mille = meter.MaxPermille();
        s.overruns = meter.Overruns();
        s.late_starts = meter.LateStarts();
        s.snapshot_drops = snapshot_drops;
        s.midi_drops = midi_drops.load(std::memory_order_relaxed);
        if (!snapshots.Push(s))
            ++snapshot_drops;
    }
    meter.End(hw);
}

void Midi(daisy::MidiEvent event)
{
    using hydrapulse::MidiCommand;
    using hydrapulse::MidiCommandType;
    MidiCommand command{};
    bool send = false;
    if (event.type == daisy::SystemRealTime)
    {
        if (event.srt_type == daisy::Start)
        {
            command.type = MidiCommandType::Start;
            send = true;
        }
        else if (event.srt_type == daisy::Stop || event.srt_type == daisy::Reset)
            urgent.fetch_or(1u, std::memory_order_release);
        // TimingClock and Continue intentionally unsupported; internal tempo is authoritative.
    }
    else if (event.channel == 9 && event.type == daisy::NoteOn && event.data[1] > 0)
    {
        constexpr std::uint8_t notes[4] = {36, 38, 42, 39};
        for (std::uint8_t t = 0; t < 4; ++t)
            if (event.data[0] == notes[t])
            {
                command = {MidiCommandType::Trigger, t, event.data[1]};
                send = true;
                break;
            }
    }
    else if (event.channel == 9 && (event.type == daisy::ChannelMode || event.type == daisy::ControlChange))
    {
        if (event.data[0] == 120 || event.data[0] == 123)
            urgent.fetch_or(1u, std::memory_order_release);
    }
    if (send && !midi_commands.Push(command))
        midi_drops.fetch_add(1, std::memory_order_relaxed);
}
void Draw(const hydrapulse::UiSnapshot &s)
{
    const char *voices[4] = {"HAMMER", "CRACK", "STEEL", "ARC"};
    const char banks[3] = {'A', 'B', 'F'};
    char line[32];
    hw.display.Fill(false);
    if (s.shift)
    {
        hydrapulse::field::Text(hw, 0, "SHIFT: RELEASE=HOME");
        hydrapulse::field::Text(hw, 1, "1-4 VOICE  5=A 6=B");
        hydrapulse::field::Text(hw, 2, "7=EDIT F 8=HOLD FILL");
        hydrapulse::field::Text(hw, 3, "9-12=MUTE VOICES");
        hydrapulse::field::Text(hw, 4, "13=HOLD CLEAR (STOP)");
        hydrapulse::field::Text(hw, 5, "14/15 PREV/NEXT PRE");
        if (s.preset_armed)
        {
            std::snprintf(line, sizeof(line), "LOAD? %s",
                          hydrapulse::kTutorialPresets[s.pending_preset].name);
            hydrapulse::field::Text(hw, 6, line);
            hydrapulse::field::Text(hw, 7, "16=LOAD RELEASE=HOME");
        }
        else
        {
            hydrapulse::field::Text(hw, 6, "16=LOAD (STOP ONLY)");
            hydrapulse::field::Text(hw, 7, "BOTH 0.5s = PANIC");
        }
    }
    else
    {
        std::snprintf(line, sizeof(line), "HPF %s %c>%c %03u", s.running ? "RUN" : "STOP", banks[s.edit_bank],
                      banks[s.play_bank], unsigned(s.bpm));
        hydrapulse::field::Text(hw, 0, line);
        std::snprintf(line, sizeof(line), "%s %02u %s", voices[s.selected], unsigned(s.step + 1u),
                      s.fill ? "FILL" : "");
        hydrapulse::field::Text(hw, 1, line);
        std::snprintf(line, sizeof(line), "TUN%03u DEC%03u", unsigned(s.params[0] * 100),
                      unsigned(s.params[1] * 100));
        hydrapulse::field::Text(hw, 2, line);
        std::snprintf(line, sizeof(line), "CHR%03u LEV%03u", unsigned(s.params[2] * 100),
                      unsigned(s.params[3] * 100));
        hydrapulse::field::Text(hw, 3, line);
        std::snprintf(line, sizeof(line), "SW%02u DR%02u VOL%02u", unsigned(50 + 20 * s.swing),
                      unsigned(100 * s.drive), unsigned(100 * s.master));
        hydrapulse::field::Text(hw, 4, line);
        if (s.loading)
            hydrapulse::field::Text(hw, 5, "LOADING...");
        else if (s.preset_armed)
        {
            std::snprintf(line, sizeof(line), "LOAD? %s",
                          hydrapulse::kTutorialPresets[s.pending_preset].name);
            hydrapulse::field::Text(hw, 5, line);
        }
        else
            hydrapulse::field::Text(hw, 5, hydrapulse::kTutorialPresets[s.preset].name);
        std::snprintf(line, sizeof(line), "PICKUP:%02X MUTE:%X", unsigned(s.pickup_mask), unsigned(s.mutes));
        hydrapulse::field::Text(hw, 6, line);
        std::snprintf(line, sizeof(line), "CPUmax %u.%u%% #%u", unsigned(s.load_max_per_mille / 10u),
                      unsigned(s.load_max_per_mille % 10u), unsigned(s.faults));
        hydrapulse::field::Text(hw, 7, line);
    }
    hw.display.Update();
    for (std::size_t i = 0; i < daisy::DaisyField::LED_LAST; ++i)
        hw.led_driver.SetLed(i, 0.0f);
    for (std::size_t i = 0; i < 16; ++i)
    {
        float brightness = (s.notes & (1u << i)) ? ((s.accents & (1u << i)) ? 0.7f : 0.25f) : 0.0f;
        if (s.running && s.step == i)
            brightness = 1.0f;
        if (s.shift && i < 4)
            brightness = (i == s.selected) ? 1.0f : 0.15f;
        hw.led_driver.SetLed(hydrapulse::field::kLogicalToLed[i], brightness);
    }
    hw.led_driver.SetLed(daisy::DaisyField::LED_SW_1, s.running ? 0.6f : 0);
    hw.led_driver.SetLed(daisy::DaisyField::LED_SW_2, s.shift ? 0.6f : 0);
    hw.led_driver.SwapBuffersAndTransmit();
}
} // namespace
int main()
{
    hydrapulse::field::InitHardware(hw);
    boot.Capture(hw);
    engine.Init(hydrapulse::field::kSampleRate);
    controller.Init(engine, hydrapulse::field::ReadControls(hw));
    meter.Init();
    hw.StartAudio(AudioCallback);
    hydrapulse::UiSnapshot current{};
    std::uint32_t last_log = 0, last_boot = 0;
    while (true)
    {
        hw.midi.Listen();
        for (unsigned i = 0; i < 32 && hw.midi.HasEvents(); ++i)
            Midi(hw.midi.PopEvent());
        bool updated = false;
        for (unsigned i = 0; i < 8 && snapshots.Pop(current); ++i)
            updated = true;
        if (updated)
            Draw(current);
        const auto now = daisy::System::GetNow();
        if (now - last_log >= 250u)
        {
            last_log = now;
            hw.seed.PrintLine("[HPF] beat cb=%lu load_pm=%lu faults=%lu",
                              static_cast<unsigned long>(current.callback_count),
                              static_cast<unsigned long>(current.load_max_per_mille),
                              static_cast<unsigned long>(current.faults));
            hw.seed.PrintLine("[HPF] timing cb=%lu over=%lu late=%lu",
                              static_cast<unsigned long>(current.callback_count),
                              static_cast<unsigned long>(current.overruns),
                              static_cast<unsigned long>(current.late_starts));
            hw.seed.PrintLine("[HPF] queues mdrops=%lu sdrops=%lu",
                              static_cast<unsigned long>(current.midi_drops),
                              static_cast<unsigned long>(current.snapshot_drops));
        }
        if (now - last_boot >= 2000u)
        {
            last_boot = now;
            boot.Print(hw, "beat");
        }
        daisy::System::Delay(1);
    }
}
