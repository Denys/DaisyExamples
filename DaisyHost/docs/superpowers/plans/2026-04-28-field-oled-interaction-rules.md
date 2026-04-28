# Field OLED Interaction Rules Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add Subharmoniq/Daisy Field OLED touch-zoom behavior with a 2.0 second hold window, clearer quantize wording, and a separate audio-dropout regression.

**Architecture:** Add a small reusable host-side transient display helper, then wire it into `apps::SubharmoniqCore` without changing firmware, DAW/VST3, routing, or Patch-board DSP behavior. `BuildDisplay()` will render a transient zoom first when active, otherwise the compact Subharmoniq status view.

**Tech Stack:** C++17, DaisyHost hosted app core interfaces, `DisplayModel`, GoogleTest/CTest, CMake/MSBuild on Windows.

---

## File Structure

- Create `include/daisyhost/FieldOledTransient.h`
  - Owns reusable transient OLED state: title, large value, detail lines, optional bar, hold/expiry timing.
- Create `src/FieldOledTransient.cpp`
  - Implements `Show`, `Tick`, `ReleaseHold`, `IsVisible`, and `ApplyToDisplay`.
- Modify `CMakeLists.txt`
  - Add `src/FieldOledTransient.cpp` to `daisyhost_core`.
  - Add `tests/test_field_oled_transient.cpp` to `DAISYHOST_UNIT_TEST_SOURCES`.
- Create `tests/test_field_oled_transient.cpp`
  - Pure unit tests for 2.0 second timeout, refresh behavior, held behavior, and display rendering.
- Modify `include/daisyhost/apps/SubharmoniqCore.h`
  - Add a `FieldOledTransient oledTransient_;` member.
  - Add private formatter/recording helpers.
- Modify `src/apps/SubharmoniqCore.cpp`
  - Add compact/detail formatter helpers.
  - Record transient zooms from parameter changes, menu key actions, SW/page navigation, and direct app wrapper actions.
  - Expire transient state from `TickUi`.
  - Render transient display before compact status.
- Modify `tests/test_subharmoniq_core.cpp`
  - Add red tests for compact `Quant 12-JI`, zoom text, B7/B5/A5-A8 confirmations, 2.0s expiry, and multi-note/multi-pulse audio.
- Modify `PROJECT_TRACKER.md`
  - Record the iteration, commands, results, LED/OLED assumptions, and manual validation caveats after implementation.

---

### Task 1: Add Red Tests For OLED Status, Transients, And Audio Dropout

**Files:**
- Modify: `tests/test_subharmoniq_core.cpp`

- [ ] **Step 1: Add helper functions near the existing `FindSection` helper**

Add this code inside the anonymous namespace after `FindSection`:

```cpp
bool DisplayContainsText(const daisyhost::DisplayModel& display,
                         const std::string&            needle)
{
    for(const auto& text : display.texts)
    {
        if(text.text.find(needle) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

float StereoEnergy(const std::array<float, 48000>& left,
                   const std::array<float, 48000>& right)
{
    float energy = 0.0f;
    for(std::size_t i = 0; i < left.size(); ++i)
    {
        EXPECT_TRUE(std::isfinite(left[i]));
        EXPECT_TRUE(std::isfinite(right[i]));
        energy += std::abs(left[i]) + std::abs(right[i]);
    }
    return energy;
}
```

- [ ] **Step 2: Add compact status wording test**

Add this test before `HostedWrapperExposesMenuBindingsAndRegistry`:

```cpp
TEST(SubharmoniqCoreTest, OledCompactStatusUsesClearQuantizeLabel)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);
    core.SetQuantizeMode(daisyhost::DaisySubharmoniqQuantizeMode::kTwelveJust);
    core.TickUi(0.0);

    const auto& display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "Quant 12-JI"));
    EXPECT_FALSE(DisplayContainsText(display, "Q 12-JI"));
}
```

- [ ] **Step 3: Add knob touch zoom test**

Add this test after the compact status test:

```cpp
TEST(SubharmoniqCoreTest, OledShowsKnobTouchZoomForTwoSecondsAfterLastChange)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);
    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisySubharmoniqPage::kVcf));

    core.SetControl("node0/control/cutoff", 0.50f);
    auto display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "Cutoff"));
    EXPECT_TRUE(DisplayContainsText(display, "K1"));
    EXPECT_FALSE(display.bars.empty());

    core.TickUi(1900.0);
    display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "Cutoff"));

    core.TickUi(200.0);
    display = core.GetDisplayModel();
    EXPECT_FALSE(DisplayContainsText(display, "K1 Cutoff"));
    EXPECT_TRUE(DisplayContainsText(display, "Subharmoniq VCF"));
}
```

- [ ] **Step 4: Add repeated movement refresh test**

Add this test after the knob touch zoom test:

```cpp
TEST(SubharmoniqCoreTest, OledTouchZoomRefreshesWhenParameterMovesAgain)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);
    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisySubharmoniqPage::kVcf));

    core.SetControl("node0/control/cutoff", 0.35f);
    core.TickUi(1500.0);
    core.SetControl("node0/control/cutoff", 0.70f);
    core.TickUi(1000.0);

    auto display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "K1 Cutoff"));

    core.TickUi(1100.0);
    display = core.GetDisplayModel();
    EXPECT_FALSE(DisplayContainsText(display, "K1 Cutoff"));
}
```

- [ ] **Step 5: Add B7 transport zoom test**

Add this test after the refresh test:

```cpp
TEST(SubharmoniqCoreTest, OledShowsB7PlayConfirmation)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    core.SetMenuItemValue("node0/menu/field_keys/b7", 1.0f);
    auto display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "B7 Play"));
    EXPECT_TRUE(DisplayContainsText(display, "Running"));
    EXPECT_TRUE(DisplayContainsText(display, "12-note Equal"));

    core.TickUi(2100.0);
    display = core.GetDisplayModel();
    EXPECT_FALSE(DisplayContainsText(display, "B7 Play"));
    EXPECT_TRUE(DisplayContainsText(display, "Play"));
}
```

- [ ] **Step 6: Add B5 quantize and A6 rhythm zoom tests**

Add these tests after the B7 test:

```cpp
TEST(SubharmoniqCoreTest, OledShowsB5QuantizeConfirmation)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    core.SetMenuItemValue("node0/menu/field_keys/b5", 1.0f);
    auto display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "B5 Quantize"));
    EXPECT_TRUE(DisplayContainsText(display, "8-note Equal"));
}

TEST(SubharmoniqCoreTest, OledShowsRhythmTargetConfirmation)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    core.SetMenuItemValue("node0/menu/field_keys/a6", 1.0f);
    auto display = core.GetDisplayModel();
    EXPECT_TRUE(DisplayContainsText(display, "A6 Rhythm 2"));
    EXPECT_TRUE(DisplayContainsText(display, "Both"));
    EXPECT_TRUE(DisplayContainsText(display, "R2 advances both"));
}
```

- [ ] **Step 7: Add multi-note audio regression**

Add this test near the existing audio tests:

```cpp
TEST(SubharmoniqCoreTest, MultipleMidiNotesAndFieldClockPulsesStayAudible)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);
    ASSERT_TRUE(core.SetParameterValue("output", 1.0f));
    ASSERT_TRUE(core.SetParameterValue("cutoff", 0.65f));

    std::array<float, 48000> left{};
    std::array<float, 48000> right{};
    float totalEnergy = 0.0f;

    for(int note = 0; note < 8; ++note)
    {
        left.fill(0.0f);
        right.fill(0.0f);
        core.HandleMidiEvent(0x90, static_cast<std::uint8_t>(48 + note), 100);
        core.ProcessAudio(left.data(), right.data(), left.size());
        totalEnergy += StereoEnergy(left, right);
        core.HandleMidiEvent(0x80, static_cast<std::uint8_t>(48 + note), 0);
    }

    core.SetMenuItemValue("node0/menu/field_keys/b7", 1.0f);
    for(int block = 0; block < 4; ++block)
    {
        left.fill(0.0f);
        right.fill(0.0f);
        core.HandleMidiEvent(0xF8, 0, 0);
        core.ProcessAudio(left.data(), right.data(), left.size());
        totalEnergy += StereoEnergy(left, right);
    }

    EXPECT_TRUE(core.IsPlaying());
    EXPECT_GT(core.GetTriggerCount(), 0u);
    EXPECT_GT(totalEnergy, 10.0f);
}
```

- [ ] **Step 8: Run red tests**

Run:

```powershell
cmake --build build --config Debug --target unit_tests
ctest --test-dir build -C Debug --output-on-failure -R "SubharmoniqCoreTest"
```

Expected:

- Build passes.
- The new OLED behavior tests fail because OLED still renders `Q 12-JI` and no transient display exists.
- The multi-note audio regression either passes, proving no current core fix is needed, or fails and becomes Task 4 input.
- Record the exact red result in working notes for later tracker update.

---

### Task 2: Add Reusable Field OLED Transient Helper

**Files:**
- Create: `include/daisyhost/FieldOledTransient.h`
- Create: `src/FieldOledTransient.cpp`
- Create: `tests/test_field_oled_transient.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Add the header**

Create `include/daisyhost/FieldOledTransient.h`:

```cpp
#pragma once

#include <string>
#include <vector>

#include "daisyhost/DisplayModel.h"

namespace daisyhost
{
struct FieldOledTransientRequest
{
    std::string              title;
    std::string              value;
    std::vector<std::string> detailLines;
    bool                     held          = false;
    bool                     hasBar        = false;
    float                    barNormalized = 0.0f;
};

class FieldOledTransient
{
  public:
    static constexpr double kDefaultHoldMs = 2000.0;

    void Show(const FieldOledTransientRequest& request,
              double                           holdMs = kDefaultHoldMs);
    void Tick(double deltaMs);
    void SetHeld(bool held);
    bool IsVisible() const;
    void Clear();
    void ApplyToDisplay(DisplayModel& display) const;

  private:
    std::string              title_;
    std::string              value_;
    std::vector<std::string> detailLines_;
    double                   remainingMs_   = 0.0;
    bool                     visible_       = false;
    bool                     held_          = false;
    bool                     hasBar_        = false;
    float                    barNormalized_ = 0.0f;
};
} // namespace daisyhost
```

- [ ] **Step 2: Add the implementation**

Create `src/FieldOledTransient.cpp`:

```cpp
#include "daisyhost/FieldOledTransient.h"

#include <algorithm>

namespace daisyhost
{
namespace
{
float Clamp01(float value)
{
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}
} // namespace

void FieldOledTransient::Show(const FieldOledTransientRequest& request,
                              double                           holdMs)
{
    title_         = request.title;
    value_         = request.value;
    detailLines_   = request.detailLines;
    remainingMs_   = holdMs > 0.0 ? holdMs : kDefaultHoldMs;
    visible_       = true;
    held_          = request.held;
    hasBar_        = request.hasBar;
    barNormalized_ = Clamp01(request.barNormalized);
}

void FieldOledTransient::Tick(double deltaMs)
{
    if(!visible_ || held_)
    {
        return;
    }
    remainingMs_ -= std::max(0.0, deltaMs);
    if(remainingMs_ <= 0.0)
    {
        Clear();
    }
}

void FieldOledTransient::SetHeld(bool held)
{
    held_ = held;
    if(visible_ && !held_)
    {
        remainingMs_ = kDefaultHoldMs;
    }
}

bool FieldOledTransient::IsVisible() const
{
    return visible_;
}

void FieldOledTransient::Clear()
{
    title_.clear();
    value_.clear();
    detailLines_.clear();
    remainingMs_   = 0.0;
    visible_       = false;
    held_          = false;
    hasBar_        = false;
    barNormalized_ = 0.0f;
}

void FieldOledTransient::ApplyToDisplay(DisplayModel& display) const
{
    if(!visible_)
    {
        return;
    }
    display.texts.clear();
    display.bars.clear();
    display.mode = DisplayMode::kStatus;
    display.texts.push_back({0, 0, title_, true});
    display.texts.push_back({0, 16, value_, false});
    int y = hasBar_ ? 38 : 32;
    if(hasBar_)
    {
        display.bars.push_back({0, 32, 118, 6, barNormalized_});
    }
    for(const auto& line : detailLines_)
    {
        display.texts.push_back({0, y, line, false});
        y += 10;
        if(y > 54)
        {
            break;
        }
    }
}
} // namespace daisyhost
```

- [ ] **Step 3: Add pure unit tests**

Create `tests/test_field_oled_transient.cpp`:

```cpp
#include <gtest/gtest.h>

#include "daisyhost/FieldOledTransient.h"

namespace
{
bool HasText(const daisyhost::DisplayModel& display, const std::string& text)
{
    for(const auto& item : display.texts)
    {
        if(item.text.find(text) != std::string::npos)
        {
            return true;
        }
    }
    return false;
}

TEST(FieldOledTransientTest, ShowsAndExpiresAfterTwoSeconds)
{
    daisyhost::FieldOledTransient transient;
    transient.Show({"K8 Cutoff", "1.84 kHz", {"returns after 2.0s"}, false, true, 0.5f});

    daisyhost::DisplayModel display;
    transient.ApplyToDisplay(display);
    EXPECT_TRUE(HasText(display, "K8 Cutoff"));
    EXPECT_TRUE(HasText(display, "1.84 kHz"));
    ASSERT_EQ(display.bars.size(), 1u);
    EXPECT_FLOAT_EQ(display.bars[0].normalized, 0.5f);

    transient.Tick(1999.0);
    EXPECT_TRUE(transient.IsVisible());
    transient.Tick(2.0);
    EXPECT_FALSE(transient.IsVisible());
}

TEST(FieldOledTransientTest, RefreshingShowRestartsTimeout)
{
    daisyhost::FieldOledTransient transient;
    transient.Show({"K8 Cutoff", "1.0 kHz", {}, false, false, 0.0f});
    transient.Tick(1500.0);
    transient.Show({"K8 Cutoff", "2.0 kHz", {}, false, false, 0.0f});
    transient.Tick(1000.0);
    EXPECT_TRUE(transient.IsVisible());
    transient.Tick(1001.0);
    EXPECT_FALSE(transient.IsVisible());
}

TEST(FieldOledTransientTest, HeldTransientStaysVisibleUntilReleased)
{
    daisyhost::FieldOledTransient transient;
    transient.Show({"B7 Play", "Running", {}, true, false, 0.0f});
    transient.Tick(5000.0);
    EXPECT_TRUE(transient.IsVisible());
    transient.SetHeld(false);
    transient.Tick(1999.0);
    EXPECT_TRUE(transient.IsVisible());
    transient.Tick(2.0);
    EXPECT_FALSE(transient.IsVisible());
}
} // namespace
```

- [ ] **Step 4: Wire new files into CMake**

Modify `CMakeLists.txt`:

```cmake
add_library(daisyhost_core STATIC
    src/AppRegistry.cpp
    src/BoardControlMapping.cpp
    src/BoardProfile.cpp
    src/ComputerKeyboardMidi.cpp
    src/DaisyBraidsCore.cpp
    src/DaisyCloudSeedCore.cpp
    src/DaisyHarmoniqsCore.cpp
    src/DaisyPolyOscCore.cpp
    src/DaisySubharmoniqCore.cpp
    src/DaisyVASynthCore.cpp
    src/EffectiveHostStateSnapshot.cpp
    src/FieldOledTransient.cpp
```

Add the test source in the `DAISYHOST_UNIT_TEST_SOURCES` list near related host tests:

```cmake
    tests/test_field_oled_transient.cpp
```

- [ ] **Step 5: Run the pure helper tests**

Run:

```powershell
cmake --build build --config Debug --target unit_tests
ctest --test-dir build -C Debug --output-on-failure -R "FieldOledTransientTest"
```

Expected:

- `FieldOledTransientTest` passes.
- Subharmoniq OLED tests from Task 1 still fail until Task 3.

---

### Task 3: Wire Subharmoniq OLED Transient Behavior

**Files:**
- Modify: `include/daisyhost/apps/SubharmoniqCore.h`
- Modify: `src/apps/SubharmoniqCore.cpp`

- [ ] **Step 1: Include helper and add private recording state**

Modify `include/daisyhost/apps/SubharmoniqCore.h` includes:

```cpp
#include "daisyhost/DaisySubharmoniqCore.h"
#include "daisyhost/FieldOledTransient.h"
#include "daisyhost/HostedAppCore.h"
```

Add private helpers:

```cpp
    void RecordParameterZoom(const std::string& parameterId,
                             float              normalizedValue);
    void RecordFieldKeyZoom(std::size_t zeroBasedIndex);
    void RecordPageZoom();
    std::string FormatParameterValue(const std::string& parameterId,
                                     float              normalizedValue) const;
    std::string FieldKnobLabelForParameter(const std::string& parameterId) const;
```

Add private members:

```cpp
    FieldOledTransient                       oledTransient_;
```

- [ ] **Step 2: Add formatter helpers in `SubharmoniqCore.cpp`**

Add these helper functions in the anonymous namespace after `FormatQuantize`:

```cpp
std::string FormatQuantizeDetail(DaisySubharmoniqQuantizeMode mode)
{
    switch(mode)
    {
        case DaisySubharmoniqQuantizeMode::kOff: return "Off";
        case DaisySubharmoniqQuantizeMode::kEightEqual: return "8-note Equal";
        case DaisySubharmoniqQuantizeMode::kTwelveJust: return "12-note Just";
        case DaisySubharmoniqQuantizeMode::kEightJust: return "8-note Just";
        default: return "12-note Equal";
    }
}

std::string FormatRhythmTargetDetail(DaisySubharmoniqRhythmTarget target)
{
    switch(target)
    {
        case DaisySubharmoniqRhythmTarget::kSeq1: return "Seq1";
        case DaisySubharmoniqRhythmTarget::kSeq2: return "Seq2";
        case DaisySubharmoniqRhythmTarget::kBoth: return "Both";
        default: return "Off";
    }
}

std::string FormatFrequencyFromNormalized(float normalized)
{
    const float clamped = Clamp01(normalized);
    const float hz = 20.0f * std::pow(1000.0f, clamped);
    char buffer[32];
    if(hz >= 1000.0f)
    {
        std::snprintf(buffer, sizeof(buffer), "%.2f kHz", hz / 1000.0f);
    }
    else
    {
        std::snprintf(buffer, sizeof(buffer), "%.0f Hz", hz);
    }
    return std::string(buffer);
}

std::string FieldKeyName(std::size_t zeroBasedIndex)
{
    const char row = zeroBasedIndex < 8 ? 'A' : 'B';
    const std::size_t number = zeroBasedIndex < 8 ? zeroBasedIndex + 1
                                                  : zeroBasedIndex - 7;
    return std::string(1, row) + std::to_string(number);
}
```

- [ ] **Step 3: Tick transient state from UI ticks**

Change `TickUi`:

```cpp
void SubharmoniqCore::TickUi(double deltaMs)
{
    oledTransient_.Tick(deltaMs);
    BuildDisplay();
}
```

- [ ] **Step 4: Record parameter zoom from `SetControl`**

Change the parameter branch inside `SetControl`:

```cpp
    const std::string suffix = StripControlId(controlId);
    if(!suffix.empty() && sharedCore_.SetParameterValue(suffix, normalizedValue))
    {
        RecordParameterZoom(suffix, normalizedValue);
        RefreshSnapshots();
    }
```

- [ ] **Step 5: Record parameter zoom from `SetParameterValue`**

Change `SetParameterValue(const std::string&, float)`:

```cpp
    const std::string stripped = StripParameterId(parameterId);
    const bool changed = sharedCore_.SetParameterValue(stripped, normalizedValue);
    if(changed)
    {
        RecordParameterZoom(stripped, normalizedValue);
        RefreshSnapshots();
    }
    return changed;
```

Change `SetParameterValue(const char*, float)`:

```cpp
bool SubharmoniqCore::SetParameterValue(const char* parameterId,
                                        float       normalizedValue)
{
    if(parameterId == nullptr)
    {
        return false;
    }
    const bool changed = sharedCore_.SetParameterValue(parameterId, normalizedValue);
    if(changed)
    {
        RecordParameterZoom(parameterId, normalizedValue);
        RefreshSnapshots();
    }
    return changed;
}
```

- [ ] **Step 6: Record page zoom from SW/menu page rotation**

Change `MenuRotate`:

```cpp
void SubharmoniqCore::MenuRotate(int delta)
{
    const auto next = static_cast<int>(sharedCore_.GetActivePage()) + delta;
    const int wrapped = (next + kSubharmoniqPageCount) % kSubharmoniqPageCount;
    sharedCore_.SetActivePage(static_cast<DaisySubharmoniqPage>(wrapped));
    RecordPageZoom();
    RefreshSnapshots();
}
```

In `SetMenuItemValue`, after setting the pages item, add:

```cpp
        RecordPageZoom();
```

- [ ] **Step 7: Record Field key zooms**

At the end of `SetMenuItemValue`, before `RefreshSnapshots();`, keep the existing refresh call but ensure handled Field keys call `RecordFieldKeyZoom(i)` immediately after `TriggerFieldKeyAction(i)`:

```cpp
                handled = TriggerFieldKeyAction(i);
                if(handled)
                {
                    RecordFieldKeyZoom(i);
                }
                break;
```

Do not add a Subharmoniq test-only key-held hook. The reusable helper owns held-state mechanics, and production key actions create a 2.0 second momentary confirmation from the existing app/menu action path.

- [ ] **Step 8: Add Subharmoniq recording helpers**

Add these methods before `BuildDisplay()`:

```cpp
void SubharmoniqCore::RecordParameterZoom(const std::string& parameterId,
                                          float              normalizedValue)
{
    const std::string knobLabel = FieldKnobLabelForParameter(parameterId);
    const std::string title = (knobLabel.empty() ? std::string("Param") : knobLabel)
                              + " " + FormatParameterValue(parameterId, normalizedValue);
    oledTransient_.Show({
        knobLabel.empty() ? parameterId : knobLabel,
        FormatParameterValue(parameterId, normalizedValue),
        {"returns after 2.0s"},
        false,
        true,
        normalizedValue,
    });
}

void SubharmoniqCore::RecordPageZoom()
{
    const auto binding = sharedCore_.GetActivePageBinding();
    oledTransient_.Show({
        "Page",
        binding.pageLabel,
        {"SW1<- SW2->"},
        false,
        false,
        0.0f,
    });
}

void SubharmoniqCore::RecordFieldKeyZoom(std::size_t zeroBasedIndex)
{
    const std::string keyName = FieldKeyName(zeroBasedIndex);
    if(zeroBasedIndex == 12)
    {
        oledTransient_.Show({keyName + " Quantize",
                             FormatQuantizeDetail(sharedCore_.GetQuantizeMode()),
                             {"Quant " + FormatQuantize(sharedCore_.GetQuantizeMode())},
                             false,
                             false,
                             0.0f});
        return;
    }
    if(zeroBasedIndex == 13)
    {
        oledTransient_.Show({keyName + " Seq Oct",
                             std::to_string(sharedCore_.GetSeqOctaveRange())
                                 + " octave range",
                             {},
                             false,
                             false,
                             0.0f});
        return;
    }
    if(zeroBasedIndex == 14)
    {
        oledTransient_.Show({keyName + (sharedCore_.IsPlaying() ? " Play" : " Stop"),
                             sharedCore_.IsPlaying() ? "Running" : "Stopped",
                             {"Step S1:"
                                  + std::to_string(sharedCore_.GetSequencerStepIndex(0) + 1)
                                  + " S2:"
                                  + std::to_string(sharedCore_.GetSequencerStepIndex(1) + 1),
                              "Quant: "
                                  + FormatQuantizeDetail(sharedCore_.GetQuantizeMode())},
                             false,
                             false,
                             0.0f});
        return;
    }
    if(zeroBasedIndex == 15)
    {
        oledTransient_.Show({keyName + " Reset",
                             "Reset",
                             {"Steps S1:1 S2:1"},
                             false,
                             false,
                             0.0f});
        return;
    }
    if(zeroBasedIndex >= 4 && zeroBasedIndex < 8)
    {
        const std::size_t rhythm = zeroBasedIndex - 4;
        const auto target = sharedCore_.GetRhythmTarget(rhythm);
        const std::string value = FormatRhythmTargetDetail(target);
        const std::string detail = target == DaisySubharmoniqRhythmTarget::kBoth
                                       ? "R" + std::to_string(rhythm + 1)
                                             + " advances both"
                                       : "R" + std::to_string(rhythm + 1)
                                             + " routes to " + value;
        oledTransient_.Show({keyName + " Rhythm " + std::to_string(rhythm + 1),
                             value,
                             {detail},
                             false,
                             false,
                             0.0f});
        return;
    }

    oledTransient_.Show({keyName + " " + FieldKeyDetailLabel(zeroBasedIndex),
                         "Selected",
                         {"Seq/Rhy"},
                         false,
                         false,
                         0.0f});
}

std::string SubharmoniqCore::FormatParameterValue(
    const std::string& parameterId,
    float              normalizedValue) const
{
    if(parameterId == "cutoff" || parameterId == "cutoff_cv")
    {
        return FormatFrequencyFromNormalized(normalizedValue);
    }
    if(parameterId == "quantize_mode")
    {
        return FormatQuantizeDetail(sharedCore_.GetQuantizeMode());
    }
    if(parameterId == "seq_oct_range")
    {
        return std::to_string(sharedCore_.GetSeqOctaveRange()) + " octave range";
    }
    return FormatPercent(normalizedValue);
}

std::string SubharmoniqCore::FieldKnobLabelForParameter(
    const std::string& parameterId) const
{
    const auto binding = sharedCore_.GetActivePageBinding();
    for(std::size_t i = 0; i < binding.parameterIds.size(); ++i)
    {
        if(binding.parameterIds[i] == parameterId)
        {
            return "K" + std::to_string(i + 1) + " " + binding.parameterLabels[i];
        }
    }
    return "";
}
```

- [ ] **Step 9: Render transient before compact status and fix compact quantize label**

At the end of the `BuildDisplay()` setup, after incrementing revision and before adding compact status lines, insert:

```cpp
    if(oledTransient_.IsVisible())
    {
        oledTransient_.ApplyToDisplay(display_);
        return;
    }
```

Change the compact quantize text from:

```cpp
                              "Q " + FormatQuantize(sharedCore_.GetQuantizeMode()),
```

to:

```cpp
                              "Quant " + FormatQuantize(sharedCore_.GetQuantizeMode()),
```

- [ ] **Step 10: Run Subharmoniq OLED tests**

Run:

```powershell
cmake --build build --config Debug --target unit_tests
ctest --test-dir build -C Debug --output-on-failure -R "(FieldOledTransientTest|SubharmoniqCoreTest)"
```

Expected:

- `FieldOledTransientTest` passes.
- OLED-related `SubharmoniqCoreTest` tests pass.
- If `MultipleMidiNotesAndFieldClockPulsesStayAudible` fails, stop and execute Task 4 before claiming green.

---

### Task 4: Diagnose And Fix Multi-Note Audio Dropout If The Regression Fails

**Files:**
- Modify if needed: `src/DaisySubharmoniqCore.cpp`
- Modify if needed: `src/apps/SubharmoniqCore.cpp`
- Test: `tests/test_subharmoniq_core.cpp`

- [ ] **Step 1: Run the isolated audio regression**

Run:

```powershell
ctest --test-dir build -C Debug --output-on-failure -R "MultipleMidiNotesAndFieldClockPulsesStayAudible"
```

Expected:

- If it passes, do not change DSP/runtime code for this task.
- If it fails, use the test output to identify whether the silence occurs after MIDI note events, B7 play toggle, MIDI clock pulses, or parameter state.

- [ ] **Step 2: Add diagnostic assertions before changing code if it fails**

If the test fails, temporarily extend the failing test with these assertions at the point where energy collapses:

```cpp
EXPECT_TRUE(core.IsPlaying());
EXPECT_GT(core.GetTriggerCount(), 0u);
EXPECT_GE(core.GetSequencerStepIndex(0), 0);
EXPECT_GE(core.GetSequencerStepIndex(1), 0);
```

Run the isolated test again and capture the failing condition.

- [ ] **Step 3: Apply the smallest fix matching the failing condition**

Use these exact fix rules:

- If `panic` or `hardMuted` remains active after note/play events, clear hard mute only on explicit note-on, gate rising edge, and play start.
- If envelope state reaches zero and never retriggers on MIDI note-on or clock pulse, retrigger envelopes in the relevant event path.
- If output is zero because `output`, oscillator levels, or cutoff effective state collapsed to zero through Field movement, preserve the existing Field cutoff safety floor and do not add a second safety layer.

Expected code shape for hard-mute/envelope fix in `src/DaisySubharmoniqCore.cpp`:

```cpp
if(statusNibble == 0x90 && data2 > 0)
{
    impl_->currentMidiNote = static_cast<int>(data1);
    impl_->currentVelocity = static_cast<int>(data2);
    impl_->hardMuted = false;
    impl_->TriggerEnvelopes();
}
```

Expected code shape for play start:

```cpp
if(actionId == "play_toggle")
{
    impl_->playing = !impl_->playing;
    if(impl_->playing)
    {
        impl_->internalClockSamplesUntilPulse = 0.0f;
        impl_->hardMuted = false;
        impl_->TriggerEnvelopes();
    }
    return true;
}
```

- [ ] **Step 4: Rerun audio regression**

Run:

```powershell
ctest --test-dir build -C Debug --output-on-failure -R "MultipleMidiNotesAndFieldClockPulsesStayAudible"
```

Expected:

- Test passes.
- Record the root cause in `PROJECT_TRACKER.md`.

---

### Task 5: Verification And Tracker Closeout

**Files:**
- Modify: `PROJECT_TRACKER.md`

- [ ] **Step 1: Run targeted Debug build**

Run:

```powershell
cmake --build build --config Debug --target unit_tests
```

Expected:

- Build passes.

- [ ] **Step 2: Run targeted Debug tests**

Run:

```powershell
ctest --test-dir build -C Debug --output-on-failure -R "(FieldOledTransientTest|SubharmoniqCoreTest|BoardControlMappingTest|RenderRuntimeTest|HostModulationTest)"
```

Expected:

- All selected tests pass.
- The run includes the new OLED transient tests and the multi-note audio regression.

- [ ] **Step 3: Build Debug standalone**

Run:

```powershell
cmake --build build --config Debug --target DaisyHostPatch_Standalone
```

Expected:

- `build\DaisyHostPatch_artefacts\Debug\Standalone\DaisyHost Patch.exe` builds.

- [ ] **Step 4: Run Field/Subharmoniq standalone smoke**

Run:

```powershell
py -3 tests\run_smoke.py --mode standalone --build-dir build --source-dir . --config Debug --board daisy_field --app subharmoniq --timeout-seconds 60
```

Expected:

- Smoke passes for `board=daisy_field`, `app=subharmoniq`.

- [ ] **Step 5: Run whitespace check**

Run:

```powershell
git diff --check -- include\daisyhost\FieldOledTransient.h src\FieldOledTransient.cpp include\daisyhost\apps\SubharmoniqCore.h src\apps\SubharmoniqCore.cpp tests\test_field_oled_transient.cpp tests\test_subharmoniq_core.cpp CMakeLists.txt PROJECT_TRACKER.md
```

Expected:

- Passes with no whitespace errors. CRLF/LF warnings are acceptable if no diff-check errors are reported.

- [ ] **Step 6: Update `PROJECT_TRACKER.md`**

Add a new latest implementation iteration with:

- date `2026-04-28`
- slice `TF8/TF9 Field OLED transient display and Subharmoniq audio regression`
- manager-readable result:
  - compact OLED now uses `Quant 12-JI`
  - touched controls show 2.0 second zoom
  - B7/B5/A5-A8 show detail confirmations
  - multi-note/multi-pulse Subharmoniq audio regression result
- exact commands and pass/fail results
- root cause if the audio regression required a code fix
- caveat that manual visual/audio and real hardware validation were not performed unless separately done

---

## Self-Review

Spec coverage:

- Compact `Q 12-JI` clarification is covered in Task 1 and Task 3.
- 2.0 second touch zoom is covered in Task 1, Task 2, and Task 3.
- Held button behavior is covered in Task 2's pure helper tests. Production Subharmoniq key actions currently show a 2.0 second confirmation from the existing menu/action path; real held-key wiring remains a processor/input integration follow-up.
- B7/B5/A5-A8 detail wording is covered in Task 1 and Task 3.
- Audio stopping after a couple notes is covered as a separate regression in Task 1 and Task 4.
- Verification and tracker update are covered in Task 5.

Placeholder scan:

- No `TBD`, `TODO`, `implement later`, or vague test instructions are intentionally present.

Type consistency:

- `FieldOledTransientRequest`, `FieldOledTransient`, `RecordParameterZoom`, `RecordFieldKeyZoom`, and formatter names are introduced before use.
- The plan uses existing `DisplayModel`, `DisplayText`, `DisplayBar`, and `apps::SubharmoniqCore` surfaces.
