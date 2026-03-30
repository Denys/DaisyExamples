#pragma once
#include "daisy_pod.h"
#include "ExtEncoder.h"
#include "DrumSeqUI.h"

// ============================================================
// AudioParams — volatile struct for audio/main cross-context
//
// Written by main loop (DrumControls::Poll), read by audio ISR.
// Single-word float reads are atomic on Cortex-M7.
// ============================================================

struct AudioParams {
    volatile float   tempo_hz;        // Metro frequency (Hz)
    volatile float   kick_decay;      // Kick vol env decay (seconds)
    volatile float   kick_pitch_max;  // Kick pitch env max (Hz)
    volatile float   snare_decay;     // Snare env decay (seconds)
    volatile float   volume;          // Master volume 0-1
    volatile float   mix;             // 0=all kick, 1=all snare, 0.5=equal
    volatile uint8_t kick_length;     // 1-32
    volatile uint8_t snare_length;    // 1-32
};

// ============================================================
// DrumControls — Pod + ExtEncoder control hub for drum sequencer
//
// Control assignment:
//   Pot1             → density for active drum (0-100%)
//   Pot2             → length for active drum  (1-32 steps)
//   Pod Encoder turn → BPM (±1 per click, 30-300)
//   Pod Encoder push → reset step counters to 0
//   Button 1         → toggle active drum (kick ↔ snare)
//   Button 2         → tap tempo (rolling avg of last 4 taps)
//   OLED Encoder     → menu scroll / value edit
//   PSH              → Enter submenu / edit mode
//   CON              → Save & Exit
//   BAK              → Back / cancel
//   PSH held >500ms  → Jump to sequencer home
//
// LEDs:
//   LED1 → white 30ms flash on each metro tick (beat sync)
//   LED2 → steady green (kick active) or blue (snare active)
//
// Zoom overlay:
//   Pot1/Pot2 move → shows density/length for 1.2s
//   Pod Encoder    → shows BPM for 1.2s
//   Tap tempo      → shows computed BPM for 1.2s
// ============================================================

class DrumControls {
public:
    static constexpr float KNOB_DEADBAND = 0.02f;
    static constexpr int   TAP_COUNT     = 4;

    // ---- Init -----------------------------------------------
    // Call after pod.Init() and ui.Init(), before StartAudio().
    void Init(daisy::DaisyPod& pod, DrumSeqUI& ui,
              volatile bool* kickSeq, volatile bool* snareSeq,
              volatile uint8_t& kickStep, volatile uint8_t& snareStep,
              volatile bool& tick_flag, AudioParams& ap)
    {
        pod_       = &pod;
        ui_        = &ui;
        kickSeq_   = kickSeq;
        snareSeq_  = snareSeq;
        kickStep_  = &kickStep;
        snareStep_ = &snareStep;
        tick_flag_ = &tick_flag;
        ap_        = &ap;
        ext_.Init();

        active_drum_    = 0;
        prev_knob_[0]   = 0.5f;
        prev_knob_[1]   = 0.5f;
        last_debounce_  = 0;
        led1_flash_end_ = 0;

        tap_count_ = 0;
        last_tap_  = 0;
        for(int i = 0; i < TAP_COUNT; i++) tap_intervals_[i] = 0;

        prev_kick_density_  = -1;
        prev_kick_length_   = -1;
        prev_snare_density_ = -1;
        prev_snare_length_  = -1;
        prev_kick_rot_      = -1;
        prev_snare_rot_     = -1;

        // Initial pattern build + audio params sync
        RebuildPatterns();
        SyncAudioParams();
    }

    // ---- Poll -----------------------------------------------
    // Call every main loop iteration.
    void Poll() {
        uint32_t now = daisy::System::GetNow();

        // ---- 1 kHz debounce tick ----------------------------
        if(now - last_debounce_ >= 1) {
            last_debounce_ = now;

            pod_->ProcessAllControls();
            ext_.Debounce();
            ext_.SetFlipped(ui_->GetEncFlipped());

            // -- OLED encoder → menu navigation --
            int8_t ext_inc = ext_.Increment();
            if(ext_inc != 0) ui_->Scroll(ext_inc);

            bool psh = ext_.EncoderPressed();
            bool con = ext_.ConfirmPressed();
            bool bak = ext_.BackPressed();
            bool hld = ext_.EncoderHeld();

            if(psh) ui_->Confirm();
            if(con) ui_->SaveAndBack();
            if(bak) ui_->Back();
            if(hld) ui_->BackToRoot();

            // -- Pod Encoder turn → BPM --
            int8_t pod_inc = pod_->encoder.Increment();
            if(pod_inc != 0) {
                int bpm = ui_->GetVal(DrumSeqUI::V_TEMPO) + pod_inc;
                if(bpm < 30)  bpm = 30;
                if(bpm > 300) bpm = 300;
                ui_->SetVal(DrumSeqUI::V_TEMPO, bpm);
                float norm = (bpm - 30) / 270.f;
                ui_->SetZoom(0, norm, "BPM");
            }

            // -- Pod Encoder push → reset steps to beat 1 --
            if(pod_->encoder.RisingEdge()) {
                *kickStep_  = 0;
                *snareStep_ = 0;
            }

            // -- Button 1 → toggle active drum --
            if(pod_->button1.RisingEdge()) {
                active_drum_ ^= 1;
                ui_->SetActiveDrum(active_drum_);
            }

            // -- Button 2 → tap tempo --
            if(pod_->button2.RisingEdge()) {
                if(last_tap_ > 0 && (now - last_tap_) < 2000) {
                    tap_intervals_[tap_count_ % TAP_COUNT] = now - last_tap_;
                    tap_count_++;
                    int n = (tap_count_ < TAP_COUNT) ? tap_count_ : TAP_COUNT;
                    uint32_t sum = 0;
                    for(int i = 0; i < n; i++) sum += tap_intervals_[i];
                    float avg_ms = (float)sum / (float)n;
                    int bpm = (int)(60000.f / avg_ms + 0.5f);
                    if(bpm < 30)  bpm = 30;
                    if(bpm > 300) bpm = 300;
                    ui_->SetVal(DrumSeqUI::V_TEMPO, bpm);
                    float norm = (bpm - 30) / 270.f;
                    ui_->SetZoom(1, norm, "Tap BPM");
                } else {
                    tap_count_ = 0;
                }
                last_tap_ = now;
            }
        }

        // ---- Pots → density/length with change-detection zoom --
        float k0 = pod_->GetKnobValue(daisy::DaisyPod::KNOB_1);
        float k1 = pod_->GetKnobValue(daisy::DaisyPod::KNOB_2);

        float d0 = k0 - prev_knob_[0]; if(d0 < 0) d0 = -d0;
        float d1 = k1 - prev_knob_[1]; if(d1 < 0) d1 = -d1;

        if(d0 > KNOB_DEADBAND) {
            int density = (int)(k0 * 100.f + 0.5f);
            if(active_drum_ == 0)
                ui_->SetVal(DrumSeqUI::V_KICK_DENSITY, density);
            else
                ui_->SetVal(DrumSeqUI::V_SNARE_DENSITY, density);
            ui_->SetZoom(2, k0, active_drum_ == 0 ? "K Dens" : "S Dens");
            prev_knob_[0] = k0;
        }
        if(d1 > KNOB_DEADBAND) {
            int length = 1 + (int)(k1 * 31.f + 0.5f);
            if(active_drum_ == 0)
                ui_->SetVal(DrumSeqUI::V_KICK_LENGTH, length);
            else
                ui_->SetVal(DrumSeqUI::V_SNARE_LENGTH, length);
            ui_->SetZoom(3, k1, active_drum_ == 0 ? "K Len" : "S Len");
            prev_knob_[1] = k1;
        }

        // ---- Rebuild patterns if params changed -----------------
        int kd = ui_->GetVal(DrumSeqUI::V_KICK_DENSITY);
        int kl = ui_->GetVal(DrumSeqUI::V_KICK_LENGTH);
        int sd = ui_->GetVal(DrumSeqUI::V_SNARE_DENSITY);
        int sl = ui_->GetVal(DrumSeqUI::V_SNARE_LENGTH);
        int kr = ui_->GetVal(DrumSeqUI::V_KICK_ROT);
        int sr = ui_->GetVal(DrumSeqUI::V_SNARE_ROT);

        if(kd != prev_kick_density_  || kl != prev_kick_length_  ||
           sd != prev_snare_density_ || sl != prev_snare_length_ ||
           kr != prev_kick_rot_      || sr != prev_snare_rot_) {
            RebuildPatterns();
        }

        // ---- Sync menu → audio params ---------------------------
        SyncAudioParams();

        // ---- LED drive ------------------------------------------

        // LED1: beat flash — white 30ms on each metro tick
        if(*tick_flag_) {
            *tick_flag_     = false;
            led1_flash_end_ = daisy::System::GetNow() + 30;
        }
        uint32_t now2 = daisy::System::GetNow();
        if(led1_flash_end_ != 0 && now2 < led1_flash_end_) {
            pod_->led1.Set(1.f, 1.f, 1.f);
        } else {
            pod_->led1.Set(0.f, 0.f, 0.f);
            if(now2 >= led1_flash_end_) led1_flash_end_ = 0;
        }

        // LED2: steady drum indicator — green=kick, blue=snare
        if(active_drum_ == 0)
            pod_->led2.Set(0.f, 0.8f, 0.f);
        else
            pod_->led2.Set(0.f, 0.f, 0.8f);

        pod_->UpdateLeds();
    }

    int GetActiveDrum() const { return active_drum_; }

private:
    daisy::DaisyPod*  pod_       = nullptr;
    DrumSeqUI*        ui_        = nullptr;
    ExtEncoder        ext_;
    volatile bool*    kickSeq_   = nullptr;
    volatile bool*    snareSeq_  = nullptr;
    volatile uint8_t* kickStep_  = nullptr;
    volatile uint8_t* snareStep_ = nullptr;
    volatile bool*    tick_flag_ = nullptr;
    AudioParams*      ap_        = nullptr;

    int       active_drum_;
    float     prev_knob_[2];
    uint32_t  last_debounce_;
    uint32_t  led1_flash_end_;

    int       tap_count_;
    uint32_t  last_tap_;
    uint32_t  tap_intervals_[TAP_COUNT];

    int       prev_kick_density_;
    int       prev_kick_length_;
    int       prev_snare_density_;
    int       prev_snare_length_;
    int       prev_kick_rot_;
    int       prev_snare_rot_;

    // ---- Euclidean pattern generator ----------------------------
    // Distributes 'ones' hits across 'arrayLen' steps as evenly as
    // possible. Fixed-size local arrays (original uses VLAs — illegal
    // in strict C++14 and unsafe on stack).

    static void SetArray(volatile bool* seq, int arrayLen, float density) {
        for(int i = 0; i < 32; i++) seq[i] = false;
        if(arrayLen < 1)  arrayLen = 1;
        if(arrayLen > 32) arrayLen = 32;

        int ones  = (int)(arrayLen * density + 0.5f);
        int zeros = arrayLen - ones;

        if(ones <= 0) return;
        if(zeros <= 0) {
            for(int i = 0; i < arrayLen; i++) seq[i] = true;
            return;
        }

        int oneArr[32]  = {};
        int zeroArr[32] = {};

        // Distribute zeros across ones
        int idx = 0;
        for(int i = 0; i < zeros; i++) {
            zeroArr[idx] += 1;
            idx = (idx + 1) % ones;
        }

        // Count remaining ones (those with no trailing zeros)
        int rem = 0;
        for(int i = 0; i < ones; i++) {
            if(zeroArr[i] == 0) rem++;
        }

        // Distribute remaining ones across non-remainder slots
        idx = 0;
        if(ones - rem > 0) {
            for(int i = 0; i < rem; i++) {
                oneArr[idx] += 1;
                idx = (idx + 1) % (ones - rem);
            }
        }

        // Fill the output sequence
        idx = 0;
        for(int i = 0; i < (ones - rem) && idx < arrayLen; i++) {
            seq[idx++] = true;
            for(int j = 0; j < zeroArr[i] && idx < arrayLen; j++)
                seq[idx++] = false;
            for(int j = 0; j < oneArr[i] && idx < arrayLen; j++)
                seq[idx++] = true;
        }
    }

    static void ApplyRotation(volatile bool* seq, int len, int rot) {
        if(len <= 1 || rot == 0) return;
        rot = ((rot % len) + len) % len;
        bool tmp[32];
        for(int i = 0; i < len; i++) tmp[i] = seq[i];
        for(int i = 0; i < len; i++) seq[i] = tmp[(i + rot) % len];
    }

    void RebuildPatterns() {
        int kd = ui_->GetVal(DrumSeqUI::V_KICK_DENSITY);
        int kl = ui_->GetVal(DrumSeqUI::V_KICK_LENGTH);
        int sd = ui_->GetVal(DrumSeqUI::V_SNARE_DENSITY);
        int sl = ui_->GetVal(DrumSeqUI::V_SNARE_LENGTH);
        int kr = ui_->GetVal(DrumSeqUI::V_KICK_ROT);
        int sr = ui_->GetVal(DrumSeqUI::V_SNARE_ROT);

        SetArray(kickSeq_,  kl, kd / 100.f);
        SetArray(snareSeq_, sl, sd / 100.f);

        if(kr > 0) ApplyRotation(kickSeq_, kl, kr);
        if(sr > 0) ApplyRotation(snareSeq_, sl, sr);

        prev_kick_density_  = kd;
        prev_kick_length_   = kl;
        prev_snare_density_ = sd;
        prev_snare_length_  = sl;
        prev_kick_rot_      = kr;
        prev_snare_rot_     = sr;
    }

    void SyncAudioParams() {
        int bpm = ui_->GetVal(DrumSeqUI::V_TEMPO);
        ap_->tempo_hz      = bpm * 4.f / 60.f;
        ap_->kick_decay    = ui_->GetVal(DrumSeqUI::V_KICK_DECAY) * 0.01f;
        ap_->kick_pitch_max = (float)ui_->GetVal(DrumSeqUI::V_KICK_PITCH_HI);
        ap_->snare_decay   = ui_->GetVal(DrumSeqUI::V_SNARE_DECAY) * 0.01f;
        ap_->volume        = ui_->GetVal(DrumSeqUI::V_VOLUME) / 100.f;
        ap_->mix           = ui_->GetVal(DrumSeqUI::V_MIX) / 100.f;
        ap_->kick_length   = (uint8_t)ui_->GetVal(DrumSeqUI::V_KICK_LENGTH);
        ap_->snare_length  = (uint8_t)ui_->GetVal(DrumSeqUI::V_SNARE_LENGTH);
    }
};
