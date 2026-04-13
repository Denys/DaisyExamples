// ============================================================
// EDGE_mono_DSP — Daisy Pod Performance FX Instrument
//
// Target: Daisy Pod + external I2C board (OLED, encoder, BAK, CON)
//
// Phase 1: Hardware bring-up + full UI state machine
// Phase 2: Input gain + DcBlock + input HP + Overdrive
//
// Build:  make
// Flash:  make program  (ST-Link)
//
// Pin map (external board):
//   TRA=D7  TRB=D8  PSH=D9  CON=D10  BAK=D22
//   SCL=D11  SDA=D12  (I2C_1, 400kHz)
//
// Controls:
//   Pod Knob1   → Delay Time (K1) / Subdivision (Shift+K1)
//   Pod Knob2   → Feedback  (K2) / FB LP macro  (Shift+K2)
//   SW1 short   → Freeze momentary
//   SW1 hold    → Shift modifier
//   SW2         → Tap Tempo / Delay Mode toggle (Shift+SW2)
//   Pod Enc     → Wet/Dry / Drive (Shift) / Freeze Latch (push)
//   Ext Enc     → Menu navigate / value adjust (edit mode)
//   Ext PSH     → Edit/Confirm / Quick Save (long hold)
//   BAK         → Back / cancel edit
//   CON         → Confirm
// ============================================================

#include "daisy_pod.h"
#include "daisysp.h"
#include "ExtEncoder.h"
#include "parameters.h"
#include "presets.h"
#include "display.h"
#include "ui_state.h"
#include "tonestack.h"
#include "fdn_reverb.h"
#include <cstring>

using namespace daisy;
using namespace daisysp;

// ---- Hardware objects ---------------------------------------
static DaisyPod   pod;
static ExtEncoder ext_enc;
static Display    display;
static UIState    ui;
static TimerHandle control_timer;

// ---- Timer Callback for 1kHz control polling ----------------
// Only digital debouncing here — analog (ADC) processing stays in the main loop
// where the ADC DMA timing is correct.
void ControlTimerCallback(void* data) {
    pod.ProcessDigitalControls(); // buttons + pod encoder
    ext_enc.Debounce();           // external encoder (must be 1kHz)
}

// ---- Parameter stores (double-buffer, per DAISY_DEVELOPMENT_STANDARDS §7) --
// params_      : authoritative copy — written by main loop only
// param_buf[2] : double buffer — main writes inactive side, swaps atomic index
// active_buf_  : volatile index — ISR reads param_buf[active_buf_] only
static FxParams          params_;
static FxParams          param_buf[2];
static volatile int      active_buf_ = 0;

// ---- Factory presets ----------------------------------------
static FxParams          presets_[kNumPresets];

// ---- Phase 2 active DSP modules ------------------------------
static DcBlock      dcb;
static Svf          svf_hp_in;       // Input high-pass
static Overdrive    overdrive;

// Smoothed DSP parameters written only in the ISR.
static float        sm_input_gain = 1.0f;
static float        sm_drive      = 0.25f;
static float        sm_hp_hz      = 80.0f;

// ---- Phase 3 delay modules -----------------------------------
#define MAX_DELAY static_cast<size_t>(48000 * 2.0f)
DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delay_engine;
static Svf          svf_fb_lp;       // Feedback low-pass
static Svf          svf_fb_hp;       // Feedback high-pass

// Smoothed DSP parameters for Phase 3
static float        sm_delay_samples = 48000.f * 0.5f;
static float        sm_feedback      = 0.5f;
static float        sm_fb_lp_hz      = 6000.f;
static float        sm_fb_hp_hz      = 60.f;
static float        sm_wet           = 0.4f;
static float        sm_wow_depth     = 0.0f;
static float        sm_wow_rate_hz   = 1.2f;
static float        sm_output_tilt_db= 0.0f;
static float        sm_diffuse_damping= 8000.f;
static bool         wow_enabled      = false;

// ---- Later-phase DSP modules (Phase 4 active) ----------------
static Limiter      limiter;
static Phasor       wow_phasor;
static ToneStack    tilt_eq;
static FDNReverb16K reverb;       // from DAFX_2_Daisy_lib

// ---- SW1 (Shift / Freeze) state -----------------------------
static uint32_t sw1_press_time_ = 0;
static bool     sw1_held_       = false;
static bool     shift_consumed_ = false;  // Shift was used while held — suppress freeze

struct DrawState {
    int      page            = 0;
    int      cursor          = 0;
    bool     edit_mode       = false;
    bool     shift_active    = false;
    bool     preset_selected = false;
    FxParams params          = {};
};

static bool DrawStateChanged(const DrawState& a, const DrawState& b) {
    return a.page            != b.page
        || a.cursor          != b.cursor
        || a.edit_mode       != b.edit_mode
        || a.shift_active    != b.shift_active
        || a.preset_selected != b.preset_selected
        || std::memcmp(&a.params, &b.params, sizeof(FxParams)) != 0;
}

// ============================================================
// AudioCallback — ISR context
// RULES: no I2C, no GPIO reads, no dynamic alloc, no printf
// ============================================================

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size) {
    // ---- Handle Analog Controls -----------------------------
    // Placed here so the ADC is sampled and filtered exactly
    // at AudioCallbackRate() (1000 Hz), matching its internal 
    // low-pass filter calculation.
    pod.ProcessAnalogControls();

    // Snapshot: read the currently active buffer (atomic pointer swap)
    const FxParams& p    = param_buf[active_buf_];
    const bool  bypass   = p.bypass;
    const float target_input_gain = fclamp(p.input_gain, 0.5f, 2.0f);
    const float target_drive      = fclamp(p.drive, 0.0f, 1.0f);
    const float target_hp_hz      = fclamp(p.hp_hz, 20.0f, 500.0f);

    // Callback-local smoothing keeps audible control changes stable
    // without adding main-loop work or cross-thread complexity.
    fonepole(sm_input_gain, target_input_gain, 0.001f);
    fonepole(sm_drive,      target_drive,      0.001f);
    fonepole(sm_hp_hz,      target_hp_hz,      0.001f);

    float target_delay_ms = p.delay_time_ms;
    if (p.sync_mode) {
        target_delay_ms = BpmToDelayMs(p.bpm, p.subdiv_idx);
    }
    float target_delay_samples = fclamp(target_delay_ms * (pod.AudioSampleRate() / 1000.f), 1.0f, (float)MAX_DELAY);
    
    fonepole(sm_delay_samples, target_delay_samples, 0.001f);
    fonepole(sm_feedback,      fclamp(p.feedback, 0.0f, 0.98f), 0.001f);
    fonepole(sm_fb_lp_hz,      fclamp(p.fb_lp_hz, 500.f, 18000.f), 0.001f);
    fonepole(sm_fb_hp_hz,      fclamp(p.fb_hp_hz, 20.f, 500.f), 0.001f);
    fonepole(sm_wet,           fclamp(p.wet, 0.0f, 1.0f), 0.001f);
    fonepole(sm_wow_depth,     fclamp(p.wow_depth, 0.0f, 1.0f), 0.001f);
    fonepole(sm_wow_rate_hz,   fclamp(p.wow_rate_hz, 0.1f, 5.0f), 0.001f);
    fonepole(sm_output_tilt_db,fclamp(p.output_tilt_db, -6.0f, 6.0f), 0.001f);
    fonepole(sm_diffuse_damping, fclamp(p.diffuse_damping, 1000.0f, 20000.0f), 0.001f);
    wow_enabled = p.wow_enabled;

    svf_hp_in.SetFreq(sm_hp_hz);
    overdrive.SetDrive(sm_drive);
    svf_fb_lp.SetFreq(sm_fb_lp_hz);
    svf_fb_hp.SetFreq(sm_fb_hp_hz);

    tilt_eq.SetTreble(sm_output_tilt_db / 6.0f);
    tilt_eq.SetBass(-sm_output_tilt_db / 6.0f);
    reverb.SetDamping(sm_diffuse_damping / 20000.0f); // Range 0-1
    wow_phasor.SetFreq(sm_wow_rate_hz);

    for (size_t i = 0; i < size; i++) {
        float sig = in[0][i];  // Mono — take Pod L channel

        if (bypass) {
            out[0][i] = out[1][i] = sig;
            continue;
        }

        // =====================================================
        // PHASE 2+ DSP CHAIN — replace stubs with real modules
        // =====================================================

        // --- 1. Input gain ---
        sig *= sm_input_gain;

        // --- 2. DC blocker ---
        sig = dcb.Process(sig);

        // --- 3. Input high-pass (Svf) ---
        svf_hp_in.Process(sig);
        sig = svf_hp_in.High();

        // --- 4. Soft drive (Overdrive or tanh) ---
        sig = overdrive.Process(sig);

        // --- 5. Delay core (DelayLine SDRAM) ---
        float current_delay = sm_delay_samples;
        if (wow_enabled) {
            float wow_val = sinf(wow_phasor.Process() * TWOPI_F);
            current_delay += wow_val * sm_wow_depth * 48000.f * 0.05f; // +-50ms wobble
        }
        delay_engine.SetDelay(current_delay);
        
        float delay_out = delay_engine.Read();
        
        // Calculate feedback filtering
        float fb_sig = delay_out * sm_feedback;
        
        svf_fb_hp.Process(fb_sig);
        fb_sig = svf_fb_hp.High();
        
        svf_fb_lp.Process(fb_sig);
        fb_sig = svf_fb_lp.Low();
        
        // Saturation applied on the feedback line to tame repeats
        fb_sig = tanhf(fb_sig);
        
        delay_engine.Write(sig + fb_sig);

        // --- Diffusion / FDN Reverb ---
        // Run the tap output through the reverb network for dense wash
        delay_out = reverb.Process(delay_out);

        // --- 6. Wet/dry mix ---
        sig = (sig * (1.f - sm_wet)) + (delay_out * sm_wet);

        // --- 7. Output tilt (ToneStack shelving) ---
        sig = tilt_eq.Process(sig);

        // --- 8. Limiter ---
        limiter.ProcessBlock(&sig, 1, 1.0f);

        // Dual-mono output
        out[0][i] = out[1][i] = sig;
    }
}

// ============================================================
// Main
// ============================================================

int main() {
    // ---- Hardware init --------------------------------------
    pod.Init();
    pod.SetAudioBlockSize(48);
    pod.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    float sr = pod.AudioSampleRate();
    (void)sr;  // Used by DSP init in Phase 2+

    // ---- External encoder init ------------------------------
    ext_enc.Init();

    // ---- Parameter init from factory defaults ---------------
    params_      = FxParams::Defaults();
    param_buf[0] = params_;   // Pre-load both buffers
    param_buf[1] = params_;
    active_buf_  = 0;

    // ---- Preset init ----------------------------------------
    InitPresets(presets_);

    // ---- UI state machine init ------------------------------
    ui.Init(&params_, presets_);

    // ---- Display init (must be before StartAudio) -----------
    display.Init();

    // ---- Phase 2+ DSP module init ---------------------------
    dcb.Init(sr);
    svf_hp_in.Init(sr);
    svf_hp_in.SetFreq(params_.hp_hz);
    svf_hp_in.SetRes(0.f);
    overdrive.Init();
    
    // Delay and feedback initialized
    delay_engine.Init();
    delay_engine.SetDelay(48000.f * 0.5f);
    svf_fb_lp.Init(sr); 
    svf_fb_lp.SetFreq(params_.fb_lp_hz); 
    svf_fb_lp.SetRes(0.f);
    svf_fb_hp.Init(sr); 
    svf_fb_hp.SetFreq(params_.fb_hp_hz); 
    svf_fb_hp.SetRes(0.f);

    sm_input_gain = params_.input_gain;
    sm_drive      = params_.drive;
    sm_hp_hz      = params_.hp_hz;
    sm_delay_samples = BpmToDelayMs(params_.bpm, params_.subdiv_idx) * (sr / 1000.f);
    sm_feedback   = params_.feedback;
    sm_fb_lp_hz   = params_.fb_lp_hz;
    sm_fb_hp_hz   = params_.fb_hp_hz;
    sm_wet        = params_.wet;

    // ---- Later-phase DSP module init -----------------------
    wow_phasor.Init(sr, params_.wow_rate_hz);
    tilt_eq.Init(sr);
    tilt_eq.SetMiddle(0.0f);
    reverb.Init(sr); 
    reverb.SetDecay(0.8f); 
    reverb.SetMix(0.3f);
    limiter.Init();

    sm_wow_depth       = params_.wow_depth;
    sm_wow_rate_hz     = params_.wow_rate_hz;
    sm_output_tilt_db  = params_.output_tilt_db;
    sm_diffuse_damping = params_.diffuse_damping;
    wow_enabled        = params_.wow_enabled;

    // ---- CRITICAL: StartAdc before StartAudio ---------------
    pod.StartAdc();
    pod.StartAudio(AudioCallback);

    // ---- Hardware Timer for low-latency control polling (1kHz) --
    TimerHandle::Config timer_cfg;
    timer_cfg.periph     = TimerHandle::Config::Peripheral::TIM_5;
    timer_cfg.enable_irq = true;
    control_timer.Init(timer_cfg);
    control_timer.SetPeriod(control_timer.GetFreq() / 1000);
    control_timer.SetCallback(ControlTimerCallback);
    control_timer.Start();

    // ---- Show boot screen -----------------------------------
    display.DrawPage(0, 0, false, false, false, params_);

    // ---- Main loop -----------------------------------------
    uint32_t last_draw_ms = 0;
    bool     draw_dirty   = true;
    DrawState last_drawn  = {};

    for (;;) {
        // Digital controls are polled in the Timer ISR (ControlTimerCallback) at 1kHz.
        // Analog controls (knobs/ADC) are processed in the AudioCallback at exactly 1kHz.

        // ---- SW1: Shift / Freeze momentary ------------------
        if (pod.button1.RisingEdge()) {
            sw1_press_time_ = System::GetNow();
            sw1_held_       = true;
            shift_consumed_ = false;
        }

        uint32_t sw1_held_ms = sw1_held_
            ? (System::GetNow() - sw1_press_time_) : 0;
        bool shift_active = sw1_held_ && (sw1_held_ms > 200);

        // Feed continuous page-aware controls before discrete dispatches so
        // UIState sees the current shift layer for push/button actions.
        int pod_enc_delta = pod.encoder.Increment();
        ui.PollControls(
            pod.knob1.Value(), // already processed by ProcessAnalogControls() above
            pod.knob2.Value(),
            pod_enc_delta,
            shift_active
        );

        if (pod.button1.FallingEdge()) {
            if (sw1_held_ms < 200 && !shift_consumed_) {
                // Short tap, shift never activated: toggle freeze momentary
                ui.Dispatch(UIEvent::FREEZE_INSTANT);
            }
            // Clear freeze momentary if it was a short press
            if (!params_.freeze_latch_mode) {
                params_.freeze_momentary = false;
            }
            sw1_held_ = false;
        }

        // ---- SW2: page-aware primary / secondary action --------
        if (pod.button2.RisingEdge()) {
            if (shift_active) {
                shift_consumed_ = true;
                ui.Dispatch(UIEvent::SW2_SHIFT);
            } else {
                ui.Dispatch(UIEvent::SW2_PRESS);
            }
        }

        // ---- Pod encoder ------------------------------------
        if (pod_enc_delta != 0) {
            if (shift_active) {
                shift_consumed_ = true;
                // Shift + Pod Enc turn is handled in PollControls().
            }
            // Page-aware encoder turns are applied in PollControls().
        }

        if (pod.encoder.RisingEdge()) {
            if (shift_active) {
                shift_consumed_ = true;
                ui.Dispatch(UIEvent::PAGE_JUMP);
            } else {
                ui.Dispatch(UIEvent::POD_ENC_PUSH);
            }
        }

        // ---- External encoder: menu navigation --------------
        int ext_delta = ext_enc.Increment();
        if (ext_delta > 0) ui.Dispatch(UIEvent::EXT_ENC_CW);
        if (ext_delta < 0) ui.Dispatch(UIEvent::EXT_ENC_CCW);

        if (ext_enc.EncoderPressed()) ui.Dispatch(UIEvent::EXT_PSH_SHORT);
        if (ext_enc.EncoderHeld())    ui.Dispatch(UIEvent::EXT_PSH_LONG);
        if (ext_enc.BackPressed())    ui.Dispatch(UIEvent::BAK);
        if (ext_enc.ConfirmPressed()) ui.Dispatch(UIEvent::CON);

        // ---- Enc direction from system page -----------------
        ext_enc.SetFlipped(params_.enc_flipped);

        // ---- Sync params to ISR via double-buffer swap ------
        // Write to the INACTIVE buffer, then atomically swap index.
        // ISR reads param_buf[active_buf_] — never the buffer being written.
        int write_buf        = 1 - active_buf_;
        param_buf[write_buf] = params_;
        __DSB();                  // Ensure write completes before index swap
        active_buf_          = write_buf;

        // ---- OLED redraw (event-driven, capped) -------------
        DrawState current_draw = {
            static_cast<int>(ui.current_page()),
            ui.cursor(),
            ui.edit_mode(),
            ui.shift_active(),
            ui.preset_selected(),
            params_
        };

        if (DrawStateChanged(current_draw, last_drawn)) {
            draw_dirty = true;
        }

        uint32_t now = System::GetNow();
        if (draw_dirty && (now - last_draw_ms >= 66)) {
            last_draw_ms = now;
            display.DrawPage(
                current_draw.page,
                current_draw.cursor,
                current_draw.edit_mode,
                current_draw.shift_active,
                current_draw.preset_selected,
                current_draw.params
            );
            last_drawn = current_draw;
            draw_dirty = false;
        }
    }
}
