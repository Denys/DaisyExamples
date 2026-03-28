// ============================================================
// ExtControls.cpp — Daisy Pod + External OLED-Rotary-Encoder
//
// Demonstrates:
//  • SSD1306 OLED via I²C with hierarchical menu UI
//  • External EC11 encoder + CON + BAK buttons
//  • Co-existence with Pod onboard controls
//  • Audio passthrough skeleton (no blocking in callback)
//
// Build:  make
// Flash:  make program          (ST-Link, default)
//         make program-dfu      (DFU bootloader fallback)
// ============================================================

#include "daisy_pod.h"
#include "daisysp.h"
#include "ExtEncoder.h"
#include "OledUI.h"

using namespace daisy;
using namespace daisysp;

// ---- Global objects ----------------------------------------
static DaisyPod  pod;
static ExtEncoder ext;
static OledUI    ui;

// ---- Audio state (written in main loop, read in callback) --
// No mutex needed: single-word reads/writes on Cortex-M7 are atomic
static volatile float param[4] = { 0.5f, 0.5f, 0.5f, 0.5f };

// ---- Audio callback ----------------------------------------
// RULES: no I²C, no GPIO reads, no dynamic allocation, no printf
// Only read pre-latched param[] values.
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // Passthrough skeleton — replace with DSP chain as needed
    for(size_t i = 0; i < size; i++) {
        out[0][i] = in[0][i];
        out[1][i] = in[1][i];
    }
}

// ---- Main --------------------------------------------------
int main()
{
    // ---- Hardware init -------------------------------------
    pod.Init();
    pod.SetAudioBlockSize(4);
    pod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    // External module and UI
    ext.Init();
    ui.Init();

    // Start audio first, then ADC
    pod.StartAudio(AudioCallback);
    pod.StartAdc();

    // ---- Timing state --------------------------------------
    uint32_t last_debounce = 0;
    uint32_t last_draw     = 0;
    uint32_t led_flash_end = 0;   // time when LED flash should extinguish
    float    flash_r = 0.f, flash_g = 0.f, flash_b = 0.f;

    // ---- Main loop -----------------------------------------
    while(true) {
        uint32_t now = System::GetNow();

        // ---- 1 kHz debounce tick ---------------------------
        // ALL input reads must happen here, once per tick.
        // Encoder::Increment() and Switch::RisingEdge() hold their
        // value until the next Debounce() call — reading them in the
        // tight main loop would fire them hundreds of times per ms.
        if(now - last_debounce >= 1) {
            last_debounce = now;

            // Pod onboard: encoder, buttons, knobs
            pod.ProcessAllControls();

            // External module: encoder, CON, BAK
            ext.Debounce();

            // Sync encoder-flip setting from menu
            ext.SetFlipped(ui.GetEncFlipped());

            // ---- External encoder & buttons ----------------
            int8_t ext_inc = ext.Increment();
            if(ext_inc != 0)
                ui.Scroll(ext_inc);

            if(ext.EncoderPressed() || ext.ConfirmPressed())
                ui.Confirm();

            if(ext.BackPressed())
                ui.Back();

            if(ext.EncoderHeld())
                ui.BackToRoot();

            // ---- Pod encoder (dual control) ----------------
            int8_t pod_inc = pod.encoder.Increment();
            if(pod_inc != 0)
                ui.Scroll(pod_inc);

            if(pod.encoder.RisingEdge())
                ui.Confirm();
        }

        // ---- Read pots → update latched params -------------
        // params[0] and [1] are driven by POT1 / POT2
        // params[2] and [3] are placeholders (extend as needed)
        param[0] = pod.GetKnobValue(DaisyPod::KNOB_1);
        param[1] = pod.GetKnobValue(DaisyPod::KNOB_2);

        // ---- Display update at ~30 fps ---------------------
        if(now - last_draw >= 33) {
            last_draw = now;

            // Cast away volatile for the draw call (read-only snapshot)
            float p[4] = { param[0], param[1], param[2], param[3] };
            ui.Draw(p, ui.GetPatchIdx());
        }

        // ---- LED feedback (set flash color in debounce block) -----
        // CON / PSH edge  → green flash  200 ms
        // BAK deb. edge   → blue flash   200 ms
        // (raw BAK read is applied outside the debounce block below)
        if(ext.ConfirmPressed() || ext.EncoderPressed()) {
            flash_r = 0.f; flash_g = 1.f; flash_b = 0.f;  // green
            led_flash_end = now + 200;
        }
        if(ext.BackPressed()) {
            flash_r = 0.f; flash_g = 0.f; flash_b = 1.f;  // blue
            led_flash_end = now + 200;
        }

        // ---- LED drive (outside debounce block) -------------------
        // NOTE: Never call pod.UpdateLeds() — that drives D17 (BAK pin) as OUTPUT.
        //
        // Priority:
        //   1. WHITE steady  = BAK pin is LOW right now (raw GPIO confirmed)
        //   2. Colour flash  = debounced edge fired
        //   3. OFF
        if(ext.BakRawPressed()) {
            pod.led1.Set(1.f, 1.f, 1.f);   // white: BAK physically held
        } else if(led_flash_end != 0 && now < led_flash_end) {
            pod.led1.Set(flash_r, flash_g, flash_b);
        } else {
            pod.led1.Set(0.f, 0.f, 0.f);
            if(now >= led_flash_end) led_flash_end = 0;
        }

        pod.led1.Update();  // Drive LED1 only — do NOT call pod.UpdateLeds()
    }
}
