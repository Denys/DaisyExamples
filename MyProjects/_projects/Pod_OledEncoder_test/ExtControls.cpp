// ============================================================
// ExtControls.cpp — Daisy Pod + External OLED-Rotary-Encoder
//
// Control layout:
//   Pot1          → param[0]  (any audio param, labeled "Volume")
//   Pot2          → param[1]  (any audio param, labeled "Tone")
//   Pod Encoder   → param[2]  (any audio param, labeled "Mix")
//   OLED Encoder  → menu scroll / navigation
//   PSH           → Enter submenu / enter edit mode
//   CON           → Save & Exit
//   BAK           → Back / cancel
//   PSH hold      → Jump to root menu
//
// Moving any pot or the Pod encoder triggers a zoom popup on the
// OLED showing the parameter value for 1.2 s.
//
// To adapt to another project:
//   1. Copy PodControls.h, ExtEncoder.h, OledUI.h alongside your .cpp
//   2. Replace the AudioCallback DSP chain below
//   3. Call ctl.SetParamLabel() to rename params for your use case
//   4. Keep Init / Poll / Draw structure — no other changes needed
//
// Build:  make
// Flash:  make program  (ST-Link)
// ============================================================

#include "daisy_pod.h"
#include "daisysp.h"
#include "OledUI.h"
#include "PodControls.h"

using namespace daisy;
using namespace daisysp;

// ---- Global objects ----------------------------------------
static DaisyPod    pod;
static OledUI      ui;
static PodControls ctl;  // must be declared before AudioCallback

// ---- Audio callback ----------------------------------------
// RULES: no I²C, no GPIO reads, no dynamic allocation, no printf.
// Read ctl.GetParam() — safe (single-word volatile read, Cortex-M7).
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    // Example DSP: passthrough with volume (param[2]) and tone (param[1]).
    // Replace this with your DSP chain.
    float vol  = ctl.GetParam(2);   // Pod encoder
    float mix  = ctl.GetParam(0);   // Pot1
    (void)mix;                       // suppress unused-variable warning

    for(size_t i = 0; i < size; i++) {
        out[0][i] = in[0][i] * vol;
        out[1][i] = in[1][i] * vol;
    }
}

// ---- Main --------------------------------------------------
int main()
{
    // Hardware init
    pod.Init();
    pod.SetAudioBlockSize(4);
    pod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    // Name the parameters (shown in zoom overlay)
    ctl.SetParamLabel(0, "Volume");
    ctl.SetParamLabel(1, "Tone");
    ctl.SetParamLabel(2, "Mix");

    // Init UI and controls (order: ui first, then ctl)
    ui.Init();
    ctl.Init(pod, ui);

    // Start audio before ADC
    pod.StartAudio(AudioCallback);
    pod.StartAdc();

    uint32_t last_draw = 0;

    while(true) {
        // Poll all controls, navigate menu, drive LEDs, trigger zoom
        ctl.Poll();

        // Draw OLED at ~30 fps
        uint32_t now = System::GetNow();
        if(now - last_draw >= 33) {
            last_draw = now;
            float p[3] = {
                ctl.GetParam(0),
                ctl.GetParam(1),
                ctl.GetParam(2)
            };
            ui.Draw(p, ui.GetPatchIdx());
        }
    }
}
