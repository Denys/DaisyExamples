# HydraPulse development contract

Work within `field/HydraPulse` and the additive `.github/workflows/hydrapulse.yml`.
Read parent repository instructions before changing this subtree. Do not modify sibling
examples, the root dependency gitlinks, root license, or the old HydraPulse repository
as a side effect. Use the existing root dependency at `../../libDaisy`, verified
against `deps.lock.json`. Qualification must run in an isolated clean checkout;
never force-rebuild a shared user's dependency tree with different compiler flags.

The firmware audio callback is the only writer of Engine, Controller and sampled control
state. Foreground MIDI sends fixed-size messages through SPSC queues; foreground display
and logs consume snapshots. Never introduce logging, display/I2C transfers, file I/O,
heap allocation, mutex waits or unbounded queue drains into the callback.

Keep 16 step keys and Tune / Decay / Character / Level. Changes to input gestures must
update Controller tests, the manual, control map and Mermaid sources in the same change.
Keep the two images separate: Field Truth is permanent diagnostics; Beat is an instrument
candidate. Neither host tests nor an ARM build can close the physical P0 gate.

Run the local CMake/CTest suite, Python tests, offline repository validator, preset
generation check and deterministic host render comparison. Run a real ARM build through
`tools/build_target.py` before calling an image build-verified. The adapter stub is only a
syntax/ownership aid; never package it as firmware. Preserve failures and NOT_RUN states.

No automatic hardware flashing, force push, global dependency update, commercial manual
redistribution or invented hardware evidence. A `.dvpe` project requires a real schema,
known block IDs, import/export roundtrip and generated-code tests; otherwise omit it.

The user has authorized a normal non-destructive integration into Denys/DaisyExamples.
Use the repository's existing branch/PR policy, compare every destination file, test the
actual merged tree, and report exact commits and CI runs. Ask only for material conflicts,
destructive replacement, credential actions, public-release licensing, or physical flashing.
