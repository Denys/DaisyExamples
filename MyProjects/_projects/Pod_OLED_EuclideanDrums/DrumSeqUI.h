#pragma once
#include "daisy_seed.h"
#include "dev/oled_ssd130x.h"

// ============================================================
// DrumSeqUI — OLED display engine for euclidean drum sequencer
//
// SSD1306 128x64, I2C address 0x3C, I2C_1 bus
// SCL = seed::D11 (P12), SDA = seed::D12 (P13)
//
// Three display modes:
//   SEQUENCER — dot-grid pattern view (home screen)
//   MENU      — hierarchical parameter pages
//   ZOOM      — full-screen single-param overlay (1.2s timeout)
// ============================================================

using DrumDisplay = daisy::OledDisplay<daisy::SSD130xI2c128x64Driver>;

class DrumSeqUI {
public:
    // ---- Display modes --------------------------------------
    enum class Mode { SEQUENCER, MENU };

    // ---- Menu page IDs --------------------------------------
    static constexpr int MENU_ROOT   = 0;
    static constexpr int MENU_KICK   = 1;
    static constexpr int MENU_SNARE  = 2;
    static constexpr int MENU_GLOBAL = 3;
    static constexpr int MENU_SYSTEM = 4;
    static constexpr int MENU_COUNT  = 5;

    static constexpr int MAX_ITEMS = 4;
    static constexpr int MAX_DEPTH = 3;

    // ---- Value store indices --------------------------------
    static constexpr int V_KICK_DENSITY  = 0;   // 0-100 %
    static constexpr int V_KICK_LENGTH   = 1;   // 1-32
    static constexpr int V_KICK_DECAY    = 2;   // 5-100 (x10ms)
    static constexpr int V_KICK_PITCH_HI = 3;   // 50-800 Hz
    static constexpr int V_SNARE_DENSITY = 4;   // 0-100 %
    static constexpr int V_SNARE_LENGTH  = 5;   // 1-32
    static constexpr int V_SNARE_DECAY   = 6;   // 5-100 (x10ms)
    static constexpr int V_SNARE_ROT     = 7;   // 0-31
    static constexpr int V_TEMPO         = 8;   // 30-300 BPM
    static constexpr int V_VOLUME        = 9;   // 0-100 %
    static constexpr int V_MIX           = 10;  // 0-100 % (50=equal)
    static constexpr int V_KICK_ROT      = 11;  // 0-31
    static constexpr int V_ENC_FLIP      = 12;  // 0/1
    static constexpr int V_DISP_TO       = 13;  // 0-3
    static constexpr int VAL_COUNT       = 14;

    // ---- Menu item types ------------------------------------
    enum class Action { SUBMENU, TOGGLE, INT_RANGE };

    struct MenuItem {
        const char* label;
        Action      action;
        int         submenu_id;
        int         val_idx;
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
        DrumDisplay::Config cfg;
        auto& i2c          = cfg.driver_config.transport_config.i2c_config;
        i2c.periph         = daisy::I2CHandle::Config::Peripheral::I2C_1;
        i2c.speed          = daisy::I2CHandle::Config::Speed::I2C_400KHZ;
        i2c.pin_config.scl = daisy::seed::D11;
        i2c.pin_config.sda = daisy::seed::D12;
        cfg.driver_config.transport_config.i2c_address = 0x3C;
        display_.Init(cfg);

        // Defaults
        for(int i = 0; i < VAL_COUNT; i++) vals_[i] = 0;
        vals_[V_KICK_DENSITY]  = 50;
        vals_[V_KICK_LENGTH]   = 16;
        vals_[V_KICK_DECAY]    = 50;   // 500ms
        vals_[V_KICK_PITCH_HI] = 400;
        vals_[V_SNARE_DENSITY] = 50;
        vals_[V_SNARE_LENGTH]  = 16;
        vals_[V_SNARE_DECAY]   = 20;   // 200ms
        vals_[V_TEMPO]         = 120;
        vals_[V_VOLUME]        = 80;
        vals_[V_MIX]           = 50;

        // Build menu pages
        pages_[MENU_ROOT].title = "MENU";
        pages_[MENU_ROOT].count = 4;
        pages_[MENU_ROOT].items[0] = { "Kick",   Action::SUBMENU, MENU_KICK,   0, 0, 0 };
        pages_[MENU_ROOT].items[1] = { "Snare",  Action::SUBMENU, MENU_SNARE,  0, 0, 0 };
        pages_[MENU_ROOT].items[2] = { "Global", Action::SUBMENU, MENU_GLOBAL, 0, 0, 0 };
        pages_[MENU_ROOT].items[3] = { "System", Action::SUBMENU, MENU_SYSTEM, 0, 0, 0 };

        pages_[MENU_KICK].title = "KICK";
        pages_[MENU_KICK].count = 4;
        pages_[MENU_KICK].items[0] = { "Density",  Action::INT_RANGE, 0, V_KICK_DENSITY,  0, 100 };
        pages_[MENU_KICK].items[1] = { "Length",   Action::INT_RANGE, 0, V_KICK_LENGTH,   1, 32  };
        pages_[MENU_KICK].items[2] = { "Decay",    Action::INT_RANGE, 0, V_KICK_DECAY,    5, 100 };
        pages_[MENU_KICK].items[3] = { "Pitch Hi", Action::INT_RANGE, 0, V_KICK_PITCH_HI, 50, 800 };

        pages_[MENU_SNARE].title = "SNARE";
        pages_[MENU_SNARE].count = 4;
        pages_[MENU_SNARE].items[0] = { "Density",  Action::INT_RANGE, 0, V_SNARE_DENSITY, 0, 100 };
        pages_[MENU_SNARE].items[1] = { "Length",   Action::INT_RANGE, 0, V_SNARE_LENGTH,  1, 32  };
        pages_[MENU_SNARE].items[2] = { "Decay",    Action::INT_RANGE, 0, V_SNARE_DECAY,   5, 100 };
        pages_[MENU_SNARE].items[3] = { "Rotation", Action::INT_RANGE, 0, V_SNARE_ROT,     0, 31  };

        pages_[MENU_GLOBAL].title = "GLOBAL";
        pages_[MENU_GLOBAL].count = 4;
        pages_[MENU_GLOBAL].items[0] = { "Tempo",   Action::INT_RANGE, 0, V_TEMPO,    30, 300 };
        pages_[MENU_GLOBAL].items[1] = { "Volume",  Action::INT_RANGE, 0, V_VOLUME,   0,  100 };
        pages_[MENU_GLOBAL].items[2] = { "K/S Mix", Action::INT_RANGE, 0, V_MIX,      0,  100 };
        pages_[MENU_GLOBAL].items[3] = { "K Rotatn", Action::INT_RANGE, 0, V_KICK_ROT, 0,  31  };

        pages_[MENU_SYSTEM].title = "SYSTEM";
        pages_[MENU_SYSTEM].count = 2;
        pages_[MENU_SYSTEM].items[0] = { "Invert Enc",   Action::TOGGLE,    0, V_ENC_FLIP, 0, 1 };
        pages_[MENU_SYSTEM].items[1] = { "Disp Timeout", Action::INT_RANGE, 0, V_DISP_TO,  0, 3 };

        nav_stack_[0] = MENU_ROOT;
        nav_depth_    = 0;
        cursor_       = 0;
        editing_      = false;
        dirty_        = true;
        mode_         = Mode::SEQUENCER;
        active_drum_  = 0;

        zoom_idx_   = -1;
        zoom_val_   = 0.f;
        zoom_label_ = nullptr;
        zoom_start_ = 0;

        display_on_ = true;
        last_input_ = daisy::System::GetNow();
    }

    // ---- Navigation -----------------------------------------

    void Scroll(int8_t delta) {
        WakeDisplay();
        if(mode_ == Mode::SEQUENCER) return;

        if(editing_) {
            const MenuItem& it = pages_[CurrentMenu()].items[cursor_];
            vals_[it.val_idx] += delta;
            if(vals_[it.val_idx] < it.val_min) vals_[it.val_idx] = it.val_min;
            if(vals_[it.val_idx] > it.val_max) vals_[it.val_idx] = it.val_max;
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
        if(mode_ == Mode::SEQUENCER) {
            mode_ = Mode::MENU;
            nav_stack_[0] = MENU_ROOT;
            nav_depth_ = 0;
            cursor_ = 0;
            editing_ = false;
            dirty_ = true;
            return;
        }

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
        if(mode_ == Mode::SEQUENCER) return;

        editing_ = false;
        if(nav_depth_ > 0) {
            nav_depth_--;
            cursor_ = 0;
        } else {
            mode_ = Mode::SEQUENCER;
        }
        dirty_ = true;
    }

    void SaveAndBack() {
        WakeDisplay();
        if(mode_ == Mode::SEQUENCER) return;
        editing_ = false;
        if(nav_depth_ > 0) {
            nav_depth_--;
            cursor_ = 0;
        } else {
            mode_ = Mode::SEQUENCER;
        }
        dirty_ = true;
    }

    void BackToRoot() {
        WakeDisplay();
        editing_   = false;
        nav_depth_ = 0;
        cursor_    = 0;
        mode_      = Mode::SEQUENCER;
        dirty_     = true;
    }

    void SetZoom(int idx, float val, const char* label) {
        zoom_idx_   = idx;
        zoom_val_   = val;
        zoom_label_ = label;
        zoom_start_ = daisy::System::GetNow();
        dirty_      = true;
        WakeDisplay();
    }

    // ---- Value access ---------------------------------------

    int  GetVal(int idx) const { return (idx >= 0 && idx < VAL_COUNT) ? vals_[idx] : 0; }
    void SetVal(int idx, int v) {
        if(idx >= 0 && idx < VAL_COUNT) {
            const MenuItem* item = FindItemByVal(idx);
            if(item) {
                if(v < item->val_min) v = item->val_min;
                if(v > item->val_max) v = item->val_max;
            }
            vals_[idx] = v;
            dirty_ = true;
        }
    }
    bool  GetEncFlipped()  const { return vals_[V_ENC_FLIP] != 0; }
    Mode  GetMode()        const { return mode_; }
    void  SetActiveDrum(int d) { active_drum_ = d; dirty_ = true; }

    // ---- Draw -----------------------------------------------

    void Draw(const volatile bool* kickSeq, const volatile bool* snareSeq,
              uint8_t kickStep, uint8_t snareStep)
    {
        // Display timeout
        static const uint32_t kTimeoutMs[4] = { 0, 5000, 10000, 30000 };
        uint32_t now = daisy::System::GetNow();
        uint32_t to  = kTimeoutMs[vals_[V_DISP_TO]];
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
            dirty_ = true;
        } else if(mode_ == Mode::SEQUENCER) {
            DrawSequencerPage(kickSeq, snareSeq, kickStep, snareStep);
        } else {
            DrawMenuPage(CurrentMenu());
        }

        display_.Update();
    }

private:
    DrumDisplay display_;
    MenuPage    pages_[MENU_COUNT];
    int         vals_[VAL_COUNT];

    int  nav_stack_[MAX_DEPTH];
    int  nav_depth_;
    int  cursor_;
    bool editing_;
    bool dirty_;
    Mode mode_;
    int  active_drum_;   // 0=kick, 1=snare

    int          zoom_idx_;
    float        zoom_val_;
    const char*  zoom_label_;
    uint32_t     zoom_start_;

    bool     display_on_;
    uint32_t last_input_;

    int CurrentMenu() const { return nav_stack_[nav_depth_]; }

    void WakeDisplay() {
        last_input_ = daisy::System::GetNow();
        display_on_ = true;
        dirty_      = true;
    }

    const MenuItem* FindItemByVal(int val_idx) const {
        for(int p = 0; p < MENU_COUNT; p++)
            for(int i = 0; i < pages_[p].count; i++)
                if(pages_[p].items[i].val_idx == val_idx)
                    return &pages_[p].items[i];
        return nullptr;
    }

    // ---- Sequencer page -------------------------------------
    // y=0-9   Title bar "EUCLIDEAN DRUMS"
    // y=12    Kick info: "K:4/16  BPM:120"
    // y=22-26 Kick dot grid (32 steps x 4px)
    // y=29    Snare info: "S:3/12  R:0"
    // y=39-43 Snare dot grid
    // y=56    Status: ">KICK  Vol:80%"

    void DrawSequencerPage(const volatile bool* kickSeq, const volatile bool* snareSeq,
                           uint8_t kickStep, uint8_t snareStep)
    {
        char buf[24];

        // Title bar
        display_.DrawRect(0, 0, 127, 9, true, true);
        display_.SetCursor(2, 1);
        display_.WriteString("EUCLIDEAN DRUMS", Font_6x8, false);

        // Kick info
        int kd = vals_[V_KICK_DENSITY];
        int kl = vals_[V_KICK_LENGTH];
        int kh = (int)(kl * kd / 100.f + 0.5f);
        snprintf(buf, 24, "K:%d/%d", kh, kl);
        display_.SetCursor(0, 12);
        display_.WriteString(buf, Font_6x8, true);

        snprintf(buf, 24, "BPM:%d", vals_[V_TEMPO]);
        display_.SetCursor(78, 12);
        display_.WriteString(buf, Font_6x8, true);

        // Kick dot grid
        DrawPatternRow(kickSeq, kl, kickStep, 22);

        // Snare info
        int sd = vals_[V_SNARE_DENSITY];
        int sl = vals_[V_SNARE_LENGTH];
        int sh = (int)(sl * sd / 100.f + 0.5f);
        snprintf(buf, 24, "S:%d/%d", sh, sl);
        display_.SetCursor(0, 29);
        display_.WriteString(buf, Font_6x8, true);

        int sr = vals_[V_SNARE_ROT];
        if(sr > 0) {
            snprintf(buf, 24, "R:%d", sr);
            display_.SetCursor(78, 29);
            display_.WriteString(buf, Font_6x8, true);
        }

        // Snare dot grid
        DrawPatternRow(snareSeq, sl, snareStep, 39);

        // Status bar
        snprintf(buf, 24, "%s  Vol:%d%%",
                 active_drum_ == 0 ? ">KICK" : ">SNRE",
                 vals_[V_VOLUME]);
        display_.SetCursor(0, 56);
        display_.WriteString(buf, Font_6x8, true);
    }

    void DrawPatternRow(const volatile bool* seq, int len, uint8_t curStep, int y) {
        if(len < 1) len = 1;
        if(len > 32) len = 32;
        for(int i = 0; i < len; i++) {
            int x = i * 4;
            if(i == curStep) {
                // Current step: inverted 3x5 column
                display_.DrawRect(x, y - 1, x + 2, y + 3, true, true);
            } else if(seq[i]) {
                // Active hit: filled 3x3
                display_.DrawRect(x, y, x + 2, y + 2, true, true);
            } else {
                // Inactive: single center pixel
                display_.DrawPixel(x + 1, y + 1, true);
            }
        }
        // End-of-pattern marker
        if(len < 32) {
            int xe = len * 4;
            display_.DrawLine(xe, y - 1, xe, y + 3, true);
        }
    }

    // ---- Menu page ------------------------------------------

    void DrawMenuPage(int menu_id) {
        const MenuPage& pg = pages_[menu_id];
        char buf[24];

        // Title bar
        display_.DrawRect(0, 0, 127, 9, true, true);
        display_.SetCursor(2, 1);
        display_.WriteString(pg.title, Font_6x8, false);

        for(int i = 0; i < pg.count; i++) {
            const MenuItem& it = pg.items[i];
            if(!it.label || it.label[0] == '\0') continue;

            int  y   = 10 + i * 11;
            bool sel = (i == cursor_);

            if(sel) display_.DrawRect(0, y - 1, 127, y + 8, true, true);

            display_.SetCursor(1, y);
            display_.WriteString(sel ? ">" : " ", Font_6x8, !sel);

            display_.SetCursor(9, y);
            display_.WriteString(it.label, Font_6x8, !sel);

            // Value display
            if(it.action == Action::TOGGLE) {
                const char* vs = vals_[it.val_idx] ? "[ON] " : "[OFF]";
                display_.SetCursor(84, y);
                display_.WriteString(vs, Font_6x8, !sel);
            } else if(it.action == Action::INT_RANGE) {
                if(it.val_idx == V_DISP_TO) {
                    static const char* kTo[4] = { "Off", "5s", "10s", "30s" };
                    int vi = vals_[it.val_idx];
                    if(vi < 0) vi = 0;
                    if(vi > 3) vi = 3;
                    display_.SetCursor(96, y);
                    display_.WriteString(kTo[vi], Font_6x8, !sel);
                } else {
                    snprintf(buf, 24, "%d", vals_[it.val_idx]);
                    int xpos = (vals_[it.val_idx] >= 100) ? 96 : 108;
                    display_.SetCursor(xpos, y);
                    display_.WriteString(buf, Font_6x8, !sel);
                }
                if(sel && editing_) {
                    display_.SetCursor(120, y);
                    display_.WriteString("*", Font_6x8, !sel);
                }
            }
        }

        // Status bar
        display_.SetCursor(0, 56);
        display_.WriteString(active_drum_ == 0 ? "KICK" : "SNARE", Font_6x8, true);
        snprintf(buf, 24, "BPM:%d", vals_[V_TEMPO]);
        display_.SetCursor(84, 56);
        display_.WriteString(buf, Font_6x8, true);
    }

    // ---- Zoom overlay ---------------------------------------

    void DrawZoomOverlay() {
        char buf[24];
        display_.SetCursor(2, 2);
        display_.WriteString(zoom_label_ ? zoom_label_ : "Param", Font_6x8, true);

        int pct = static_cast<int>(zoom_val_ * 100.f);
        snprintf(buf, 24, "%3d%%", pct);
        display_.SetCursor(24, 18);
        display_.WriteString(buf, Font_11x18, true);

        snprintf(buf, 24, "%.3f", zoom_val_);
        display_.SetCursor(44, 40);
        display_.WriteString(buf, Font_6x8, true);

        display_.DrawRect(0, 50, 127, 58, true, false);
        int fill = static_cast<int>(zoom_val_ * 127.f);
        if(fill > 0)
            display_.DrawRect(0, 50, fill, 58, true, true);
    }
};
