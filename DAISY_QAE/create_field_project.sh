#!/bin/bash
# create_field_project.sh — Scaffolds a Daisy Field project with correct patterns
#
# Fixes the helper.py anti-pattern (ProcessAllControls in AudioCallback)
# and integrates field_defaults.h from the start.
#
# Usage:  ./create_field_project.sh ProjectName
# Result: MyProjects/_projects/ProjectName/ with .dvpe plan, corrected .cpp, Makefile, CONTROLS.md

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXAMPLES_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BASE_DIR="$EXAMPLES_DIR/MyProjects/_projects"

if [ $# -lt 1 ]; then
    echo "Usage: $0 <ProjectName>"
    echo "Creates: $BASE_DIR/<ProjectName>/"
    exit 1
fi

PROJECT_NAME="$1"
PROJECT_DIR="$BASE_DIR/$PROJECT_NAME"

if [ -d "$PROJECT_DIR" ]; then
    echo "Error: $PROJECT_DIR already exists"
    exit 1
fi

# Step 1: Use helper.py to get correct Makefile paths and .vscode configs
echo "1/5  Running helper.py create..."
cd "$EXAMPLES_DIR"
python helper.py create "MyProjects/_projects/$PROJECT_NAME" -b field

# Step 2: Create .dvpe planning artifact before implementation logic
echo "2/5  Creating $PROJECT_NAME.dvpe planning template..."
cat > "$PROJECT_DIR/$PROJECT_NAME.dvpe" << 'DVPEEOF'
{
  "version": "1.0.0",
  "patch": {
    "metadata": {
      "name": "PROJECT_NAME_PLACEHOLDER",
      "author": "",
      "description": "TODO: complete this visual source project before firmware implementation. Define DSP blocks, signal/control connections, parameter ranges, defaults, and Field control bindings.",
      "created": "",
      "modified": "",
      "version": "0.1.0",
      "targetHardware": "field",
      "sampleRate": 48000,
      "blockSize": 48
    },
    "blocks": [
      {
        "id": "knobs_1",
        "definitionId": "knob",
        "position": {
          "x": -800,
          "y": -160
        },
        "parameterValues": {
          "controls": "K1-K8",
          "status": "TODO: parameter ranges/defaults"
        },
        "label": "K1-K8 CONTROL PLAN"
      },
      {
        "id": "keybed_1",
        "definitionId": "keyboard_input",
        "position": {
          "x": -800,
          "y": 40
        },
        "parameterValues": {
          "aRow": "TODO",
          "bRow": "TODO",
          "scanRule": "A uses Keyboard indices 8-15, B uses 0-7"
        },
        "label": "FIELD KEYBED PLAN"
      },
      {
        "id": "midi_note_1",
        "definitionId": "midi_note",
        "position": {
          "x": -520,
          "y": -260
        },
        "parameterValues": {
          "channel": "TODO"
        },
        "label": "MIDI / GATE SOURCE"
      },
      {
        "id": "dsp_placeholder_1",
        "definitionId": "audio_input",
        "position": {
          "x": -220,
          "y": -120
        },
        "parameterValues": {
          "status": "Replace with intended oscillator/filter/effect chain"
        },
        "label": "DSP CHAIN TODO"
      },
      {
        "id": "audio_output_1",
        "definitionId": "audio_output",
        "position": {
          "x": 120,
          "y": -120
        },
        "parameterValues": {
          "channels": "TODO"
        },
        "label": "AUDIO OUT"
      },
      {
        "id": "oled_display_1",
        "definitionId": "oled_display",
        "position": {
          "x": 120,
          "y": 80
        },
        "parameterValues": {
          "status": "TODO: display pages and value formats"
        },
        "label": "OLED PLAN"
      }
    ],
    "connections": [
      {
        "id": "conn-dsp-out",
        "sourceBlockId": "dsp_placeholder_1",
        "sourcePortId": "out",
        "targetBlockId": "audio_output_1",
        "targetPortId": "in",
        "type": "audio"
      },
      {
        "id": "conn-knobs-dsp",
        "sourceBlockId": "knobs_1",
        "sourcePortId": "values",
        "targetBlockId": "dsp_placeholder_1",
        "targetPortId": "params",
        "type": "control"
      }
    ]
  }
}
DVPEEOF
sed -i "s/PROJECT_NAME_PLACEHOLDER/$PROJECT_NAME/g" "$PROJECT_DIR/$PROJECT_NAME.dvpe"

# Step 3: Overwrite the .cpp with corrected template
echo "3/5  Writing corrected $PROJECT_NAME.cpp..."
cat > "$PROJECT_DIR/$PROJECT_NAME.cpp" << 'CPPEOF'
#include "daisy_field.h"
#include "daisysp.h"
#include "../../foundation_examples/field_defaults.h"

using namespace daisy;
using namespace daisysp;
using namespace FieldDefaults;

DaisyField        hw;
FieldKeyboardLEDs keyLeds;
FieldOLEDDisplay  display;

// ---------- DSP Modules ----------
// Declare oscillators, filters, effects here

// ---------- Parameters ----------
struct Params {
    float param1 = 0.5f;
    float param2 = 0.5f;
    // Map all 8 knobs
} params;

// ---------- Control Processing (main loop only) ----------
void ProcessKnobs() {
    params.param1 = hw.knob[0].Process();
    params.param2 = hw.knob[1].Process();
    // Update DSP modules with smoothed values
}

// ---------- Audio Callback ----------
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size) {
    // Audio processing ONLY — no control reads, no allocations, no prints
    for(size_t i = 0; i < size; i++) {
        out[0][i] = in[0][i];
        out[1][i] = in[1][i];
    }
}

// ---------- Main ----------
int main(void) {
    hw.Init();
    hw.SetAudioBlockSize(48);  // ~1ms latency at 48kHz
    float sr = hw.AudioSampleRate();

    // Initialize helpers
    keyLeds.Init(&hw);
    display.Init(&hw);
    display.SetTitle("NEW PROJECT");

    // Initialize DSP modules with sample rate
    // osc.Init(sr);
    // filter.Init(sr);

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(1) {
        hw.ProcessAllControls();  // Correct: main loop, not callback
        ProcessKnobs();

        // TODO: Handle A-row keyboard with kKeyAIndices[] / keyLeds.SetA().
        // TODO: Handle B-row keyboard with kKeyBIndices[] / keyLeds.SetB().
        // Field key scan indices and native LED_KEY_A/B names are not physical
        // row truth; keep using the FieldDefaults adapter.
        // TODO: Handle switches (SW1, SW2)

        keyLeds.Update();
        display.Update();
        System::Delay(16);  // ~60Hz UI refresh
    }
}
CPPEOF

# Step 4: Append LGPL flag to Makefile if not present
echo "4/5  Updating Makefile..."
if ! grep -q "USE_DAISYSP_LGPL" "$PROJECT_DIR/Makefile"; then
    # Insert LGPL flag before the SYSTEM_FILES_DIR line
    sed -i '/^SYSTEM_FILES_DIR/i # Uncomment for LGPL modules (ReverbSc, ModalVoice, StringVoice, MoogLadder)\n# USE_DAISYSP_LGPL = 1\n' "$PROJECT_DIR/Makefile"
fi

# Step 5: Create CONTROLS.md template
echo "5/5  Creating CONTROLS.md..."
cat > "$PROJECT_DIR/CONTROLS.md" << 'CTRLEOF'
# Controls — PROJECT_NAME_PLACEHOLDER

## Knobs

| Knob | Parameter | Range | Default | Notes |
|------|-----------|-------|---------|-------|
| K1   | TBD       | 0-1   | 0.5     |       |
| K2   | TBD       | 0-1   | 0.5     |       |
| K3   | TBD       | 0-1   | 0.5     |       |
| K4   | TBD       | 0-1   | 0.5     |       |
| K5   | TBD       | 0-1   | 0.5     |       |
| K6   | TBD       | 0-1   | 0.5     |       |
| K7   | TBD       | 0-1   | 0.5     |       |
| K8   | TBD       | 0-1   | 0.5     |       |

## Keys — Row A (Top)

Use `kKeyAIndices[]` and `keyLeds.SetA()/ToggleA()` from `field_defaults.h`.
Do not hard-code raw keyboard scan indices or native `LED_KEY_A/B` enum names
as physical row truth.

| Key | Function |
|-----|----------|
| A1  | TBD      |
| A2  | TBD      |
| A3  | TBD      |
| A4  | TBD      |
| A5  | TBD      |
| A6  | TBD      |
| A7  | TBD      |
| A8  | TBD      |

## Keys — Row B (Bottom)

Use `kKeyBIndices[]` and `keyLeds.SetB()/ToggleB()` from `field_defaults.h`.

| Key | Function |
|-----|----------|
| B1  | TBD      |
| B2  | TBD      |
| B3  | TBD      |
| B4  | TBD      |
| B5  | TBD      |
| B6  | TBD      |
| B7  | TBD      |
| B8  | TBD      |

## Switches

| Switch | Function |
|--------|----------|
| SW1    | TBD      |
| SW2    | TBD      |

## OLED Display

Use integer-only `snprintf` formatting for firmware OLED/serial values. Default
Daisy Make builds use newlib-nano without float printf support, so `%f` values
can render blank unless `_printf_float` is deliberately linked and flash size is
checked.

| Screen | Content |
|--------|---------|
| Default | Project name + parameter overview |
| Knob Zoom | Active parameter name + value + bar |

## Presets (if applicable)

| Preset | Description | Key Settings |
|--------|-------------|--------------|
| 1      | TBD         |              |
CTRLEOF

# Replace placeholder with actual project name
sed -i "s/PROJECT_NAME_PLACEHOLDER/$PROJECT_NAME/" "$PROJECT_DIR/CONTROLS.md"

echo ""
echo "Done! Created: $PROJECT_DIR/"
echo "  $PROJECT_NAME.dvpe — Visual source project to complete before logic"
echo "  $PROJECT_NAME.cpp  — Corrected template (no ProcessAllControls in callback)"
echo "  Makefile            — With LGPL flag option"
echo "  CONTROLS.md         — Parameter mapping template"
echo ""
echo "Next steps:"
echo "  1. Complete $PROJECT_NAME.dvpe with the DSP/control block plan"
echo "  2. Fill in CONTROLS.md with your parameter plan"
echo "  3. Add DSP modules and Init() calls in main()"
echo "  4. make clean && make"
