# Subharmoniq Field Control Upgrade Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Upgrade DaisyHost Subharmoniq Field controls to a four-page, performance-first model with globally stable A/B key actions.

**Architecture:** Keep the change host-side and centered on the portable Subharmoniq core/page bindings plus HostedApp wrapper display/menu behavior. The Field board mapping already consumes app patch bindings and menu items, so the implementation should preserve the existing `field_keys` menu seam and only change page names, page count, default page, bindings, display status, and tests.

**Tech Stack:** C++17, CMake, GTest, DaisyHost hosted app interfaces, Daisy Field board-control mapping, RenderRuntime smoke harness.

---

## Spec And Current Files

Approved spec:

- `docs/superpowers/specs/2026-04-28-subharmoniq-field-control-upgrade-design.md`

Files to modify:

- `include/daisyhost/DaisySubharmoniqCore.h`: reduce public page enum to the approved pages and keep page binding shape stable.
- `src/DaisySubharmoniqCore.cpp`: default active page, reset/restore page clamp, active page binding IDs/labels.
- `src/apps/SubharmoniqCore.cpp`: page formatting, page menu normalization, menu sections, display/RSP text, Field key post-action page focus.
- `tests/test_subharmoniq_core.cpp`: red/green coverage for page bindings, stable key actions, LEDs, display status.
- `tests/test_board_control_mapping.cpp`: red/green coverage that Field keys remain menu-backed and stable after page changes.
- `tests/test_render_runtime.cpp`: only if existing runtime coverage does not catch the new stable-key page behavior.
- `PROJECT_TRACKER.md`: iteration evidence after verification.
- `SKILL_PLAYBOOK.md`: update only if the skill use is material to local tracking expectations.

Files not to modify:

- firmware sources
- unrelated DSP apps
- Patch board layout/behavior
- DAW/VST behavior

Do not commit automatically in this dirty workspace unless the user explicitly asks. Preserve unrelated edits.

---

### Task 1: Write Red Tests For The Four-Page Subharmoniq Contract

**Files:**

- Modify: `tests/test_subharmoniq_core.cpp`

- [ ] **Step 1: Add page-binding tests**

Add a test near `DefaultsExposeSixSourcesPagesAndQuantization`:

```cpp
TEST(SubharmoniqCoreTest, FieldPerformancePagesMatchApprovedHybridMap)
{
    daisyhost::DaisySubharmoniqCore core;
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    auto binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.page, daisyhost::DaisySubharmoniqPage::kSeqRhythm);
    EXPECT_EQ(binding.pageLabel, "Seq/Rhy");
    EXPECT_EQ(binding.parameterIds[0], "seq1_step1");
    EXPECT_EQ(binding.parameterIds[3], "seq1_step4");
    EXPECT_EQ(binding.parameterIds[4], "seq2_step1");
    EXPECT_EQ(binding.parameterIds[7], "seq2_step4");

    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisySubharmoniqPage::kVco));
    binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.pageLabel, "VCO");
    EXPECT_EQ(binding.parameterIds[0], "vco1_pitch");
    EXPECT_EQ(binding.parameterIds[1], "vco2_pitch");
    EXPECT_EQ(binding.parameterIds[2], "vco1_sub1_div");
    EXPECT_EQ(binding.parameterIds[3], "vco1_sub2_div");
    EXPECT_EQ(binding.parameterIds[4], "vco2_sub1_div");
    EXPECT_EQ(binding.parameterIds[5], "vco2_sub2_div");
    EXPECT_EQ(binding.parameterLabels[6], "Quantize");
    EXPECT_EQ(binding.parameterLabels[7], "Seq Oct");

    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisySubharmoniqPage::kVcf));
    binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.pageLabel, "VCF");
    EXPECT_EQ(binding.parameterIds[0], "cutoff");
    EXPECT_EQ(binding.parameterIds[1], "resonance");
    EXPECT_EQ(binding.parameterIds[2], "vcf_env_amt");
    EXPECT_EQ(binding.parameterIds[3], "vcf_attack");
    EXPECT_EQ(binding.parameterIds[4], "vcf_decay");
    EXPECT_EQ(binding.parameterIds[5], "tempo");
    EXPECT_EQ(binding.parameterIds[6], "drive");
    EXPECT_EQ(binding.parameterIds[7], "output");

    ASSERT_TRUE(core.SetActivePage(daisyhost::DaisySubharmoniqPage::kVcaMix));
    binding = core.GetActivePageBinding();
    EXPECT_EQ(binding.pageLabel, "VCA/Mix");
    EXPECT_EQ(binding.parameterIds[0], "vco1_level");
    EXPECT_EQ(binding.parameterIds[1], "vco1_sub1_level");
    EXPECT_EQ(binding.parameterIds[2], "vco1_sub2_level");
    EXPECT_EQ(binding.parameterIds[3], "vco2_level");
    EXPECT_EQ(binding.parameterIds[4], "vco2_sub1_level");
    EXPECT_EQ(binding.parameterIds[5], "vco2_sub2_level");
    EXPECT_EQ(binding.parameterIds[6], "vca_decay");
    EXPECT_EQ(binding.parameterIds[7], "output");
}
```

- [ ] **Step 2: Add hosted-wrapper page cycling and display tests**

Add this test after `HostedWrapperExposesMenuBindingsAndRegistry`:

```cpp
TEST(SubharmoniqCoreTest, HostedWrapperCyclesOnlyApprovedFieldPages)
{
    daisyhost::apps::SubharmoniqCore core("node0");
    core.Prepare(48000.0, 48);
    core.ResetToDefaultState(0);

    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kSeqRhythm);
    EXPECT_EQ(core.GetPatchBindings().knobDetailLabels[0], "S1 Step1");

    core.MenuRotate(1);
    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kVco);
    EXPECT_EQ(core.GetPatchBindings().knobDetailLabels[0], "VCO 1");

    core.MenuRotate(1);
    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kVcf);
    EXPECT_EQ(core.GetPatchBindings().knobDetailLabels[0], "Cutoff");

    core.MenuRotate(1);
    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kVcaMix);
    EXPECT_EQ(core.GetPatchBindings().knobDetailLabels[0], "VCO1 Lvl");

    core.MenuRotate(1);
    EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kSeqRhythm);

    const auto& display = core.GetDisplayModel();
    bool sawRhythmStatus = false;
    bool sawAssignStatus = false;
    for(const auto& text : display.texts)
    {
        sawRhythmStatus = sawRhythmStatus || text.text.find("R1/") != std::string::npos;
        sawAssignStatus = sawAssignStatus || text.text.find("Assign") != std::string::npos;
    }
    EXPECT_TRUE(sawRhythmStatus);
    EXPECT_TRUE(sawAssignStatus);
}
```

- [ ] **Step 3: Update existing tests to expect the new default page**

Change existing expectations:

```cpp
EXPECT_EQ(binding.page, daisyhost::DaisySubharmoniqPage::kSeqRhythm);
EXPECT_EQ(binding.pageLabel, "Seq/Rhy");
EXPECT_EQ(binding.parameterLabels[0], "S1 Step1");
EXPECT_EQ(binding.parameterLabels[7], "S2 Step4");
```

and:

```cpp
EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kSeqRhythm);
```

- [ ] **Step 4: Run red test build**

Run:

```powershell
cmake --build build --config Debug --target unit_tests
```

Expected: fail to compile because `DaisySubharmoniqPage::kSeqRhythm`, `kVco`, `kVcf`, and `kVcaMix` do not exist yet.

---

### Task 2: Implement The Four-Page Core Binding

**Files:**

- Modify: `include/daisyhost/DaisySubharmoniqCore.h`
- Modify: `src/DaisySubharmoniqCore.cpp`

- [ ] **Step 1: Replace the page enum**

In `include/daisyhost/DaisySubharmoniqCore.h`, replace the current `DaisySubharmoniqPage` enum with:

```cpp
enum class DaisySubharmoniqPage
{
    kSeqRhythm = 0,
    kVco,
    kVcf,
    kVcaMix,
};
```

Set `DaisySubharmoniqPageBinding::page` default to:

```cpp
DaisySubharmoniqPage page = DaisySubharmoniqPage::kSeqRhythm;
```

- [ ] **Step 2: Update default/reset page**

In `src/DaisySubharmoniqCore.cpp`, set `Impl::activePage` and `ResetToDefaultState` to `kSeqRhythm`:

```cpp
DaisySubharmoniqPage activePage = DaisySubharmoniqPage::kSeqRhythm;
```

and:

```cpp
impl_->activePage = DaisySubharmoniqPage::kSeqRhythm;
```

- [ ] **Step 3: Update restore and page validation bounds**

Replace the restore clamp:

```cpp
ClampInt(static_cast<int>(std::round(page->second)), 0, 3)
```

Replace the `SetActivePage` max check with:

```cpp
if(index < 0 || index > static_cast<int>(DaisySubharmoniqPage::kVcaMix))
```

- [ ] **Step 4: Replace `GetActivePageBinding` switch cases**

Use these exact bindings:

```cpp
case DaisySubharmoniqPage::kSeqRhythm:
    set("Seq/Rhy",
        {"seq1_step1", "seq1_step2", "seq1_step3", "seq1_step4",
         "seq2_step1", "seq2_step2", "seq2_step3", "seq2_step4"},
        {"S1 Step1", "S1 Step2", "S1 Step3", "S1 Step4",
         "S2 Step1", "S2 Step2", "S2 Step3", "S2 Step4"});
    break;
case DaisySubharmoniqPage::kVco:
    set("VCO",
        {"vco1_pitch", "vco2_pitch", "vco1_sub1_div", "vco1_sub2_div",
         "vco2_sub1_div", "vco2_sub2_div", "", ""},
        {"VCO 1", "VCO 2", "VCO1 Sub1", "VCO1 Sub2",
         "VCO2 Sub1", "VCO2 Sub2", "Quantize", "Seq Oct"});
    break;
case DaisySubharmoniqPage::kVcf:
    set("VCF",
        {"cutoff", "resonance", "vcf_env_amt", "vcf_attack",
         "vcf_decay", "tempo", "drive", "output"},
        {"Cutoff", "Res", "VCF Env", "VCF Attack",
         "VCF Decay", "Tempo", "Drive", "Output"});
    break;
case DaisySubharmoniqPage::kVcaMix:
    set("VCA/Mix",
        {"vco1_level", "vco1_sub1_level", "vco1_sub2_level", "vco2_level",
         "vco2_sub1_level", "vco2_sub2_level", "vca_decay", "output"},
        {"VCO1 Lvl", "V1S1 Lvl", "V1S2 Lvl", "VCO2 Lvl",
         "V2S1 Lvl", "V2S2 Lvl", "VCA Decay", "Output"});
    break;
```

- [ ] **Step 5: Run targeted compile**

Run:

```powershell
cmake --build build --config Debug --target unit_tests
```

Expected: compile may still fail in `SubharmoniqCore.cpp` where old enum values and page counts are referenced.

---

### Task 3: Update Hosted Wrapper Page Menu, Display, And Stable Key Focus

**Files:**

- Modify: `src/apps/SubharmoniqCore.cpp`

- [ ] **Step 1: Add page constants and format helpers**

Near the anonymous namespace helpers, add:

```cpp
constexpr int kSubharmoniqPageCount = 4;

const char* FormatRhythmTarget(DaisySubharmoniqRhythmTarget target)
{
    switch(target)
    {
        case DaisySubharmoniqRhythmTarget::kSeq1: return "S1";
        case DaisySubharmoniqRhythmTarget::kSeq2: return "S2";
        case DaisySubharmoniqRhythmTarget::kBoth: return "Both";
        default: return "Off";
    }
}
```

Replace `FormatPageLabel` cases with:

```cpp
case DaisySubharmoniqPage::kVco: return "VCO";
case DaisySubharmoniqPage::kVcf: return "VCF";
case DaisySubharmoniqPage::kVcaMix: return "VCA/Mix";
default: return "Seq/Rhy";
```

- [ ] **Step 2: Update menu page rotation and page normalized values**

Replace the `MenuRotate` wrap logic with:

```cpp
const auto next = static_cast<int>(sharedCore_.GetActivePage()) + delta;
int wrapped = next % kSubharmoniqPageCount;
if(wrapped < 0)
{
    wrapped += kSubharmoniqPageCount;
}
sharedCore_.SetActivePage(static_cast<DaisySubharmoniqPage>(wrapped));
```

In `BuildMenuModel`, replace `/ 8.0f` with:

```cpp
/ static_cast<float>(kSubharmoniqPageCount - 1)
```

In `SetMenuItemValue`, replace page decoding with:

```cpp
const int rawPage = static_cast<int>(
    std::round(Clamp01(normalizedValue)
               * static_cast<float>(kSubharmoniqPageCount - 1)));
const int page = rawPage < 0 ? 0 : (rawPage >= kSubharmoniqPageCount
                                       ? kSubharmoniqPageCount - 1
                                       : rawPage);
sharedCore_.SetActivePage(static_cast<DaisySubharmoniqPage>(page));
```

- [ ] **Step 3: Replace page menu sections**

Replace the `pageSections` array with:

```cpp
const std::array<std::pair<const char*, const char*>, 4> pageSections = {{
    {"seq_rhythm", "Seq"},
    {"vco", "Voice"},
    {"vcf", "Filter"},
    {"vca_mix", "Mix"},
}};
```

Keep the generic parameter iteration unchanged so the menu remains parameter-backed.

- [ ] **Step 4: Keep short-press Field keys stable and focus Seq page**

In `TriggerFieldKeyAction`, keep A1-A4 and B1-B4 focusing `kSeqRhythm`:

```cpp
sharedCore_.SetActivePage(DaisySubharmoniqPage::kSeqRhythm);
```

For A5-A8 rhythm target cycling, do not leave the user on an obsolete Rhythm page. Focus the approved hub page:

```cpp
sharedCore_.SetActivePage(DaisySubharmoniqPage::kSeqRhythm);
```

Leave B5/B6/B7/B8 short-press behavior unchanged.

- [ ] **Step 5: Expand display status text**

Replace `BuildDisplay` body after title setup with a compact four-line status:

```cpp
const auto binding = sharedCore_.GetActivePageBinding();
display_.texts.push_back({0, 0, "Subharmoniq " + binding.pageLabel, true});
display_.texts.push_back({0,
                          10,
                          std::string(sharedCore_.IsPlaying() ? "Play" : "Stop")
                              + " Q " + FormatQuantize(sharedCore_.GetQuantizeMode())
                              + " Oct " + std::to_string(sharedCore_.GetSeqOctaveRange()),
                          false});
display_.texts.push_back({0,
                          20,
                          "S1 " + std::to_string(sharedCore_.GetSequencerStepIndex(0) + 1)
                              + " S2 "
                              + std::to_string(sharedCore_.GetSequencerStepIndex(1) + 1),
                          false});
display_.texts.push_back({0,
                          30,
                          "R1/" + std::to_string(sharedCore_.GetRhythmDivisor(0)) + " "
                              + FormatRhythmTarget(sharedCore_.GetRhythmTarget(0))
                              + " R2/" + std::to_string(sharedCore_.GetRhythmDivisor(1)) + " "
                              + FormatRhythmTarget(sharedCore_.GetRhythmTarget(1)),
                          false});
display_.texts.push_back({0,
                          40,
                          "R3/" + std::to_string(sharedCore_.GetRhythmDivisor(2)) + " "
                              + FormatRhythmTarget(sharedCore_.GetRhythmTarget(2))
                              + " R4/" + std::to_string(sharedCore_.GetRhythmDivisor(3)) + " "
                              + FormatRhythmTarget(sharedCore_.GetRhythmTarget(3)),
                          false});
display_.texts.push_back({0, 52, "Assign S1 OSC/SUB S2 OSC/SUB", false});
```

- [ ] **Step 6: Run compile**

Run:

```powershell
cmake --build build --config Debug --target unit_tests
```

Expected: build succeeds or only tests fail due old expectations.

---

### Task 4: Lock Field Key Mapping And Runtime Behavior

**Files:**

- Modify: `tests/test_board_control_mapping.cpp`
- Modify: `tests/test_render_runtime.cpp` only if needed

- [ ] **Step 1: Strengthen board-control mapping test**

In `SubharmoniqFieldKeysMapToDedicatedPerformanceActions`, add explicit assertions for all stable keys:

```cpp
const std::array<const char*, 16> expectedLabels = {{
    "Seq1 Step1", "Seq1 Step2", "Seq1 Step3", "Seq1 Step4",
    "Rhythm 1", "Rhythm 2", "Rhythm 3", "Rhythm 4",
    "Seq2 Step1", "Seq2 Step2", "Seq2 Step3", "Seq2 Step4",
    "Quantize", "Seq Octave", "Play/Stop", "Reset",
}};
const std::array<const char*, 16> expectedTargets = {{
    "node0/menu/field_keys/a1", "node0/menu/field_keys/a2",
    "node0/menu/field_keys/a3", "node0/menu/field_keys/a4",
    "node0/menu/field_keys/a5", "node0/menu/field_keys/a6",
    "node0/menu/field_keys/a7", "node0/menu/field_keys/a8",
    "node0/menu/field_keys/b1", "node0/menu/field_keys/b2",
    "node0/menu/field_keys/b3", "node0/menu/field_keys/b4",
    "node0/menu/field_keys/b5", "node0/menu/field_keys/b6",
    "node0/menu/field_keys/b7", "node0/menu/field_keys/b8",
}};
for(std::size_t i = 0; i < mapping.keys.size(); ++i)
{
    EXPECT_EQ(mapping.keys[i].detailLabel, expectedLabels[i]);
    EXPECT_EQ(mapping.keys[i].targetKind,
              daisyhost::BoardSurfaceTargetKind::kMenuItem);
    EXPECT_EQ(mapping.keys[i].targetId, expectedTargets[i]);
    EXPECT_EQ(mapping.keys[i].midiNote, -1);
}
```

- [ ] **Step 2: Update hosted-wrapper key action expectations**

In `HostedWrapperFieldKeyMenuActionsMatchFirmwareControls`, change A5 active page expectation to:

```cpp
EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kSeqRhythm);
```

In `FieldB7StartsAudioAndBothSequencersAfterA7RhythmEdit`, change A7 active page expectation to:

```cpp
EXPECT_EQ(core.GetActivePage(), daisyhost::DaisySubharmoniqPage::kSeqRhythm);
```

- [ ] **Step 3: Add runtime proof only if no existing test covers page focus**

If `tests/test_render_runtime.cpp::SubharmoniqFieldB7StartsAudioAfterA7RhythmEdit` already sends A7/B7 Field events and checks audio/steps, update its expected page evidence if present. Do not add duplicate runtime coverage if the test only verifies audio behavior.

- [ ] **Step 4: Run targeted tests**

Run:

```powershell
ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|SubharmoniqCoreTest|RenderRuntimeTest)"
```

Expected before final implementation: focused failures identify any old `Home`, `Voice`, `Rhythm`, or `Filter` expectations that still need migration.

---

### Task 5: Green The Targeted Gate And Update Tracker

**Files:**

- Modify: `PROJECT_TRACKER.md`
- Modify: `SKILL_PLAYBOOK.md` only if required by local skill-playbook conventions

- [ ] **Step 1: Run green build**

Run:

```powershell
cmake --build build --config Debug --target unit_tests
```

Expected: build passes.

- [ ] **Step 2: Run focused tests**

Run:

```powershell
ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|SubharmoniqCoreTest|RenderRuntimeTest|HostModulationTest)"
```

Expected: all selected tests pass.

- [ ] **Step 3: Build standalone**

Run:

```powershell
cmake --build build --config Debug --target DaisyHostPatch_Standalone
```

Expected: build passes. If raw PowerShell hits duplicate `Path` / `PATH`, rerun using the existing normalized-env pattern already documented in `PROJECT_TRACKER.md` and record both outcomes.

- [ ] **Step 4: Run Field standalone smoke**

Run:

```powershell
py -3 tests\run_smoke.py --mode standalone --build-dir build --source-dir . --config Debug --board daisy_field --timeout-seconds 60
```

Expected: smoke passes. If the helper rejects `--board`, run the existing Daisy Field standalone smoke path and record the exact substitute command.

- [ ] **Step 5: Update project tracker**

Prepend a `Latest implementation iteration` entry in `PROJECT_TRACKER.md` with:

```markdown
- Date: 2026-04-28
- Thread: Local Codex thread
- Slice: Subharmoniq Field control upgrade
- Manager-readable result:
  - Done: Subharmoniq Field now uses a Seq/Rhythm-first page model with stable A/B performance keys and VCO, VCF, and VCA/Mix knob layers.
  - Why it matters: the Field UI now matches the Subharmonicon manual model while keeping performance muscle memory stable.
  - Next: manual visual/audio validation in the standalone app remains separate unless explicitly run.
- Affected surfaces:
  - `include/daisyhost/DaisySubharmoniqCore.h`
  - `src/DaisySubharmoniqCore.cpp`
  - `src/apps/SubharmoniqCore.cpp`
  - `tests/test_subharmoniq_core.cpp`
  - `tests/test_board_control_mapping.cpp`
  - `tests/test_render_runtime.cpp` if touched
- Exact checks run:
  - Red build: record the exact `cmake --build build --config Debug --target unit_tests` result from Task 1.
  - Green build: record the exact `cmake --build build --config Debug --target unit_tests` result from Task 5.
  - Targeted tests: record the exact `ctest --test-dir build -C Debug --output-on-failure -R "(BoardControlMappingTest|SubharmoniqCoreTest|RenderRuntimeTest|HostModulationTest)"` result from Task 5.
  - Standalone build: record the exact `cmake --build build --config Debug --target DaisyHostPatch_Standalone` result from Task 5.
  - Field smoke: record the exact `py -3 tests\run_smoke.py --mode standalone --build-dir build --source-dir . --config Debug --board daisy_field --timeout-seconds 60` result from Task 5.
- Notes:
  - Firmware, DAW/VST behavior, and Patch board behavior were not changed.
  - Existing unrelated dirty work was preserved.
```

Record the exact command outputs from the current run; do not copy stale tracker evidence from an earlier iteration.

---

## Plan Self-Review

- Spec coverage: the tasks cover the stable A/B key contract, four functional pages, Seq/Rhythm default, page-specific K mappings, LED assumptions, RSP/display status, tests, standalone build, smoke, and tracker update.
- Scope check: the plan is one host-side Subharmoniq/Field slice. It does not include firmware, Patch board changes, DAW/VST behavior, or unrelated DSP.
- Type consistency: the new page enum names are `kSeqRhythm`, `kVco`, `kVcf`, and `kVcaMix` in both tests and implementation steps.
- Dirty worktree handling: no commit is required by this plan because the current workspace already contains unrelated dirty changes.
