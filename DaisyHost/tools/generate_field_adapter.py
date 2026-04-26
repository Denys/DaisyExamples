#!/usr/bin/env python3
"""Generate a Daisy Field firmware adapter from a DaisyHost shared app spec."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any


def load_spec(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as handle:
        spec = json.load(handle)
    required = ["display_name", "target_name", "core", "makefile", "audio", "controls"]
    missing = [key for key in required if key not in spec]
    if missing:
        raise SystemExit(f"Adapter spec missing required keys: {', '.join(missing)}")
    return spec


def render_makefile(spec: dict[str, Any]) -> str:
    makefile = spec["makefile"]
    target = spec["target_name"]
    source = spec["core"]["source"]
    return f"""# Project Name
TARGET = {target}

# Sources
CPP_SOURCES = {target}.cpp {source}

# Library Locations
LIBDAISY_DIR = {makefile["libdaisy_dir"]}
DAISYSP_DIR = {makefile["daisysp_dir"]}
C_INCLUDES += -I{makefile["include_dir"]}
OPT = {makefile.get("opt", "-Os")}

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
"""


def render_source(spec: dict[str, Any]) -> str:
    target = spec["target_name"]
    core = spec["core"]
    audio = spec["audio"]
    controls = spec["controls"]
    class_name = core["class_name"]
    node_id = core.get("node_id", "node0")
    knob_targets = [item["target"] for item in controls["knobs"][:4]]
    k5_target = controls["knobs"][4]["target"]
    cv1_target = controls["cv_inputs"][0]["target"]
    sw1_target = controls["switches"][0]["target"]
    block_size = audio.get("block_size", 48)

    return f"""#include "daisy_field.h"
#include "{core["header"]}"
#include "daisysp.h"

#include <array>
#include <cmath>
#include <cstdio>

using namespace daisy;
using namespace daisyhost;
using namespace {core["namespace"]};

namespace
{{
DaisyField hw;
{class_name} core("{node_id}");
{class_name}::DelayLineType DSY_SDRAM_BSS
    sdramDelays[{core["delay_count_symbol"]}];

constexpr size_t kAudioBlockSize = {block_size};
constexpr float  kTouchEpsilon   = 0.0005f;
constexpr float  kLedDim         = 0.06f;

constexpr size_t kKnobLeds[8] = {{
    DaisyField::LED_KNOB_1,
    DaisyField::LED_KNOB_2,
    DaisyField::LED_KNOB_3,
    DaisyField::LED_KNOB_4,
    DaisyField::LED_KNOB_5,
    DaisyField::LED_KNOB_6,
    DaisyField::LED_KNOB_7,
    DaisyField::LED_KNOB_8,
}};

constexpr size_t kTopRowLeds[8] = {{
    DaisyField::LED_KEY_A1,
    DaisyField::LED_KEY_A2,
    DaisyField::LED_KEY_A3,
    DaisyField::LED_KEY_A4,
    DaisyField::LED_KEY_A5,
    DaisyField::LED_KEY_A6,
    DaisyField::LED_KEY_A7,
    DaisyField::LED_KEY_A8,
}};

constexpr size_t kBottomRowLeds[8] = {{
    DaisyField::LED_KEY_B1,
    DaisyField::LED_KEY_B2,
    DaisyField::LED_KEY_B3,
    DaisyField::LED_KEY_B4,
    DaisyField::LED_KEY_B5,
    DaisyField::LED_KEY_B6,
    DaisyField::LED_KEY_B7,
    DaisyField::LED_KEY_B8,
}};

const char* kKnobControlIds[4] = {{
    "{knob_targets[0]}",
    "{knob_targets[1]}",
    "{knob_targets[2]}",
    "{knob_targets[3]}",
}};
const char* k5ParameterId = "{k5_target}";
const char* cv1PortId     = "{cv1_target}";
const char* fireImpulseMenuItemId = "{sw1_target}";

std::array<float, 8> knobValues{{}};
float                cv1Value         = 0.0f;
float                lastK5Value      = -1.0f;
float                lastCv1Value     = -1.0f;
int                  oledPage         = 0;
int                  fireImpulsePulse = 0;
bool                 sw2WasPressed    = false;

float Clamp01(float value)
{{
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}}

bool HasMeaningfulChange(float value, float previous)
{{
    return previous < -0.5f || fabsf(value - previous) > kTouchEpsilon;
}}

uint16_t ToDacCode(float normalized)
{{
    return static_cast<uint16_t>(Clamp01(normalized) * 4095.0f);
}}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{{
    const float* inputPtrs[1]  = {{in[0]}};
    float*       outputPtrs[2] = {{out[0], out[1]}};

    core.Process({{inputPtrs, 1}},
                 {{outputPtrs, 2}},
                 size);
}}

void WriteDisplayLine(int y, const char* text)
{{
    hw.display.SetCursor(0, y);
    hw.display.WriteString(text, Font_6x8, true);
}}

void DrawCoreDisplay()
{{
    const DisplayModel& model = core.GetDisplayModel();

    for(const auto& text : model.texts)
    {{
        hw.display.SetCursor(text.x, text.y);
        hw.display.WriteString(text.text.c_str(), Font_6x8, !text.inverted);
    }}

    for(const auto& bar : model.bars)
    {{
        const int x2 = bar.x + bar.width;
        const int y2 = bar.y + bar.height;
        hw.display.DrawRect(bar.x, bar.y, x2, y2, true, false);

        const int fillWidth = static_cast<int>(bar.width * bar.normalized);
        if(fillWidth > 0)
        {{
            hw.display.DrawRect(
                bar.x, bar.y, bar.x + fillWidth, y2, true, true);
        }}
    }}
}}

void DrawHardwareStatus()
{{
    char line[32];
    WriteDisplayLine(0, "Field {target}");

    snprintf(line,
             sizeof(line),
             "K1 %02d K2 %02d",
             static_cast<int>(knobValues[0] * 99.0f),
             static_cast<int>(knobValues[1] * 99.0f));
    WriteDisplayLine(10, line);

    snprintf(line,
             sizeof(line),
             "K3 %02d K4 %02d",
             static_cast<int>(knobValues[2] * 99.0f),
             static_cast<int>(knobValues[3] * 99.0f));
    WriteDisplayLine(20, line);

    snprintf(line,
             sizeof(line),
             "K5/CV1 D3 %02d/%02d",
             static_cast<int>(knobValues[4] * 99.0f),
             static_cast<int>(cv1Value * 99.0f));
    WriteDisplayLine(30, line);

    snprintf(line,
             sizeof(line),
             "CV OUT1 %1.2fV",
             Clamp01(knobValues[4]) * 5.0f);
    WriteDisplayLine(40, line);

    WriteDisplayLine(52, "SW1 Impulse SW2 Page");
}}

void UpdateDisplay()
{{
    hw.display.Fill(false);
    if(oledPage == 0)
    {{
        DrawCoreDisplay();
    }}
    else
    {{
        DrawHardwareStatus();
    }}
    hw.display.Update();
}}

void UpdateLeds()
{{
    for(size_t i = 0; i < 8; ++i)
    {{
        hw.led_driver.SetLed(kKnobLeds[i], knobValues[i]);
    }}

    for(size_t i = 0; i < 8; ++i)
    {{
        const float value = i < 5 ? (0.15f + (0.70f * knobValues[i])) : 0.0f;
        hw.led_driver.SetLed(kTopRowLeds[i], value);
    }}

    for(size_t i = 0; i < 8; ++i)
    {{
        hw.led_driver.SetLed(kBottomRowLeds[i],
                             i == static_cast<size_t>(oledPage) ? 0.35f : 0.0f);
    }}

    hw.led_driver.SetLed(DaisyField::LED_SW_1,
                         fireImpulsePulse > 0 ? 1.0f : kLedDim);
    hw.led_driver.SetLed(DaisyField::LED_SW_2,
                         oledPage == 1 ? 0.35f : kLedDim);
    hw.led_driver.SwapBuffersAndTransmit();
}}

void ProcessControls()
{{
    hw.ProcessAllControls();

    for(size_t i = 0; i < 8; ++i)
    {{
        knobValues[i] = hw.GetKnobValue(i);
    }}

    for(size_t i = 0; i < 4; ++i)
    {{
        core.SetControl(kKnobControlIds[i], knobValues[i]);
    }}

    if(HasMeaningfulChange(knobValues[4], lastK5Value))
    {{
        lastK5Value = knobValues[4];
        core.SetParameterValue(k5ParameterId, knobValues[4]);
    }}

    cv1Value = hw.GetCvValue(DaisyField::CV_1);
    if(HasMeaningfulChange(cv1Value, lastCv1Value))
    {{
        lastCv1Value = cv1Value;
        PortValue value;
        value.type   = VirtualPortType::kCv;
        value.scalar = cv1Value;
        core.SetPortInput(cv1PortId, value);
    }}

    if(hw.GetSwitch(DaisyField::SW_1)->RisingEdge())
    {{
        core.SetMenuItemValue(fireImpulseMenuItemId, 1.0f);
        fireImpulsePulse = 12;
    }}

    const bool sw2Pressed = hw.GetSwitch(DaisyField::SW_2)->Pressed();
    if(sw2Pressed && !sw2WasPressed)
    {{
        oledPage = (oledPage + 1) % 2;
    }}
    sw2WasPressed = sw2Pressed;

    if(fireImpulsePulse > 0)
    {{
        --fireImpulsePulse;
    }}

    hw.SetCvOut1(ToDacCode(knobValues[4]));
    hw.SetCvOut2(0);
}}
}} // namespace

int main(void)
{{
    hw.Init();
    hw.SetAudioBlockSize(kAudioBlockSize);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    core.AttachDelayStorage(sdramDelays, {core["delay_count_symbol"]});
    core.Prepare(hw.AudioSampleRate(), hw.AudioBlockSize());

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while(true)
    {{
        ProcessControls();
        core.TickUi(0.0);
        UpdateDisplay();
        UpdateLeds();
        System::Delay(2);
    }}
}}
"""


def render_readme(spec: dict[str, Any]) -> str:
    target = spec["target_name"]
    return f"""# Field {target}

`field/{target}` is the first generated Daisy Field firmware adapter for
DaisyHost. It runs the shared `{spec["core"]["namespace"]}::{spec["core"]["class_name"]}`
on real Daisy Field hardware so the same app core used by DaisyHost has a
hardware-facing Field path.

The Daisy Field is expected to be connected to the development computer through
an ST-Link programmer/debugger for flashing and hardware validation.

## Build

```sh
make
```

## Flash

```sh
make program
```

## Validation Status

Generated firmware may be described as build-verified or flash-verified only
after the corresponding command passes. Full hardware validation still requires
the manual checklist in `CONTROLS.md`.

Until that checklist is complete, describe this generated adapter as
flash-verified only when `make program` has actually reported verification
success.
"""


def render_controls(spec: dict[str, Any]) -> str:
    rows: list[str] = []
    controls = spec["controls"]
    for group in ("knobs", "cv_inputs", "cv_outputs", "switches"):
        for item in controls[group]:
            rows.append(f"| {item['label']} | {item['behavior']} |")
    rows.extend(
        [
            "| A/B key LEDs | Visual status only; no musical key behavior in this adapter |",
            "| Knob LEDs | Mirror K1-K8 physical knob values |",
        ]
    )
    checklist = "\n".join(f"- {item}" for item in spec["validation_checklist"])
    return f"""# Field {spec["target_name"]} Controls

## Hardware Mapping

| Field control | Firmware behavior |
|---|---|
{chr(10).join(rows)}

## Manual Hardware Validation Checklist

Record date, build commit or diff state, and tester name before marking this
adapter hardware-validated.

{checklist}

## Notes

- Field controls are processed in the main loop, not in the audio callback.
- The audio callback only runs `{spec["core"]["class_name"]}::Process(...)` and writes audio
  buffers.
- This generated adapter intentionally keeps K6-K8, CV2-CV4, and musical key
  behavior out of scope for v1.
"""


def write_generated_adapter(spec: dict[str, Any], out_dir: Path) -> None:
    out_dir.mkdir(parents=True, exist_ok=True)
    target = spec["target_name"]
    files = {
        ".gitignore": "build/\n",
        "Makefile": render_makefile(spec),
        f"{target}.cpp": render_source(spec),
        "README.md": render_readme(spec),
        "CONTROLS.md": render_controls(spec),
    }
    for name, text in files.items():
        (out_dir / name).write_text(text, encoding="utf-8", newline="\n")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Generate a Daisy Field firmware adapter from a DaisyHost app spec."
    )
    parser.add_argument("--spec", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    args = parser.parse_args()

    spec = load_spec(args.spec)
    write_generated_adapter(spec, args.out)
    print(f"generated {spec['target_name']} Field adapter at {args.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
