#!/bin/bash
# create_field_project.sh — Scaffolds a Daisy Field project with correct patterns
#
# Fixes the helper.py anti-pattern (ProcessAllControls in AudioCallback)
# and integrates field_defaults.h from the start.
#
# Usage:  ./create_field_project.sh ProjectName
# Result: MyProjects/_projects/ProjectName/ with corrected .cpp, Makefile, CONTROLS.md

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
echo "1/4  Running helper.py create..."
cd "$EXAMPLES_DIR"
python helper.py create "MyProjects/_projects/$PROJECT_NAME" -b field

# Step 2: Overwrite the .cpp with corrected template
echo "2/4  Writing corrected $PROJECT_NAME.cpp..."
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

        // TODO: Handle A-row keyboard (note selection)
        // TODO: Handle B-row keyboard (mode/preset)
        // TODO: Handle switches (SW1, SW2)

        keyLeds.Update();
        display.Update();
        System::Delay(16);  // ~60Hz UI refresh
    }
}
CPPEOF

# Step 3: Append LGPL flag to Makefile if not present
echo "3/4  Updating Makefile..."
if ! grep -q "USE_DAISYSP_LGPL" "$PROJECT_DIR/Makefile"; then
    # Insert LGPL flag before the SYSTEM_FILES_DIR line
    sed -i '/^SYSTEM_FILES_DIR/i # Uncomment for LGPL modules (ReverbSc, ModalVoice, StringVoice, MoogLadder)\n# USE_DAISYSP_LGPL = 1\n' "$PROJECT_DIR/Makefile"
fi

# Step 4: Create CONTROLS.md template
echo "4/4  Creating CONTROLS.md..."
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
echo "  $PROJECT_NAME.cpp  — Corrected template (no ProcessAllControls in callback)"
echo "  Makefile            — With LGPL flag option"
echo "  CONTROLS.md         — Parameter mapping template"
echo ""
echo "Next steps:"
echo "  1. Fill in CONTROLS.md with your parameter plan"
echo "  2. Add DSP modules and Init() calls in main()"
echo "  3. make clean && make"
