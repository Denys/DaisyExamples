import json
import shutil
import subprocess
import sys
import uuid
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def make_work_dir(name):
    base = ROOT / ".tmp" / "field_adapter_tests" / f"{name}_{uuid.uuid4().hex}"
    if base.exists():
        shutil.rmtree(base)
    base.mkdir(parents=True)
    return base


def run_python(*args, cwd=ROOT):
    return subprocess.run(
        [sys.executable, *map(str, args)],
        cwd=cwd,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=True,
    )


def test_generator_emits_multidelay_field_adapter():
    out_dir = make_work_dir("generated") / "MultiDelayGenerated"

    run_python(
        ROOT / "tools" / "generate_field_adapter.py",
        "--spec",
        ROOT / "tools" / "adapter_specs" / "field_multidelay.json",
        "--out",
        out_dir,
    )

    makefile = (out_dir / "Makefile").read_text(encoding="utf-8")
    source = (out_dir / "MultiDelay.cpp").read_text(encoding="utf-8")
    readme = (out_dir / "README.md").read_text(encoding="utf-8")
    controls = (out_dir / "CONTROLS.md").read_text(encoding="utf-8")
    gitignore = (out_dir / ".gitignore").read_text(encoding="utf-8")

    assert "TARGET = MultiDelay" in makefile
    assert (
        "CPP_SOURCES = MultiDelay.cpp ../../DaisyHost/src/apps/MultiDelayCore.cpp"
        in makefile
    )
    assert "C_INCLUDES += -I../../DaisyHost/include" in makefile
    assert "OPT = -Os" in makefile

    assert '#include "daisy_field.h"' in source
    assert '#include "daisyhost/apps/MultiDelayCore.h"' in source
    assert "DaisyField hw;" in source
    assert 'MultiDelayCore core("node0");' in source
    assert "MultiDelayCore::DelayLineType DSY_SDRAM_BSS" in source
    assert "void AudioCallback" in source
    assert "core.Process({inputPtrs, 1}," in source
    assert "void ProcessControls()" in source
    assert "hw.ProcessAllControls();" in source
    assert 'const char* k5ParameterId = "node0/param/delay_tertiary";' in source
    assert "core.SetMenuItemValue(fireImpulseMenuItemId, 1.0f);" in source
    assert "hw.SetCvOut1(ToDacCode(knobValues[4]));" in source
    assert "hw.SetCvOut2(0);" in source
    assert "std::string" not in source

    assert "first generated Daisy Field firmware adapter" in readme
    assert "make program" in readme
    assert "flash-verified only" in readme

    assert "| K5 | Tertiary delay control |" in controls
    assert "| CV OUT 1 | Mirrors K5 as `0..5V` |" in controls
    assert "Audio input 1 reaches audio outputs 1/2 with a delay effect." in controls
    assert "build/" in gitignore


def test_audit_classifies_shared_core_adapter_as_portable_ready():
    result = run_python(
        ROOT / "tools" / "audit_firmware_portability.py",
        ROOT.parent / "field" / "MultiDelay",
        "--json",
    )
    report = json.loads(result.stdout)

    assert report["classification"] == "portable-core-ready"
    assert report["project"] == "MultiDelay"
    assert "daisyhost/apps/MultiDelayCore.h" in report["evidence"]
    assert not report["blocking_findings"]


def test_audit_reports_raw_field_firmware_needs_core_extraction():
    project = make_work_dir("raw") / "RawField"
    project.mkdir()
    (project / "RawField.cpp").write_text(
        """
#include "daisy_field.h"
#include <cstdio>

using namespace daisy;
DaisyField hw;

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    printf("callback log");
    for(size_t i = 0; i < size; ++i)
    {
        out[0][i] = in[0][i] * hw.GetKnobValue(0);
    }
}

int main()
{
    hw.Init();
    hw.StartAudio(AudioCallback);
    while(true) { hw.ProcessAllControls(); }
}
""",
        encoding="utf-8",
    )

    result = run_python(
        ROOT / "tools" / "audit_firmware_portability.py",
        project,
        "--json",
    )
    report = json.loads(result.stdout)

    assert report["classification"] == "needs-core-extraction"
    assert "missing DaisyHost shared app core include" in report["blocking_findings"]
    assert "audio callback appears to read hardware controls" in report["blocking_findings"]
    assert "audio callback appears to log or print" in report["blocking_findings"]
    assert "required_actions" in report
