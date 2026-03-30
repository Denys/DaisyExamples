#pragma once
#include "daisy_pod.h"
#include "ExtEncoder.h"
#include "OledUI.h"

// ============================================================
// PodControls — reusable Daisy Pod + ExtEncoder control hub
//
// Drop into any Pod project:
//   1. #include "PodControls.h"
//   2. Optionally call SetParamLabel() to name your params
//   3. Call Init(pod, ui) after pod.Init(), before StartAudio()
//   4. Call Poll() every main loop iteration
//   5. Read GetParam(0/1/2) from audio callback
//
// Control assignment:
//   Pot1 (KNOB_1)     → param[0]  live pot position (0–1)
//   Pot2 (KNOB_2)     → param[1]  live pot position (0–1)
//   Pod Encoder       → param[2]  accumulated steps  (0–1)
//   OLED Encoder      → menu scroll / navigation only
//   PSH (enc push)    → Enter submenu / enter edit mode
//   CON button        → Save & Exit (exit edit, go up)
//   BAK button        → Back / cancel
//   PSH held >500 ms  → Jump to root menu
//
// LED feedback (Pod built-in LEDs):
//   LED1 green flash  = PSH pressed
//   LED1 white flash  = BAK pressed
//   LED2 cyan flash   = CON pressed
//
// Zoom overlay (on OLED):
//   Pot1/Pot2 move    → zoom shows param[0/1] for 1.2 s
//   Pod Encoder turn  → zoom shows param[2] for 1.2 s
// ============================================================

class PodControls {
public:
    static constexpr int   PARAM_COUNT   = 3;
    static constexpr float ENC_STEP      = 0.02f;  // param step per encoder click
    static constexpr float KNOB_DEADBAND = 0.01f;  // min change to trigger zoom

    // ---- Init -----------------------------------------------
    // Call after pod.Init() and ui.Init(), before pod.StartAudio().
    void Init(daisy::DaisyPod& pod, OledUI& ui) {
        pod_ = &pod;
        ui_  = &ui;
        ext_.Init();

        params_[0]     = 0.5f;
        params_[1]     = 0.5f;
        params_[2]     = 0.5f;
        prev_knob_[0]  = 0.5f;
        prev_knob_[1]  = 0.5f;

        last_debounce_  = 0;
        led_flash_end_  = 0;
        led2_flash_end_ = 0;
        flash_r_        = 0.f;
        flash_g_        = 0.f;
        flash_b_        = 0.f;
    }

    // Rename a parameter (shown in zoom overlay).
    // Call before Init() or at any time.
    void SetParamLabel(int idx, const char* label) {
        if(idx >= 0 && idx < PARAM_COUNT)
            param_labels_[idx] = label;
    }

    // ---- Poll -----------------------------------------------
    // Call every main loop iteration.
    // Handles 1 kHz debounce, menu navigation, LED drive, zoom triggers.
    void Poll() {
        uint32_t now = daisy::System::GetNow();

        // ---- 1 kHz debounce tick ----------------------------
        if(now - last_debounce_ >= 1) {
            last_debounce_ = now;

            pod_->ProcessAllControls();
            ext_.Debounce();
            ext_.SetFlipped(ui_->GetEncFlipped());

            // -- OLED encoder → menu navigation only --
            int8_t ext_inc = ext_.Increment();
            if(ext_inc != 0) ui_->Scroll(ext_inc);

            // Cache button states (each is valid only within this tick)
            bool psh = ext_.EncoderPressed();
            bool con = ext_.ConfirmPressed();
            bool bak = ext_.BackPressed();
            bool hld = ext_.EncoderHeld();

            if(psh) ui_->Confirm();
            if(con) ui_->SaveAndBack();
            if(bak) ui_->Back();
            if(hld) ui_->BackToRoot();

            // -- Pod encoder → param[2] (audio param, not menu) --
            int8_t pod_inc = pod_->encoder.Increment();
            if(pod_inc != 0) {
                params_[2] += pod_inc * ENC_STEP;
                if(params_[2] < 0.f) params_[2] = 0.f;
                if(params_[2] > 1.f) params_[2] = 1.f;
                ui_->SetZoom(2, params_[2], param_labels_[2]);
            }
            // Pod encoder push: no menu action (audio-domain only)

            // -- LED events --
            if(psh) {
                flash_r_ = 0.f; flash_g_ = 1.f; flash_b_ = 0.f; // green
                led_flash_end_ = now + 150;
            }
            if(bak) {
                flash_r_ = 1.f; flash_g_ = 1.f; flash_b_ = 1.f; // white
                led_flash_end_ = now + 150;
            }
            if(con) {
                led2_flash_end_ = now + 200;
            }
        }

        // ---- Pots → param[0/1] with change-detection zoom ----
        float k0 = pod_->GetKnobValue(daisy::DaisyPod::KNOB_1);
        float k1 = pod_->GetKnobValue(daisy::DaisyPod::KNOB_2);
        params_[0] = k0;
        params_[1] = k1;

        float d0 = k0 - prev_knob_[0]; if(d0 < 0) d0 = -d0;
        float d1 = k1 - prev_knob_[1]; if(d1 < 0) d1 = -d1;

        if(d0 > KNOB_DEADBAND) {
            ui_->SetZoom(0, k0, param_labels_[0]);
            prev_knob_[0] = k0;
        }
        if(d1 > KNOB_DEADBAND) {
            ui_->SetZoom(1, k1, param_labels_[1]);
            prev_knob_[1] = k1;
        }

        // ---- LED drive --------------------------------------
        // LED1: PSH (green) and BAK (white) flashes
        if(led_flash_end_ != 0 && now < led_flash_end_) {
            pod_->led1.Set(flash_r_, flash_g_, flash_b_);
        } else {
            pod_->led1.Set(0.f, 0.f, 0.f);
            if(now >= led_flash_end_) led_flash_end_ = 0;
        }

        // LED2: CON cyan flash
        if(led2_flash_end_ != 0 && now < led2_flash_end_) {
            pod_->led2.Set(0.f, 1.f, 0.8f);
        } else {
            pod_->led2.Set(0.f, 0.f, 0.f);
            if(now >= led2_flash_end_) led2_flash_end_ = 0;
        }

        // BAK is on D22 — pod.UpdateLeds() is safe (no D17 conflict)
        pod_->UpdateLeds();
    }

    // ---- Audio param access ---------------------------------
    // Returns param[idx] (0-1). Safe to call from audio callback.
    // Single-word volatile read — atomic on Cortex-M7.
    float GetParam(int idx) const {
        if(idx < 0 || idx >= PARAM_COUNT) return 0.f;
        return params_[idx];
    }

private:
    daisy::DaisyPod* pod_ = nullptr;
    OledUI*          ui_  = nullptr;
    ExtEncoder       ext_;

    // Audio params — volatile for safe cross-context reads
    volatile float   params_[PARAM_COUNT];
    float            prev_knob_[2];

    uint32_t         last_debounce_;
    uint32_t         led_flash_end_;
    uint32_t         led2_flash_end_;
    float            flash_r_, flash_g_, flash_b_;

    const char* param_labels_[PARAM_COUNT] = { "Pot 1", "Pot 2", "Pod Enc" };
};
