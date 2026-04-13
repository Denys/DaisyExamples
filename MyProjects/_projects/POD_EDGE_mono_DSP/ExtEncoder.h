#pragma once
#include "daisy_seed.h"
#include "per/gpio.h"

// ============================================================
// ExtEncoder — External OLED-Rotary-Encoder module wrapper
// Wired to Daisy Pod (old DIP-socket revision)
//
// All buttons are active-LOW with internal pull-ups.
// Call Debounce() at ~1 kHz from the main loop.
//
// BAK uses daisy::GPIO + manual debounce rather than
// daisy::Switch, because pod.Init() configures D17 (LED2_R)
// as a push-pull OUTPUT. daisy::Switch::RisingEdge() depends
// on an internal 'updated_' flag that can be affected by the
// GPIO state left by the LED driver. Direct GPIO read with our
// own shift-register debounce is immune to this.
// ============================================================

class ExtEncoder {
public:
    // ---- Pin definitions (old Pod DIP socket) ----------------
    static constexpr daisy::Pin TRA_PIN = daisy::seed::D7;   // P8  White  — encoder A-phase
    static constexpr daisy::Pin TRB_PIN = daisy::seed::D8;   // P9  Gray   — encoder B-phase
    static constexpr daisy::Pin PSH_PIN = daisy::seed::D9;   // P10 Brown  — encoder push-button

    // CON = CONFIRM (KEY1 on PCB silkscreen)
    static constexpr daisy::Pin CON_PIN = daisy::seed::D10;  // P11 Green

    // BAK = BACK (KEYO on PCB silkscreen)
    // Wired to P29 = seed::D22 (PA5 / A7) — no Pod peripheral conflict.
    // NOTE: D17 (P24, LED2_R) is driven OUTPUT by pod.Init() and must NOT be used.
    static constexpr daisy::Pin BAK_PIN = daisy::seed::D22;  // P29 A7 Cherry

    // ---- Public API -----------------------------------------

    void Init() {
        // Encoder: TRA/TRB/PSH with 1 kHz update rate
        enc_.Init(TRA_PIN, TRB_PIN, PSH_PIN, 1000.f);

        // CON button: active-LOW, internal pull-up via daisy::Switch
        con_.Init(CON_PIN, 1000.f,
                  daisy::Switch::TYPE_MOMENTARY,
                  daisy::Switch::POLARITY_INVERTED,
                  daisy::Switch::PULL_UP);

        // BAK: direct GPIO read — bypasses daisy::Switch to avoid
        // LED2_R (D17) OUTPUT-mode interference from pod.Init().
        // Use a local copy of BAK_PIN to avoid ODR-use of static
        // constexpr in C++14 (which would require an out-of-class def).
        daisy::Pin bak_p = BAK_PIN;
        bak_gpio_.Init(bak_p,
                       daisy::GPIO::Mode::INPUT,
                       daisy::GPIO::Pull::PULLUP);
        bak_state_       = 0x00;
        bak_rising_      = false;
        bak_last_update_ = daisy::System::GetNow();

        held_start_     = 0;
        held_triggered_ = false;
        flipped_        = false;
    }

    // Call at ~1 kHz (every 1 ms) from the main loop
    void Debounce() {
        enc_.Debounce();
        con_.Debounce();

        // ---- Manual BAK debounce (shift-register, 8-bit) ----
        // Matches daisy::Switch logic: RisingEdge = state 0x7f
        // active-LOW: pin LOW (=false) means pressed → state bit = 1
        uint32_t now = daisy::System::GetNow();
        bak_rising_ = false;
        if(now - bak_last_update_ >= 1) {
            bak_last_update_ = now;
            uint8_t bit  = bak_gpio_.Read() ? 0u : 1u;  // LOW = pressed = 1
            bak_state_   = static_cast<uint8_t>((bak_state_ << 1) | bit);
            bak_rising_  = (bak_state_ == 0x7f);         // 7 ms of rising edge
        }

        // Track PSH hold time
        if(enc_.Pressed()) {
            if(held_start_ == 0)
                held_start_ = daisy::System::GetNow();
        } else {
            held_start_     = 0;
            held_triggered_ = false;
        }
    }

    // +1 CW, -1 CCW, 0 no change. Respects flipped_ flag.
    int8_t Increment() {
        int8_t v = static_cast<int8_t>(enc_.Increment());
        return flipped_ ? -v : v;
    }

    // PSH just-pressed (rising edge)
    bool EncoderPressed() { return enc_.RisingEdge(); }

    // PSH held > 500 ms — returns true ONCE per hold event
    bool EncoderHeld() {
        if(held_start_ != 0 && !held_triggered_ &&
           (daisy::System::GetNow() - held_start_ > 500)) {
            held_triggered_ = true;
            return true;
        }
        return false;
    }

    // CON (CONFIRM / KEY1) just-pressed
    bool ConfirmPressed() { return con_.RisingEdge(); }

    // BAK (BACK / KEYO) just-pressed — manual debounce, valid only
    // after Debounce() has been called in the current 1 ms tick
    bool BackPressed()  { return bak_rising_; }

    // BAK held (raw, no debounce) — useful for diagnostics
    bool BackHeld()     { return bak_state_ == 0xff; }

    // BAK raw GPIO read — true whenever pin is physically LOW (pressed)
    bool BakRawPressed() { return !bak_gpio_.Read(); }

    // Toggle encoder rotation direction (callable from System menu)
    void  SetFlipped(bool f) { flipped_ = f; }
    bool  GetFlipped() const { return flipped_; }

private:
    daisy::Encoder enc_;   // TRA / TRB / PSH
    daisy::Switch  con_;   // CON button (Switch is fine here — no pod pin conflict)

    // BAK: raw GPIO + manual 8-bit shift-register debounce
    daisy::GPIO  bak_gpio_;
    uint8_t      bak_state_;
    bool         bak_rising_;
    uint32_t     bak_last_update_;

    uint32_t held_start_;
    bool     held_triggered_;
    bool     flipped_;
};
