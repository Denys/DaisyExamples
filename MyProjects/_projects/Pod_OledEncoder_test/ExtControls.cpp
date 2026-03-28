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
    uint32_t last_debounce  = 0;
    uint32_t last_draw      = 0;
    uint32_t led_flash_end  = 0;   // LED1 flash timeout
    uint32_t led2_flash_end = 0;   // LED2 flash timeout (CON)
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

            // PSH (encoder push) — navigate deeper / enter edit mode
            if(ext.EncoderPressed())
                ui.Confirm();

            // CON — save value and go up (Save & Exit)
            if(ext.ConfirmPressed())
                ui.SaveAndBack();

            if(ext.BackPressed())
                ui.Back();

            if(ext.EncoderHeld())
                ui.BackToRoot();

            // ---- Pod encoder (mirrors external) ----------------
            int8_t pod_inc = pod.encoder.Increment();
            if(pod_inc != 0)
                ui.Scroll(pod_inc);

            // Pod encoder push = Enter (same as PSH)
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

        // ---- LED feedback (set in debounce block) -----------------
        // LED1:  PSH/BAK activity
        //   white steady = BAK physically held (raw GPIO)
        //   blue flash   = BAK debounced rising edge
        //   green flash  = PSH (encoder push) rising edge
        // LED2:  CON activity
        //   cyan flash   = CON (Confirm / Save & Exit) pressed
        if(ext.EncoderPressed()) {
            flash_r = 0.f; flash_g = 1.f; flash_b = 0.f;  // green = PSH
            led_flash_end = now + 150;
        }
        if(ext.BackPressed()) {
            flash_r = 0.f; flash_g = 0.f; flash_b = 1.f;  // blue = BAK deb.
            led_flash_end = now + 150;
        }
        if(ext.ConfirmPressed()) {
            led2_flash_end = now + 200;
        }

        // ---- LED drive (outside debounce block) -------------------
        // BAK is now on D22 (not D17), so pod.UpdateLeds() is safe to call.

        // LED1: BAK raw > debounced flash > off
        if(ext.BakRawPressed()) {
            pod.led1.Set(1.f, 1.f, 1.f);           // white: BAK held
        } else if(led_flash_end != 0 && now < led_flash_end) {
            pod.led1.Set(flash_r, flash_g, flash_b);
        } else {
            pod.led1.Set(0.f, 0.f, 0.f);
            if(now >= led_flash_end) led_flash_end = 0;
        }

        // LED2: CON flash (cyan) > off
        if(led2_flash_end != 0 && now < led2_flash_end) {
            pod.led2.Set(0.f, 1.f, 0.8f);          // cyan: CON pressed
        } else {
            pod.led2.Set(0.f, 0.f, 0.f);
            if(now >= led2_flash_end) led2_flash_end = 0;
        }

        pod.UpdateLeds();   // drives both LED1 and LED2 (BAK is on D22, no conflict)
    }
}
