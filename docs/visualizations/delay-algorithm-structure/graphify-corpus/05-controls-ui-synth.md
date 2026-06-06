# Controls, OLED, Keybed, and Internal Synth

The Field UI adapter maps physical controls to the shared delay core.

SW1 and SW2 select shifted knob layers. The adapter uses an until-touched model:
when a layer is entered, it stores the current knob values and does not write
any parameter until the physical knob moves. Shifted knob movement does not
modify the unshifted parameter.

A1-A4 select algorithms. The adapter saves the current algorithm parameter
snapshot, switches source, restores the selected algorithm snapshot if it
exists, resets the layer touch gates, and records an OLED zoom message.

A5 cycles internal synth mode: off, pluck, and pad. A6 cycles hold mode:
momentary, latch, and drone. A7 and A8 shift octave. B1-B8 trigger the internal
synth as white keys from C4 to C5, shifted by the octave offset. External MIDI
uses the same voice engine.

The keybed has a recurring mapping hazard. The observed Field scan order for
this adapter is physical B1-B8 at indices 0 through 7 and physical A1-A8 at
indices 8 through 15. LED enum order is different, so input mapping and LED
mapping must stay separate in diagrams and code.

The OLED shows the project title, active algorithm, octave, current layer values,
units, and a short zoom view after a parameter or mode changes.
