# System reference

Version 0.2.0-dev. Each Markdown chapter includes its matching editable `.mmd` source.

- [Context](01_context.md)
- [Firmware Ownership](02_firmware_ownership.md)
- [Synthesis](03_synthesis.md)
- [Clock Transport](04_clock_transport.md)
- [Controls Menu](05_controls_menu.md)
- [Field Hardware](06_field_hardware.md)
- [Preset Transaction](07_preset_transaction.md)
- [Verification](08_verification.md)

These diagrams describe the delivered candidate. Dashed deferred paths are not implemented.
Use `tools/validate_repo.py` to check source/fence parity. Real Mermaid rendering is a separate
Codex check: run an installed `mmdc` against each `.mmd` file, preserving its version and errors.
