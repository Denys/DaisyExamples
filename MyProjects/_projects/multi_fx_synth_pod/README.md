# Daisy Pod Multi-Effect Synth

A production-ready C++ project for the **Daisy Pod** that combines a MIDI-controlled dual-oscillator synthesizer with an external audio processor rack.

## 🎧 Features

### 1. Dual Oscillator Synthesis
- **Oscillators**: Two high-quality saw-wave oscillators.
- **Detune**: Oscillator 2 is subtley detuned (~5 cents) from Oscillator 1 for a thick, rich sound.
- **MIDI Control**: Controlled via an external MIDI keyboard for pitch and gating.
- **Envelope**: Integrated ADSR envelope (Attack: 10ms, Decay: 100ms, Sustain: 80%, Release: 200ms).

### 2. Multi-Stage FX Rack
The project includes a sequential high-performance DSP chain:
1. **Mixer**: Sums the MIDI Synth output with the external Audio Input.
2. **Overdrive**: Adds harmonic saturation.
3. **Compressor**: Levels the dynamics (LGPL module).
4. **Delay**: 400ms delay line with feedback.
5. **Chorus**: Stereo spatialization effect.
6. **DC Block**: Removes low-frequency offset.
7. **Limiter**: Prevents digital clipping on the output.

## 🎛️ Controls

The project features a **Shift Mode** (via Button 1) to allow access to multiple parameters using the two hardware knobs.

### Global Controls
| Control | Description |
|---------|-------------|
| **Button 1** | **Shift Toggle**: Switches between **Base** and **Shift** modes. |
| **Button 2** | **FX Bypass**: Toggles the global FX rack on/off (Overdrive, Comp, Delay, Chorus). |
| **LED 1** | Indicates Mode: **Blue** = Base, **Green** = Shift. |
| **LED 2** | Indicates Bypass: Matches Mode color when active, **OFF** when bypassed. |

### Mode-Specific Controls

#### 1. Base Mode (Blue LED)
| Control | Description |
|---------|-------------|
| **Knob 1** | **Osc Blend**: Crossfade between Osc 1 (left) and Osc 2 (right). |
| **Knob 2** | **Drive**: Adjusts the overdrive saturation level. |

#### 2. Shift Mode (Green LED)
| Control | Description |
|---------|-------------|
| **Knob 1** | **Delay Time**: Adjusts delay from 50ms to 800ms. |
| **Knob 2** | **Chorus Depth**: Adjusts the intensity of the stereo chorus. |

### Signal Routing
- **MIDI In**: Controls pitch playback and triggers the ADSR envelope.
- **Audio In**: External stereo input is summed to mono and processed through the FX rack.

## 🛠️ Build & Flash

### Prerequisites
- [arm-none-eabi-gcc](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain)
- libDaisy and DaisySP libraries installed in the parent `DaisyExamples` directory.

### Commands
Navigate to the project directory and run:

```bash
# Clean and build the binary
make clean && make

# Flash to Daisy Pod (via OpenOCD/ST-Link)
make program

# Alternatively, flash via DFU (USB)
make program-dfu
```

## 📜 Source Files
- `multi_fx_synth_pod.cpp`: Core logic, audio callback, and module initialization.
- `Makefile`: Build script. Note that `USE_DAISYSP_LGPL = 1` is enabled to support the `Compressor` module.

---
*Created with the help of the DVPE Code Generation Agent.*
