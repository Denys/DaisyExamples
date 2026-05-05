from pathlib import Path


PROJECT = Path(__file__).resolve().parents[1]


def read_text(name: str) -> str:
    return (PROJECT / name).read_text(encoding="utf-8")


def test_project_files_exist():
    for name in ("field_mixer.cpp", "Makefile", "README.md", "CONTROLS.md"):
        assert (PROJECT / name).is_file(), name


def test_makefile_targets_field_mixer_source():
    makefile = read_text("Makefile")

    assert "TARGET = field_mixer" in makefile
    assert "CPP_SOURCES = field_mixer.cpp" in makefile
    assert "LIBDAISY_DIR = ../../../libDaisy" in makefile
    assert "DAISYSP_DIR  = ../../../DaisySP" in makefile


def test_controls_document_confirmed_field_layout():
    controls = read_text("CONTROLS.md")

    for phrase in (
        "Hydra uses physical IN L",
        "Edge uses physical IN R",
        "K1 | Hydra level",
        "K2 | Edge level",
        "K5 | Hydra delay send",
        "K6 | Edge delay send",
        "A1 | Hydra mute",
        "A2 | Edge mute",
        "B1 | Delay freeze",
        "B5 | Scene 1: clean mix",
    ):
        assert phrase in controls


def test_source_has_no_pan_or_stereo_features():
    source = read_text("field_mixer.cpp").lower()

    for forbidden in ("pan", "widen", "stereo"):
        assert forbidden not in source


def test_headphones_receive_common_mix():
    source = read_text("field_mixer.cpp")
    readme = read_text("README.md")
    controls = read_text("CONTROLS.md")

    assert "WriteCommonOutputs" in source
    assert "headphone output carries the same common mix" in readme
    assert "Headphone output carries the same common mix" in controls


def test_no_input_startup_has_idle_guard():
    source = read_text("field_mixer.cpp")
    controls = read_text("CONTROLS.md")

    assert "InputGateState" in source
    assert "ApplyInputGate" in source
    assert "Input gate threshold" in controls
    assert "no-input hum" in controls
