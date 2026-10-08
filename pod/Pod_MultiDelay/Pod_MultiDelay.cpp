// Daisy Pod local demonstrator. Source ownership and behavior: CONTRACT.md.
#include "Controls.h"
#include "daisy_pod.h"
using namespace daisy;
namespace {
DaisyPod hw;
poddemo::Demo demo;
poddemo::Controls controls;
poddemo::Snapshot published;
float DSY_SDRAM_BSS history[2 * poddemo::kHistory];
float DSY_SDRAM_BSS freezeLoop[2 * poddemo::kFreeze];
// Main cannot preempt audio. Publication masks IRQs only for the fixed-size copy.
void Publish(const poddemo::Snapshot &s) {
    const auto primask = __get_PRIMASK();
    __disable_irq();
    published = s;
    __DMB();
    __set_PRIMASK(primask);
}
void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    const auto snapshot = published;
    demo.Process(in[0], out[0], out[1], size, snapshot);
}
void Leds(bool shift) {
    const float colors[5][3] = {{0, 1, 0}, {1, 0.3f, 0}, {0, 0.3f, 1}, {0.7f, 0, 1}, {0, 1, 1}};
    float b = controls.s.bypass ? 0.025f : 0.2f;
    if (controls.s.driveOn && (System::GetNow() / 250) % 2)
        b *= .4f;
    const auto *c = colors[controls.s.mode];
    hw.led1.Set(c[0] * b, c[1] * b, c[2] * b);
    if (controls.page == poddemo::Controls::kDistPage) {
        const float d = controls.s.driveOn ? .2f : .025f;
        hw.led2.Set(d, d, 0);
    } else if (demo.Paused())
        hw.led2.Set(0.2f, 0, 0);
    else if (shift) {
        int n =
            controls.s.mode == 0 ? controls.s.config : static_cast<int>(demo.freezeStatus.load());
        n = std::clamp(n, 0, 4);
        const auto *f = colors[n];
        hw.led2.Set(f[0] * 0.2f, f[1] * 0.2f, f[2] * 0.2f);
    } else {
        const int p = controls.page;
        hw.led2.Set(p == 1 ? 0.2f : 0, p == 0 ? 0.2f : 0, p == 2 ? 0.2f : 0);
    }
    hw.UpdateLeds();
}
} // namespace
int main() {
    hw.Init();
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    hw.SetAudioBlockSize(poddemo::kBlock);
    controls.Init();
    demo.Init(history, freezeLoop, controls.s);
    Publish(controls.s);
    const uint32_t adcStarted = System::GetNow();
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    uint32_t last = System::GetNow();
    for (;;) {
        const auto now = System::GetNow();
        if (now - last < 1)
            continue;
        last = now;
        hw.ProcessAllControls();
        poddemo::Events e;
        e.now = now;
        e.pots[0] = hw.GetKnobValue(DaisyPod::KNOB_1);
        e.pots[1] = hw.GetKnobValue(DaisyPod::KNOB_2);
        e.turn = hw.encoder.Increment();
        e.encClick = hw.encoder.RisingEdge();
        e.b1Rise = hw.button1.RisingEdge();
        e.b2Rise = hw.button2.RisingEdge();
        e.b2Fall = hw.button2.FallingEdge();
        e.b2 = hw.button2.Pressed();
        e.freezeState = demo.freezeStatus.load(std::memory_order_relaxed);
        // Give ADC smoothing 100 ms to settle before arming knob movement detection.
        if (now - adcStarted >= 100) {
            controls.Step(e);
            Publish(controls.s);
        }
        demo.Service(controls.s);
        Leds(e.b2);
    }
}
