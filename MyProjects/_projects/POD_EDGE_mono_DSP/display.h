#pragma once
#include "daisy_pod.h"
#include "dev/oled_ssd130x.h"
#include "parameters.h"

// ============================================================
// Display — OLED renderer for EDGE Performance FX
//
// SSD1306 128x64, I2C address 0x3C
// SCL = seed::D11  SDA = seed::D12  (I2C_1, 400 kHz)
//
// Font_6x8 throughout: 21 chars × 8 rows per screen.
// Row y-offsets: 0, 8, 16, 24, 32, 40, 48, 56
//
// Layout per page:
//   y=0  : Header — page name (left) + status badges (right)
//   y=8  : Item row 1
//   y=16 : Item row 2
//   y=24 : Item row 3
//   y=32 : Item row 4
//   y=40 : Item row 5
//   y=48 : Item row 6 (some pages)
//   y=56 : (unused or extra status)
// ============================================================

using EdgeDisplay = daisy::OledDisplay<daisy::SSD130xI2c128x64Driver>;

class Display {
public:
    void Init();

    // Render a full page to the OLED buffer and push.
    // Call at ~30 fps from the main loop (never from ISR).
    void DrawPage(int    page,
                  int    cursor,
                  bool   edit_mode,
                  bool   shift_active,
                  bool   preset_selected,
                  const FxParams& p);

private:
    EdgeDisplay oled_;

    // ---- Per-page renderers ---------------------------------
    void DrawP1_Perform(int cursor, bool edit, bool shift, const FxParams& p);
    void DrawP2_Tone   (int cursor, bool edit, bool shift, const FxParams& p);
    void DrawP3_Motion (int cursor, bool edit, bool shift, const FxParams& p);
    void DrawP4_Freeze (int cursor, bool edit, bool shift, const FxParams& p);
    void DrawP5_Presets(int cursor, bool shift, bool preset_selected, const FxParams& p);
    void DrawP6_System (int cursor, bool edit, bool shift, const FxParams& p);

    // ---- Helpers --------------------------------------------

    // Header: page name left-justified, badges right-justified.
    // Inverted background for the whole header row.
    void Header(const char* name, bool shift, bool freeze);

    // Single parameter row.
    //   y        — pixel y coordinate
    //   label    — parameter name (max ~10 chars)
    //   value    — formatted value string (max ~9 chars)
    //   selected — cursor is on this row
    //   editing  — currently editing this value (brackets shown)
    void Row(int y, const char* label, const char* value,
             bool selected, bool editing);

    // Horizontal separator line across full width
    void Separator(int y);
};
