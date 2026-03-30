#pragma once
#include "daisy_seed.h"
#include "dev/oled_ssd130x.h"

// ============================================================
// OledUI — OLED display + hierarchical menu engine
//
// Display: SSD1306 128×64, I²C address 0x3C, I2C_1 bus
// Pins: SCL = seed::D11 (P12), SDA = seed::D12 (P13)
//
// Call Draw() at ≤ 30 fps from the main loop ONLY — never
// from the audio callback. I²C is blocking.
// ============================================================

// ---- Display type alias ------------------------------------
using MyDisplay = daisy::OledDisplay<daisy::SSD130xI2c128x64Driver>;

// ============================================================
class OledUI {
public:
    // ---- Menu IDs ------------------------------------------
    static constexpr int MENU_ROOT   = 0;
    static constexpr int MENU_PATCH  = 1;
    static constexpr int MENU_PARAMS = 2;
    static constexpr int MENU_MIDI   = 3;
    static constexpr int MENU_SYSTEM = 4;
    static constexpr int MENU_COUNT  = 5;

    // Max items per page (fits 128×64 with title + status bars)
    static constexpr int MAX_ITEMS = 4;
    // Navigation stack depth (root → category → item edit)
    static constexpr int MAX_DEPTH = 3;

    // ---- Editable-value indices ----------------------------
    // vals_[0] = patch_idx       (0–3)
    // vals_[1] = midi_channel    (1–16)
    // vals_[2] = midi_thru       (0=off, 1=on)
    // vals_[3] = enc_flipped     (0=normal, 1=flip)
    // vals_[4] = disp_timeout    (0=off, 1=5s, 2=10s, 3=30s)
    static constexpr int VAL_COUNT = 5;

    // ---- Menu item ----------------------------------------
    enum class Action { SUBMENU, SELECT, TOGGLE, INT_RANGE };

    struct MenuItem {
        const char* label;
        Action      action;
        int         submenu_id;  // destination for SUBMENU
        int         val_idx;     // index into vals_ (editable)
        int         val_min;
        int         val_max;
    };

    struct MenuPage {
        const char* title;
        int         count;
        MenuItem    items[MAX_ITEMS];
    };

    // ---- Init -----------------------------------------------
    void Init() {
        // Configure I²C display
        MyDisplay::Config cfg;
        auto& i2c            = cfg.driver_config.transport_config.i2c_config;
        i2c.periph           = daisy::I2CHandle::Config::Peripheral::I2C_1;
        i2c.speed            = daisy::I2CHandle::Config::Speed::I2C_400KHZ;
        i2c.pin_config.scl   = daisy::seed::D11;
        i2c.pin_config.sda   = daisy::seed::D12;
        cfg.driver_config.transport_config.i2c_address = 0x3C;
        display_.Init(cfg);

        // Default values
        for(int i = 0; i < VAL_COUNT; i++) vals_[i] = 0;
        vals_[1] = 1;  // MIDI channel = 1

        // ---- Build menu pages --------------------------------
        // ROOT
        pages_[MENU_ROOT].title = "MAIN MENU";
        pages_[MENU_ROOT].count = 4;
        pages_[MENU_ROOT].items[0] = { "Patch Select",  Action::SUBMENU,   MENU_PATCH,  0, 0, 0 };
        pages_[MENU_ROOT].items[1] = { "Params",        Action::SUBMENU,   MENU_PARAMS, 0, 0, 0 };
        pages_[MENU_ROOT].items[2] = { "MIDI",          Action::SUBMENU,   MENU_MIDI,   0, 0, 0 };
        pages_[MENU_ROOT].items[3] = { "System",        Action::SUBMENU,   MENU_SYSTEM, 0, 0, 0 };

        // PATCH SELECT
        pages_[MENU_PATCH].title = "PATCH SELECT";
        pages_[MENU_PATCH].count = 4;
        pages_[MENU_PATCH].items[0] = { "Basic Synth",  Action::SELECT,    MENU_ROOT, 0, 0, 3 };
        pages_[MENU_PATCH].items[1] = { "Delay Echo",   Action::SELECT,    MENU_ROOT, 0, 1, 3 };
        pages_[MENU_PATCH].items[2] = { "Reverb Pad",   Action::SELECT,    MENU_ROOT, 0, 2, 3 };
        pages_[MENU_PATCH].items[3] = { "Filter Sweep", Action::SELECT,    MENU_ROOT, 0, 3, 3 };

        // PARAMS (display-only, driven by pots)
        pages_[MENU_PARAMS].title = "PARAMS";
        pages_[MENU_PARAMS].count = 4;
        pages_[MENU_PARAMS].items[0] = { "Delay Time", Action::SELECT, MENU_ROOT, 0, 0, 0 };
        pages_[MENU_PARAMS].items[1] = { "Feedback",   Action::SELECT, MENU_ROOT, 0, 0, 0 };
        pages_[MENU_PARAMS].items[2] = { "Mix",        Action::SELECT, MENU_ROOT, 0, 0, 0 };
        pages_[MENU_PARAMS].items[3] = { "Rate",       Action::SELECT, MENU_ROOT, 0, 0, 0 };

        // MIDI
        pages_[MENU_MIDI].title = "MIDI";
        pages_[MENU_MIDI].count = 2;
        pages_[MENU_MIDI].items[0] = { "Channel", Action::INT_RANGE, MENU_ROOT, 1, 1, 16 };
        pages_[MENU_MIDI].items[1] = { "Thru",    Action::TOGGLE,    MENU_ROOT, 2, 0,  1 };
        pages_[MENU_MIDI].items[2] = { "",         Action::SELECT,    MENU_ROOT, 0, 0,  0 };
        pages_[MENU_MIDI].items[3] = { "",         Action::SELECT,    MENU_ROOT, 0, 0,  0 };

        // SYSTEM
        pages_[MENU_SYSTEM].title = "SYSTEM";
        pages_[MENU_SYSTEM].count = 2;
        pages_[MENU_SYSTEM].items[0] = { "Invert Enc",   Action::TOGGLE,    MENU_ROOT, 3, 0, 1 };
        pages_[MENU_SYSTEM].items[1] = { "Disp Timeout", Action::INT_RANGE, MENU_ROOT, 4, 0, 3 };
        pages_[MENU_SYSTEM].items[2] = { "",              Action::SELECT,    MENU_ROOT, 0, 0, 0 };
        pages_[MENU_SYSTEM].items[3] = { "",              Action::SELECT,    MENU_ROOT, 0, 0, 0 };

        // Navigation state
        nav_stack_[0] = MENU_ROOT;
        nav_depth_    = 0;
        cursor_       = 0;
        editing_      = false;
        dirty_        = true;
        display_on_   = true;
        last_input_   = daisy::System::GetNow();

        // Zoom overlay (inactive by default)
        zoom_idx_   = -1;
        zoom_val_   = 0.f;
        zoom_label_ = nullptr;
        zoom_start_ = 0;
    }

    // ---- Navigation (call from main loop) ------------------

    void Scroll(int8_t delta) {
        WakeDisplay();
        if(editing_) {
            const MenuItem& it = pages_[CurrentMenu()].items[cursor_];
            if(it.action == Action::INT_RANGE || it.action == Action::TOGGLE) {
                vals_[it.val_idx] += delta;
                if(vals_[it.val_idx] < it.val_min) vals_[it.val_idx] = it.val_min;
                if(vals_[it.val_idx] > it.val_max) vals_[it.val_idx] = it.val_max;
            }
        } else {
            const MenuPage& pg = pages_[CurrentMenu()];
            cursor_ += delta;
            if(cursor_ < 0)         cursor_ = 0;
            if(cursor_ >= pg.count) cursor_ = pg.count - 1;
        }
        dirty_ = true;
    }

    void Confirm() {
        WakeDisplay();
        const MenuPage& pg = pages_[CurrentMenu()];
        if(cursor_ < 0 || cursor_ >= pg.count) return;
        const MenuItem& it = pg.items[cursor_];

        if(editing_) {
            editing_ = false;
            dirty_   = true;
            return;
        }

        switch(it.action) {
            case Action::SUBMENU:
                if(nav_depth_ < MAX_DEPTH - 1) {
                    nav_stack_[++nav_depth_] = it.submenu_id;
                    cursor_ = 0;
                    dirty_  = true;
                }
                break;

            case Action::SELECT:
                // In Patch Select: commit the highlighted row as active patch
                if(CurrentMenu() == MENU_PATCH) {
                    vals_[0] = cursor_;
                }
                Back();
                break;

            case Action::TOGGLE:
                vals_[it.val_idx] ^= 1;
                dirty_ = true;
                break;

            case Action::INT_RANGE:
                editing_ = true;
                dirty_   = true;
                break;
        }
    }

    void Back() {
        WakeDisplay();
        editing_ = false;
        if(nav_depth_ > 0) {
            nav_depth_--;
            cursor_ = 0;
        }
        dirty_ = true;
    }

    void BackToRoot() {
        WakeDisplay();
        editing_   = false;
        nav_depth_ = 0;
        cursor_    = 0;
        dirty_     = true;
    }

    // CON "Save & Exit":
    //   • While editing a value  → commit (value already latched), exit edit
    //                               mode, and go up one level
    //   • While navigating       → go up one level (same as Back)
    void SaveAndBack() {
        WakeDisplay();
        editing_ = false;
        if(nav_depth_ > 0) {
            nav_depth_--;
            cursor_ = 0;
        }
        dirty_ = true;
    }

    // Trigger a full-screen zoom popup for one parameter.
    // Stays visible for 1.2 s after the last call.
    // Call whenever Pot1/Pot2/PodEncoder changes.
    void SetZoom(int idx, float val, const char* label) {
        zoom_idx_   = idx;
        zoom_val_   = val;
        zoom_label_ = label;
        zoom_start_ = daisy::System::GetNow();
        dirty_      = true;
        WakeDisplay();
    }

    // ---- Getters for application logic ---------------------
    int  GetPatchIdx()    const { return vals_[0]; }
    int  GetMidiChannel() const { return vals_[1]; }
    bool GetMidiThru()    const { return vals_[2] != 0; }
    bool GetEncFlipped()  const { return vals_[3] != 0; }
    int  GetDispTimeout() const { return vals_[4]; }

    // ---- Draw (call at ~30 fps from main loop ONLY) --------
    void Draw(const float* params, int /*patch_idx*/) {
        // Display timeout
        static const uint32_t kTimeoutMs[4] = { 0, 5000, 10000, 30000 };
        uint32_t now = daisy::System::GetNow();
        uint32_t to  = kTimeoutMs[vals_[4]];
        if(to > 0 && (now - last_input_ > to)) {
            if(display_on_) {
                display_.Fill(false);
                display_.Update();
                display_on_ = false;
            }
            return;
        }

        bool zoom_active = (zoom_idx_ >= 0 && now - zoom_start_ < 1200);
        if(!zoom_active && !dirty_) return;
        dirty_ = false;

        display_.Fill(false);

        if(zoom_active) {
            DrawZoomOverlay();
            dirty_ = true;  // keep redrawing until zoom expires
        } else {
            zoom_idx_ = -1;
            int menu_id = CurrentMenu();
            if(menu_id == MENU_PARAMS)
                DrawParamsPage(params);
            else
                DrawMenuPage(menu_id, params);
        }

        display_.Update();
    }

private:
    MyDisplay  display_;
    MenuPage   pages_[MENU_COUNT];
    int        vals_[VAL_COUNT];

    int      nav_stack_[MAX_DEPTH];
    int      nav_depth_;
    int      cursor_;
    bool     editing_;
    bool     dirty_;
    bool     display_on_;
    uint32_t last_input_;

    // Zoom overlay state
    int          zoom_idx_;    // -1 = inactive
    float        zoom_val_;
    const char*  zoom_label_;
    uint32_t     zoom_start_;

    int CurrentMenu() const { return nav_stack_[nav_depth_]; }

    void WakeDisplay() {
        last_input_ = daisy::System::GetNow();
        display_on_ = true;
        dirty_      = true;
    }

    // --------------------------------------------------------
    // DrawZoomOverlay — full-screen single-param view
    //
    // 128×64 layout:
    //   y=2    Label (Font_6x8)
    //   y=18   Large percentage value (Font_11x18)
    //   y=40   Raw float value (Font_6x8)
    //   y=50   Progress bar (full width)
    // --------------------------------------------------------
    void DrawZoomOverlay() {
        char buf[24];

        // Param label (top-left, small)
        display_.SetCursor(2, 2);
        display_.WriteString(zoom_label_ ? zoom_label_ : "Param", Font_6x8, true);

        // Large percentage (center)
        int pct = static_cast<int>(zoom_val_ * 100.f);
        snprintf(buf, sizeof(buf), "%3d%%", pct);
        display_.SetCursor(24, 18);
        display_.WriteString(buf, Font_11x18, true);

        // Raw float (below large text)
        snprintf(buf, sizeof(buf), "%.3f", zoom_val_);
        display_.SetCursor(44, 40);
        display_.WriteString(buf, Font_6x8, true);

        // Progress bar
        display_.DrawRect(0, 50, 127, 58, true, false);   // outline
        int fill = static_cast<int>(zoom_val_ * 127.f);
        if(fill > 0)
            display_.DrawRect(0, 50, fill, 58, true, true);
    }

    // --------------------------------------------------------
    // DrawMenuPage — standard list layout
    //
    // 128×64 layout:
    //   y=0-8    Title bar (inverted)
    //   y=10-51  Up to 4 items × 11px each
    //   y=56-63  Status bar
    // --------------------------------------------------------
    void DrawMenuPage(int menu_id, const float* params) {
        const MenuPage& pg = pages_[menu_id];
        char buf[32];

        // Title bar: filled white rectangle, black text
        display_.DrawRect(0, 0, 127, 9, true, true);
        display_.SetCursor(2, 1);
        display_.WriteString(pg.title, Font_6x8, false);

        // Items
        for(int i = 0; i < pg.count; i++) {
            const MenuItem& it = pg.items[i];
            if(it.label == nullptr || it.label[0] == '\0') continue;

            int  y        = 10 + i * 11;
            bool selected = (i == cursor_);

            // Highlight selected row
            if(selected) {
                display_.DrawRect(0, y - 1, 127, y + 8, true, true);
            }

            // Cursor glyph
            display_.SetCursor(1, y);
            display_.WriteString(selected ? ">" : " ", Font_6x8, !selected);

            // Label
            display_.SetCursor(9, y);
            display_.WriteString(it.label, Font_6x8, !selected);

            // Value suffix (right-justified at x=84)
            if(it.action == Action::TOGGLE) {
                const char* vstr = vals_[it.val_idx] ? "[ON] " : "[OFF]";
                display_.SetCursor(84, y);
                display_.WriteString(vstr, Font_6x8, !selected);
            } else if(it.action == Action::INT_RANGE) {
                // Show value; if System/Disp Timeout show friendly string
                if(menu_id == MENU_SYSTEM && it.val_idx == 4) {
                    static const char* kToStr[4] = { "Off", "5s", "10s", "30s" };
                    int vi = vals_[it.val_idx];
                    if(vi < 0) vi = 0;
                    if(vi > 3) vi = 3;
                    display_.SetCursor(96, y);
                    display_.WriteString(kToStr[vi], Font_6x8, !selected);
                } else {
                    snprintf(buf, sizeof(buf), "%d", vals_[it.val_idx]);
                    display_.SetCursor(108, y);
                    display_.WriteString(buf, Font_6x8, !selected);
                }
                // Show edit indicator
                if(selected && editing_) {
                    display_.SetCursor(120, y);
                    display_.WriteString("*", Font_6x8, !selected);
                }
            }

            // Patch-select: mark the active patch with a bullet
            if(menu_id == MENU_PATCH && it.action == Action::SELECT) {
                if(vals_[0] == i) {
                    display_.SetCursor(120, y);
                    display_.WriteString("*", Font_6x8, !selected);
                }
            }
        }

        // Status bar: POT1 value | active patch name
        static const char* kPatchNames[4] = { "BscSyn", "DlyEch", "RevPad", "FltSwp" };
        int pidx = vals_[0];
        if(pidx < 0) pidx = 0;
        if(pidx > 3) pidx = 3;

        snprintf(buf, sizeof(buf), "P1:%.2f", params ? params[0] : 0.f);
        display_.SetCursor(0, 56);
        display_.WriteString(buf, Font_6x8, true);

        display_.SetCursor(78, 56);
        display_.WriteString(kPatchNames[pidx], Font_6x8, true);
    }

    // --------------------------------------------------------
    // DrawParamsPage — bar-graph parameter display
    //
    //   y=0-8   Title bar
    //   y=10    Param 0 row
    //   y=21    Param 1 row
    //   y=32    Param 2 row
    //   y=43    Param 3 row
    //   y=56    Status: POT1/POT2 values
    //
    // Bar area: x=50 to x=90 (40px wide), height = 7px
    // --------------------------------------------------------
    void DrawParamsPage(const float* params) {
        char buf[32];

        // Title bar
        display_.DrawRect(0, 0, 127, 9, true, true);
        display_.SetCursor(2, 1);
        display_.WriteString("PARAMS", Font_6x8, false);

        static const char* kLabels[4] = {
            "DlyTime", "Feedbck", "Mix    ", "Rate   "
        };

        for(int i = 0; i < 4; i++) {
            int   y   = 10 + i * 11;
            float val = params ? params[i] : 0.f;
            if(val < 0.f) val = 0.f;
            if(val > 1.f) val = 1.f;

            // Label (7 chars × 6px = 42px, ends at x=42)
            display_.SetCursor(0, y);
            display_.WriteString(kLabels[i], Font_6x8, true);

            // Bar outline: x=44 to x=84 (40px wide)
            display_.DrawRect(44, y, 84, y + 7, true, false);

            // Bar fill
            int fill_w = static_cast<int>(val * 40.f);
            if(fill_w > 0)
                display_.DrawRect(44, y, 44 + fill_w, y + 7, true, true);

            // Numeric value (4 chars, x=88)
            snprintf(buf, sizeof(buf), "%.2f", val);
            display_.SetCursor(88, y);
            display_.WriteString(buf, Font_6x8, true);
        }

        // Status bar: POT1 and POT2 values
        snprintf(buf, sizeof(buf), "P1:%.2f P2:%.2f",
                 params ? params[0] : 0.f,
                 params ? params[1] : 0.f);
        display_.SetCursor(0, 56);
        display_.WriteString(buf, Font_6x8, true);
    }
};
