# Pod MultiDelay v0.4 controls

The encoder turns through DIGI / TAPE / MOD / REV / FREEZE. Clicking cycles
TIME/FEEDBACK -> MIX/COLOR -> MOTION/unused -> DIST -> TIME/FEEDBACK.
DIST is an editing page, not another delay engine. Entering, leaving or editing
DIST never changes distortion enable, delay bypass, topology or stored tails.

| Control | DIST page | Other pages |
|---|---|---|
| K1 | Drive 0..100%, default 40% | Existing delay assignment |
| K2 | Tone 0..100%, default 50% | Existing delay assignment |
| B1 without B2 | Toggle distortion ON/OFF | Toggle delay bypass |
| Encoder click | Leave DIST, preserve ON/OFF | Select next page |
| Encoder turn | Select delay mode, preserve ON/OFF | Select delay mode |
| B2 | Existing tap/capture/SHIFT gestures | Existing gestures |

The two DIST pots retain their ownership while B2 is held. Shift+B1 still relinks
DIGI feedback or clears FREEZE; it does not toggle distortion. Pots use the
existing 0.012 movement gate on entry/exit. DIST settings and ON/OFF survive
page/mode changes during the session; reboot restores OFF / 40% / 50%.

LED2 is yellow on DIST: dim when OFF, bright when ON. LED1 retains the delay
mode colour and pulses while distortion is ON, including outside DIST. Physical
colour, gesture and audio validation remain separate from host/build evidence.

PRE distortion feeds both dry and delay send. Delay bypass does not disable it;
mode reconfiguration does not interrupt it. Enable ramps over 240 samples (5 ms
at 48 kHz); Drive and Tone are smoothed. The cubic soft clip uses the shaping
law of the first-party Field drive, with a 1..30 pre-gain and a 1..12 kHz tone
low-pass. This bounded Pod stage has no oversampling or fixed alignment delay.
It makes no aliasing, pop-free, timing or analog-emulation claim.
