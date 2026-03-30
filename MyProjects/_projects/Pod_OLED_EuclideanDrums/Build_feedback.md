
Here's a summary of the plan before you review it:

**Pod_OLED_EuclideanDrums** — upgrade the original 2-voice euclidean drum machine with OLED display, sequencer visualization, and expanded controls.

**Key changes from original:**

- **Sequencer home screen** — dot-grid visualization of both kick/snare patterns with live step cursor
- **Menu system** — per-drum parameter pages (density, length, decay, pitch, rotation) + global (BPM, volume, mix) + system settings
- **Control split** — Pod knobs/encoder/buttons for live performance; OLED encoder/PSH/CON/BAK for menu navigation
- **Architecture fix** — move all control/LED processing out of audio ISR into main loop
- **Zoom overlay** — pot/encoder movements show full-screen parameter popup on OLED (1.2s)
- **Beat-synced LED1** — white flash on each metro tick (replaces brightness-as-tempo)
- **New features** — tap tempo (Button2), pattern rotation, step reset (Pod encoder push)

**4 implementation phases:** audio skeleton → OLED sequencer view → menu system → polish (tap tempo, rotation, etc.)

**5 files:** `ExtEncoder.h` (copy unchanged), `DrumSeqUI.h` (new), `DrumControls.h` (new), `Pod_OLED_EuclideanDrums.cpp` (new), `Makefile`


---

Clean build, zero warnings. All 5 files complete:

|File|Purpose|Status|
|---|---|---|
|Makefile| Build config (3-level depth)|Done|
|ExtEncoder.h|OLED encoder + CON/BAK hardware wrapper|Copied|
|DrumSeqUI.h|OLED display engine (sequencer + menu + zoom)|Done|
|DrumControls.h|Control hub (pots, encoder, buttons, LEDs, patterns)|Done|
|Pod_OLED_EuclideanDrums.cpp|Main app with DSP + audio callback|Done|

**Memory usage**: FLASH 74.75%, SRAM 10% — plenty of room.

**Key improvements over original EuclideanDrums**:

- All control processing moved from audio ISR to main loop (1kHz debounce)
- Non-interleaved audio callback (modern pattern)
- VLAs replaced with fixed `[32]` arrays
- OLED sequencer home screen with live dot-grid pattern visualization + step cursor
- Full menu system: Kick/Snare/Global/System pages with 14 editable parameters
- Zoom overlay on pot/encoder turn (1.2s timeout)
- Tap tempo (Button 2, rolling average of last 4 taps)
- Drum select toggle (Button 1, LED2 green=kick/blue=snare)
- Beat-synced LED1 white flash (30ms per metro tick)
- Pattern rotation (kick + snare, editable in menu)
- Encoder push resets step counters
- Display timeout + encoder invert from System menu

Ready for `make program` to flash via ST-Link.