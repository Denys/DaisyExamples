# Session Summary - 2026-04-26

## What We Did

- Built out the DaisyHost / Daisy Field workflow around SubharmoniqField, CloudSeed, MultiDelay, CV generators, and the Field host surface.
- Fixed the SubharmoniqField no-audio path earlier in the day, then adjusted defaults so the internal-clock/play path is audible and test-covered.
- Implemented DaisyHost Field right-side-panel separation: hosted-app SW1/SW2 and X/C navigation are distinct from DaisyHost program/drawer pages.
- Added CloudSeed app pages including performance/default, arpeggio, and safe advanced public-parameter controls.
- Fixed CV generator instability by preventing a generated CV lane from driving both app CV input and a latched knob/parameter target at the same time.
- Added CV target menu visibility for both default and alternative K sets using `Kx.1` and `Kx.2`, with unsafe audio-critical targets filtered.
- Redesigned Field RSP Page 1 after user feedback: Keyboard MIDI and MIDI Tracker now sit in a compact top strip, while the remaining drawer space is dedicated to an expanded four-lane modulation bay.
- Added a tested `ComputerKeyboardMidi::VisibleKeyboardStartForOctave` helper so the visible MIDI keyboard follows the selected octave.
- Added and updated local memory for the UI feedback: “make visible” means unfolded/readable, not compacted.

## Decisions Made

- Keep DaisyHost Field UI changes host-side unless firmware evidence requires firmware changes.
- Treat the user’s annotated screenshots as layout authority for Field RSP work.
- Move build/status helper text to Page 3 so Page 1 can be a modulation workspace.
- Use Page 1 as the testing/modulation engine surface and keep rack/topology/audio-source controls on Page 3.
- Interpret “visible” controls as directly inspectable and comfortably spaced, not merely present in a denser layout.
- Keep unsafe audio-critical modulation targets out of default CV generator target choices.

## Key Learnings

- The earlier compact Page 1 CV layout technically exposed the controls but violated the intended spatial/readability goal.
- DaisyHost Release builds can fail with `LNK1104` when `DaisyHost Hub` or `DaisyHost Patch` is still running and locking artifacts.
- The current automated harness can build and smoke-test the UI but does not capture a visual Field drawer screenshot for final eye-checking.
- The selected-octave keyboard issue is best handled through a small shared helper rather than hardcoded editor note bounds.

## Open Threads

- Manual standalone visual QA of Field Page 1 remains useful because screenshot capture was not run.
- Possible future modulation additions: per-target attenuverter, slew/smoothing, sample-and-hold/random, CV phase reset/sync, unipolar/bipolar toggle, and gate pulse length/probability.
- Hardware/firmware validation remains separate unless explicitly requested: Field flashing, audio/CV/MIDI checks, and DAW/VST3 manual validation.
- DaisyHostCLI planning/implementation remains a separate thread if resumed.

## Tools & Systems Touched

- Repo: `DaisyExamples/DaisyHost`
- Main files touched late in the session:
  - `include/daisyhost/ComputerKeyboardMidi.h`
  - `src/ComputerKeyboardMidi.cpp`
  - `src/juce/DaisyHostPluginEditor.cpp`
  - `tests/test_computer_keyboard_midi.cpp`
  - `PROJECT_TRACKER.md`
  - `CHECKPOINT.md`
  - `CHANGELOG.md`
  - `SKILL_PLAYBOOK.md`
- Memory files:
  - `C:/Users/denko/.codex/memories/MEMORY.md`
  - `C:/Users/denko/.codex/memories/feedback_ui_visibility_means_unfolded.md`
- Verification highlights:
  - Targeted Debug CTest passed `28/28`.
  - Full `cmd /c build_host.cmd` passed Release CTest `192/192` after closing locked DaisyHost processes.
