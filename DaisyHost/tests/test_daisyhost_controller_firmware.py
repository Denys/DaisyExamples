from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PROJECT = ROOT.parent / "field" / "DaisyHostController"


def test_daisyhost_controller_firmware_contract():
    makefile = (PROJECT / "Makefile").read_text(encoding="utf-8")
    source = (PROJECT / "DaisyHostController.cpp").read_text(encoding="utf-8")
    readme = (PROJECT / "README.md").read_text(encoding="utf-8")
    controls = (PROJECT / "CONTROLS.md").read_text(encoding="utf-8")

    assert "TARGET = DaisyHostController" in makefile
    assert "CPP_SOURCES = DaisyHostController.cpp" in makefile
    assert "LIBDAISY_DIR = ../../libDaisy" in makefile
    assert "DAISYSP_DIR = ../../DaisySP" in makefile

    assert '#include "daisy_field.h"' in source
    assert '#include "hid/midi.h"' in source
    assert "MidiUsbHandler usbMidi;" in source
    assert "usbMidi.Init" in source
    assert "usbMidi.SendMessage" in source
    assert "constexpr uint8_t kKnobCcBase = 20;" in source
    assert "constexpr uint8_t kCvCcBase = 28;" in source
    assert "constexpr uint8_t kSwitchCcBase = 80;" in source
    assert "constexpr uint8_t kKeyNoteBase = 60;" in source
    assert "hw.ProcessAllControls();" in source
    assert "AudioCallback" not in source

    assert "USB MIDI controller" in readme
    assert "DaisyHost" in readme
    assert "make program" in readme

    assert "| K1-K8 | CC 20-27 |" in controls
    assert "| CV1-CV4 | CC 28-31 |" in controls
    assert "| A1-B8 | Note 60-75 |" in controls
    assert "| SW1-SW2 | CC 80-81 |" in controls
