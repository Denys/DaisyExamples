# LED Color Organ - Daisy Pod Project

A proof-of-concept interactive LED controller that demonstrates all physical I/O capabilities of the Daisy Pod.

## Overview

This project showcases the relationship between physical controls and visual feedback, making it an ideal learning project before diving into audio processing. The two RGB LEDs respond to all physical inputs, creating an interactive light controller.

## Features

### Control Mapping

| Control | Function |
|---------|----------|
| **Knob 1** | LED 1 Hue (0°–360° color wheel) |
| **Knob 2** | LED 2 Hue (0°–360° color wheel) |
| **Encoder Turn** | Master brightness (both LEDs) |
| **Encoder Press** | Toggle color mode (Independent / Mirror / Complement / Rainbow) |
| **Button 1** | LED 1 intensity boost (momentary, or toggle with double-tap) |
| **Button 2** | LED 2 intensity boost (momentary, or toggle with double-tap) |

### Color Modes

1. **Independent** - Each knob controls its respective LED color directly
2. **Mirror** - Both LEDs show the same color (average of knob positions)
3. **Complement** - LED 2 shows the complementary color (180° offset) of LED 1
4. **Rainbow Cycle** - Automatic hue rotation with encoder controlling speed

## Building

### Prerequisites

- ARM GCC toolchain installed
- Make utility
- Daisy Pod hardware

### Build Instructions

```bash
# Navigate to project directory
cd MyProjects/ExampleProjec_openrouter

# Build the project
make

# Clean build files
make clean

# Program the Daisy Pod
make program-dfu
```

## Usage

1. **Power on** your Daisy Pod
2. **Turn Knob 1 and Knob 2** to select colors for each LED
3. **Turn the Encoder** to adjust overall brightness
4. **Press the Encoder** to cycle through color modes
5. **Press Button 1 or Button 2** momentarily to boost LED intensity
6. **Double-tap Button 1 or Button 2** to toggle permanent boost

## Technical Details

- Uses HSV to RGB color space conversion
- Smooth brightness transitions with Port filter
- Double-tap button detection (300ms window)
- Sample rate: 48kHz with 48-sample blocks
- Control smoothing: 50ms time constant

## Code Structure

- **HsvToRgb()** - Color space conversion
- **UpdateEncoder()** - Handles encoder input and mode switching
- **UpdateButtons()** - Button state management with double-tap detection
- **UpdateLEDs()** - Main LED control logic with mode-specific behavior
- **ProcessControls()** - Aggregates all control inputs
- **AudioCallback()** - Called at audio rate for smooth LED response

## Extending the Project

This project can be extended to:
- Add audio reactivity using Line IN amplitude
- MIDI control via TRS MIDI input
- MicroSD card for preset storage
- Pattern sequencing with timing
- More color modes (Strobe, Pulse, Chase patterns)

## License

This project is based on the Electrosmith Daisy examples and follows the same license terms.

## Author

Created as part of the Daisy Pod Creative Projects collection.
