#include "display.h"
#include "presets.h"
#include <stdio.h>
#include <string.h>

using namespace daisy;


// ---- Init ---------------------------------------------------

void Display::Init() {
    EdgeDisplay::Config cfg;
    auto& i2c          = cfg.driver_config.transport_config.i2c_config;
    i2c.periph         = I2CHandle::Config::Peripheral::I2C_1;
    i2c.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c.pin_config.scl = seed::D11;
    i2c.pin_config.sda = seed::D12;
    cfg.driver_config.transport_config.i2c_address = 0x3C;
    oled_.Init(cfg);

    oled_.Fill(false);
    oled_.SetCursor(0, 24);
    oled_.WriteString("  EDGE FX  ", Font_11x18, true);
    oled_.Update();
}

// ---- DrawPage dispatcher ------------------------------------

void Display::DrawPage(int page, int cursor, bool edit_mode,
                       bool shift_active, bool preset_selected,
                       const FxParams& p) {
    oled_.Fill(false);

    switch (page) {
        case 0: DrawP1_Perform(cursor, edit_mode, shift_active, p); break;
        case 1: DrawP2_Tone   (cursor, edit_mode, shift_active, p); break;
        case 2: DrawP3_Motion (cursor, edit_mode, shift_active, p); break;
        case 3: DrawP4_Freeze (cursor, edit_mode, shift_active, p); break;
        case 4: DrawP5_Presets(cursor, shift_active, preset_selected, p); break;
        case 5: DrawP6_System (cursor, edit_mode, shift_active, p); break;
        default: break;
    }

    oled_.Update();
}

// ---- Helpers ------------------------------------------------

void Display::Header(const char* name, bool shift, bool freeze) {
    // Invert the entire top row (y=0 to y=7)
    oled_.DrawRect(0, 0, 127, 7, true, true);

    // Page name — black text on white background
    oled_.SetCursor(0, 0);
    oled_.WriteString(name, Font_6x8, false);

    // Status badges on the right
    // Each badge is 3 chars: "SHF" or "FRZ", written right-aligned
    // Right-side status: "SHF" and/or "FRZ" (up to 7 chars including spaces)
    char badges[8];
    if      ( shift &&  freeze) { strncpy(badges, "SHF FRZ", 8); }
    else if ( shift && !freeze) { strncpy(badges, "SHF    ", 8); }
    else if (!shift &&  freeze) { strncpy(badges, "    FRZ", 8); }
    else                        { strncpy(badges, "       ", 8); }
    badges[7] = '\0';
    oled_.SetCursor(127 - 7 * 6, 0);  // 7 chars × 6px = 42px from right
    oled_.WriteString(badges, Font_6x8, false);
}

void Display::Row(int y, const char* label, const char* value,
                  bool selected, bool editing) {
    char buf[22];

    if (selected) {
        // Highlight: white rectangle, black text
        oled_.DrawRect(0, y, 127, y + 7, true, true);
        if (editing) {
            snprintf(buf, sizeof(buf), "[%-9s%9s]", label, value);
        } else {
            snprintf(buf, sizeof(buf), ">%-9s%9s", label, value);
        }
        oled_.SetCursor(0, y);
        oled_.WriteString(buf, Font_6x8, false);
    } else {
        snprintf(buf, sizeof(buf), " %-9s%9s", label, value);
        oled_.SetCursor(0, y);
        oled_.WriteString(buf, Font_6x8, true);
    }
}

void Display::Separator(int y) {
    oled_.DrawLine(0, y, 127, y, true);
}

// ---- P1: Perform --------------------------------------------
//
// Header: "PERFORM" + SHF/FRZ badges
// Row 1: Delay time / BPM
// Row 2: Feedback
// Row 3: Wet/Dry mix
// Row 4: Drive
// Row 5: Freeze status + mode
// Bottom: Mode indicator (SYNC/FREE)

void Display::DrawP1_Perform(int cursor, bool edit, bool shift, const FxParams& p) {
    bool freeze_on = p.freeze_momentary || p.freeze_latched;
    Header("PERFORM", shift, freeze_on);

    char val[16];   // 16 chars: enough for any value + unit suffix

    // Row 1 — Delay Time (label + BPM)
    if (p.sync_mode) {
        // "1/4   120BPM" — subdivision (5) + space + 3-digit BPM + "BPM" = 12 chars max
        snprintf(val, 16, "%s %dBPM",
                 kBeatDivs[p.subdiv_idx].label, (int)p.bpm);
    } else {
        // "500ms 120BPM" — 4-digit ms + "ms " + BPM
        snprintf(val, 16, "%dms %dBPM",
                 (int)p.delay_time_ms, (int)p.bpm);
    }
    Row(8,  "Time",     val, cursor == 0, edit && cursor == 0);

    // Row 2 — Feedback
    snprintf(val, sizeof(val), "%3d%%", (int)(p.feedback * 100.f));
    Row(16, "Feedback",  val, cursor == 1, edit && cursor == 1);

    // Row 3 — Mix
    snprintf(val, sizeof(val), "%3d%%", (int)(p.wet * 100.f));
    Row(24, "Wet/Dry",   val, cursor == 2, edit && cursor == 2);

    // Row 4 — Drive
    snprintf(val, sizeof(val), "%3d%%", (int)(p.drive * 100.f));
    Row(32, "Drive",     val, cursor == 3, edit && cursor == 3);

    // Row 5 — Freeze
    const char* frz_str = freeze_on
        ? (p.freeze_latched ? "LATCH" : "HOLD ")
        : "OFF  ";
    Row(40, "Freeze",  frz_str, cursor == 4, edit && cursor == 4);

    // Bottom status: delay mode
    Separator(50);
    oled_.SetCursor(0, 52);
    oled_.WriteString(p.sync_mode ? " SYNC  Sh+SW2=FREE" : " FREE  Sh+SW2=SYNC",
                      Font_6x8, true);
}

// ---- P2: Tone -----------------------------------------------
//
// All items edited via Ext Enc navigate + PSH edit.
// Shift + Knob 2 also controls Feedback LP (macro, noted at bottom).

void Display::DrawP2_Tone(int cursor, bool edit, bool shift, const FxParams& p) {
    Header("TONE", shift, false);

    char val[12];

    snprintf(val, sizeof(val), "%4dHz", (int)p.hp_hz);
    Row( 8, "Input HP",   val, cursor == 0, edit && cursor == 0);

    snprintf(val, sizeof(val), "%5dHz", (int)p.fb_lp_hz);
    Row(16, "FB Lo-P",    val, cursor == 1, edit && cursor == 1);

    snprintf(val, sizeof(val), "%4dHz", (int)p.fb_hp_hz);
    Row(24, "FB Hi-P",    val, cursor == 2, edit && cursor == 2);

    snprintf(val, sizeof(val), "%5dHz", (int)p.diffuse_damping);
    Row(32, "Wet Damp",   val, cursor == 3, edit && cursor == 3);

    // Tilt: show sign explicitly
    if (p.output_tilt_db >= 0.f)
        snprintf(val, sizeof(val), "+%4.1fdB", p.output_tilt_db);
    else
        snprintf(val, sizeof(val), " %4.1fdB", p.output_tilt_db);
    Row(40, "Out Tilt",   val, cursor == 4, edit && cursor == 4);

    Separator(50);
    oled_.SetCursor(0, 52);
    oled_.WriteString(shift ? "K1=FbHP K2=Damp Enc=Gn"
                            : "K1=HP   K2=FbLP Enc=Tl",
                      Font_6x8, true);
}

// ---- P3: Motion ---------------------------------------------

void Display::DrawP3_Motion(int cursor, bool edit, bool shift, const FxParams& p) {
    Header("MOTION", shift, false);

    char val[12];

    snprintf(val, sizeof(val), "%3d%%", (int)(p.wow_depth * 100.f));
    Row( 8, "Wow Depth",  val, cursor == 0, edit && cursor == 0);

    snprintf(val, sizeof(val), "%4.1fHz", p.wow_rate_hz);
    Row(16, "Wow Rate",   val, cursor == 1, edit && cursor == 1);

    Row(24, "Mod Enable", p.wow_enabled ? "ON " : "OFF", cursor == 2, edit && cursor == 2);

    Separator(50);
    oled_.SetCursor(0, 52);
    oled_.WriteString(shift ? "K1=FB  K2=Mix SW2=Tap "
                            : "K1=Wow K2=Rt  SW2=On ",
                      Font_6x8, true);
}

// ---- P4: Freeze ---------------------------------------------

void Display::DrawP4_Freeze(int cursor, bool edit, bool shift, const FxParams& p) {
    bool freeze_on = p.freeze_momentary || p.freeze_latched;
    Header("FREEZE", shift, freeze_on);

    Row( 8, "Mode",    p.freeze_latch_mode ? "Latch " : "Momentr", cursor == 0, edit && cursor == 0);
    Row(16, "Behavior",p.freeze_loop_mode  ? "Loop  " : "Hold   ", cursor == 1, edit && cursor == 1);

    char val[12];
    snprintf(val, sizeof(val), "%4dms", (int)p.freeze_size_ms);
    Row(24, "Loop Size", val, cursor == 2, edit && cursor == 2);

    Separator(50);
    oled_.SetCursor(0, 52);
    oled_.WriteString(shift ? "K1=Time K2=Mix SW2=Lat"
                            : "K1=Size K2=FB  SW2=Beh",
                      Font_6x8, true);
}

// ---- P5: Presets --------------------------------------------
// Scrollable window: shows 5 of the 10 presets at a time.
// Cursor can be anywhere in [0, kNumPresets-1].
// Window tracks cursor, kept centered when possible.

void Display::DrawP5_Presets(int cursor, bool shift, bool preset_selected, const FxParams& p) {
    (void)p;
    Header("PRESETS", shift, false);

    // Scrollable 5-item window over kNumPresets slots.
    int total    = kNumPresets;
    int kVisible = 5;
    int win      = cursor - 2;
    if (win < 0)                win = 0;
    if (win > total - kVisible) win = total - kVisible;
    if (win < 0)                win = 0;

    for (int i = 0; i < kVisible; i++) {
        int idx = win + i;
        if (idx >= total) break;

        char label[6];
        snprintf(label, sizeof(label), "P%02d: ", idx + 1);
        const char* name = kPresetNames[idx];
        bool selected = (cursor == idx);
        // Show brackets when this row is awaiting CON confirmation
        Row(8 + i * 8, label, name, selected, selected && preset_selected);
    }

    // Bottom separator + status line
    Separator(50);
    if (preset_selected) {
        // Preset highlighted and awaiting confirmation
        oled_.SetCursor(0, 52);
        oled_.WriteString("CON=LOAD  BAK=CANCEL", Font_6x8, true);
    } else {
        oled_.SetCursor(0, 52);
        oled_.WriteString("ENC/K1=SCRL CON=LOAD ", Font_6x8, true);
    }
}

// ---- P6: System ---------------------------------------------

void Display::DrawP6_System(int cursor, bool edit, bool shift, const FxParams& p) {
    Header("SYSTEM", shift, false);

    static const char* kBrightnessLabels[4] = { "25%   ", "50%   ", "75%   ", "100%  " };
    uint8_t idx = p.brightness_idx > 3 ? 3 : p.brightness_idx;

    Row( 8, "Brightness", kBrightnessLabels[idx], cursor == 0, edit && cursor == 0);
    Row(16, "Enc Dir", p.enc_flipped ? "Flip  " : "Normal",  cursor == 1, edit && cursor == 1);
    Row(24, "Bypass",  p.bypass      ? "ON    " : "OFF   ",  cursor == 2, edit && cursor == 2);

    Separator(34);
    oled_.SetCursor(0, 36);
    oled_.WriteString(" SR:48kHz  Blk:48  ", Font_6x8, true);
    oled_.SetCursor(0, 44);
    oled_.WriteString(" EDGE FX  v1.0.0   ", Font_6x8, true);
    oled_.SetCursor(0, 52);
    oled_.WriteString(shift ? "SW2=EncDir Enc=Bright "
                            : "SW2=Bypass Enc=Bright",
                      Font_6x8, true);
}
