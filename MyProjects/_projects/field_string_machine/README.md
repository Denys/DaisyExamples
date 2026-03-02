# Field String Machine

## Description
This project implements an **Exotic Oscillator Bank String Machine** on the Daisy Field. 

It utilizes the custom `OscillatorBank` block which provides 7 detuned oscillators (Sawtooth and Square waves at 8', 4', 2', and 1' octaves) to create a rich, thick "ensemble" or "string machine" sound.

The sound is shaped by a Moog Ladder Filter and controlled via an ADSR envelope triggered by MIDI.

## Hardware Requirements
*   **Daisy Field**
*   MIDI Keyboard (connected to Daisy Field MIDI In)

## Controls

| Control | Function | Description |
| :--- | :--- | :--- |
| **Knob 1** | Amp Saw 8' | Level of 8-foot Sawtooth wave |
| **Knob 2** | Amp Sqr 8' | Level of 8-foot Square wave |
| **Knob 3** | Amp Saw 4' | Level of 4-foot Sawtooth wave |
| **Knob 4** | Amp Sqr 4' | Level of 4-foot Square wave |
| **Knob 5** | Amp Saw 2' | Level of 2-foot Sawtooth wave |
| **Knob 6** | Amp Sqr 2' | Level of 2-foot Square wave |
| **Knob 7** | Amp Saw 1' | Level of 1-foot Sawtooth wave |
| **Knob 8** | Filter Cutoff | Cutoff frequency of the Moog Ladder Filter |

*Note: Knob 8 provides manual control over the filter cutoff, allowing for sweeps.*

## UX Features (FieldUX)
*   **OLED Display**: Shows project title "String Machine" and active status.
*   **Smoothed Knobs**: All knob inputs are filtered to prevent jitter.
*   **LED Feedback**: Status LEDs indicate gate activity.

## Signal Flow
1.  **MIDI Input** provides Pitch and Gate.
2.  **Pitch** is scaled to V/Octave and drives the **Oscillator Bank**.
3.  **Oscillator Bank** mixes 7 oscillator sub-outputs based on Knob 1-7 levels.
4.  **Audio** passes through a **VCA** controlled by an **ADSR Envelope** (triggered by MIDI Gate).
5.  **Output** of VCA goes through a **Moog Ladder Filter** (controlled by Knob 8).
6.  **Final Signal** is sent to the Audio Output (Stereo).

## Build Instructions

1.  Navigate to the project directory:
    ```bash
    cd DaisyExamples/MyProjects/_projects/field_string_machine
    ```
2.  Compile the project:
    ```bash
    make
    ```
3.  Flash to Daisy Field (put device in DFU mode by holding BOOT and pressing RESET):
    ```bash
    make program-dfu
    ```
