# Teensy 4.0 Setup And Programming Procedure

This procedure covers the companion controller for [Seed_MIDI_Grainlet](C:\Users\denko\Gemini\Antigravity\DVPE_Daisy-Visual-Programming-Environment\DaisyExamples\MyProjects\_projects\Seed_MIDI_Grainlet). The target sketch is [Seed_MIDI_Grainlet_Teensy40.ino](C:\Users\denko\Gemini\Antigravity\DVPE_Daisy-Visual-Programming-Environment\DaisyExamples\MyProjects\_projects\Seed_MIDI_Grainlet\teensy_controller\Seed_MIDI_Grainlet_Teensy40\Seed_MIDI_Grainlet_Teensy40.ino).

## 1. What This Teensy Sketch Does

The Teensy 4.0 acts as a simple hardware MIDI controller:

- reads three potentiometers
- reads one pushbutton
- sends raw UART MIDI on `Serial1` at `31250` baud to the Daisy Seed
- sends USB serial debug messages on `Serial` at `115200` baud to your computer

Control map:

- `A0` -> `CC 14` -> Grainlet shape
- `A1` -> `CC 15` -> Grainlet formant frequency
- `A2` -> `CC 16` -> Grainlet bleed
- `Pin 2` pushbutton -> fixed `Note On/Off` for MIDI note `60`

## 2. Hardware Needed

- `Teensy 4.0`
- USB cable for programming and serial monitor
- 3 potentiometers, typically `10k`
- 1 momentary pushbutton
- jumper wires
- common ground connection to Daisy Seed

## 3. Wiring The Teensy Controls

### Potentiometers

For each pot:

- one outer leg -> `3.3V`
- other outer leg -> `GND`
- center wiper -> Teensy analog pin

Suggested mapping:

- Pot 1 wiper -> `A0`
- Pot 2 wiper -> `A1`
- Pot 3 wiper -> `A2`

### Pushbutton

The sketch uses `INPUT_PULLUP`, so wire the button simply:

- one side of the pushbutton -> `Pin 2`
- other side -> `GND`

Pressed = logic low. Released = logic high.

## 4. Wiring Teensy To Daisy

For the MIDI UART link:

- Teensy `TX1` -> Daisy Seed `D14` (`USART1_RX`)
- Teensy `GND` -> Daisy `GND`

Keep `Serial1` reserved for MIDI only.

Important:

- both boards are `3.3V` logic
- do not connect `5V` logic into either board
- do not use USB serial for the Daisy MIDI input in this POC

## 5. Arduino IDE Setup

This project has been compiled locally against the installed Teensy board package `teensy:avr 1.59.0`.

Recommended Arduino IDE settings:

- Board: `Teensy 4.0`
- USB Type: `Serial`
- CPU Speed: default is fine for this controller

Open the sketch folder:

- [Seed_MIDI_Grainlet_Teensy40](C:\Users\denko\Gemini\Antigravity\DVPE_Daisy-Visual-Programming-Environment\DaisyExamples\MyProjects\_projects\Seed_MIDI_Grainlet\teensy_controller\Seed_MIDI_Grainlet_Teensy40)

Then open:

- [Seed_MIDI_Grainlet_Teensy40.ino](C:\Users\denko\Gemini\Antigravity\DVPE_Daisy-Visual-Programming-Environment\DaisyExamples\MyProjects\_projects\Seed_MIDI_Grainlet\teensy_controller\Seed_MIDI_Grainlet_Teensy40\Seed_MIDI_Grainlet_Teensy40.ino)

## 6. Program The Teensy

1. Connect the Teensy 4.0 to your computer over USB.
2. Open the sketch in Arduino IDE.
3. Confirm board is set to `Teensy 4.0`.
4. Confirm USB type is `Serial`.
5. Click Verify.
6. Click Upload.
7. If the Teensy Loader asks for confirmation, approve the upload.
8. Wait for the board to reboot after programming.

## 7. Open The Debug Serial Monitor

This sketch now provides debug messages over USB serial.

Open the Arduino serial monitor with:

- baud rate `115200`

On startup you should see lines similar to:

```text
Seed_MIDI_Grainlet Teensy 4.0 controller
Debug USB Serial: 115200 baud
UART MIDI Serial1: 31250 baud
A0->CC14, A1->CC15, A2->CC16, pin2->Note On/Off
```

When you move a pot, you should see messages like:

```text
CC 14 -> 73
CC 15 -> 101
CC 16 -> 40
```

When you press and release the button, you should see:

```text
NoteOn note=60 velocity=100
NoteOff note=60 velocity=0
```

## 8. Expected Daisy Behavior

With the Daisy Seed running [Seed_MIDI_Grainlet.cpp](C:\Users\denko\Gemini\Antigravity\DVPE_Daisy-Visual-Programming-Environment\DaisyExamples\MyProjects\_projects\Seed_MIDI_Grainlet\Seed_MIDI_Grainlet.cpp):

- pressing the button should trigger a note
- releasing the button should release the note
- moving `A0` should change Grainlet shape
- moving `A1` should change formant frequency
- moving `A2` should change bleed

## 9. Troubleshooting

If the serial monitor shows debug messages but Daisy does not respond:

- recheck `TX1 -> D14`
- recheck ground between Teensy and Daisy
- confirm the Daisy firmware is the `Seed_MIDI_Grainlet` project
- confirm the Daisy build flashed successfully

If Daisy responds but debug monitor is empty:

- confirm Arduino serial monitor is open at `115200`
- confirm USB type is set to `Serial`
- reconnect USB after upload if needed

If pots seem noisy:

- use short wires
- make sure pot grounds are solid
- the sketch already filters tiny CC changes, but poor wiring can still cause chatter
