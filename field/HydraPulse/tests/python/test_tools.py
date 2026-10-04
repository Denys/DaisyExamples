"""Deterministic positive and negative tests; all mutation uses temporary fixtures."""
import copy
import json
from pathlib import Path
import re
import shutil
import sys
import tempfile
import unittest
from unittest import mock

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import common
import generate_presets
import install_additive
import parse_telemetry
import validate_evidence
import validate_repo
import verify_dependencies
import build_target

class PresetTests(unittest.TestCase):
    def setUp(self):
        self.data = json.loads((ROOT / "presets/tutorials.json").read_text())
    def test_generated_table_matches(self):
        self.assertEqual(generate_presets.generate(self.data),
                         (ROOT / "src/app/Presets.generated.h").read_text())
    def test_nan_parameter_rejected(self):
        self.data["presets"][1]["voices"][0]["tune"] = float("nan")
        with self.assertRaises(ValueError): generate_presets.generate(self.data)
    def test_infinite_tempo_rejected(self):
        self.data["presets"][1]["bpm"] = float("inf")
        with self.assertRaises(ValueError): generate_presets.generate(self.data)
    def test_boolean_parameter_rejected(self):
        self.data["presets"][0]["voices"][0]["level"] = True
        with self.assertRaises(ValueError): generate_presets.generate(self.data)
    def test_bad_bank_shape_rejected(self):
        self.data["presets"][1]["notes"][0].pop()
        with self.assertRaises(ValueError): generate_presets.generate(self.data)
    def test_accent_without_note_rejected(self):
        self.data["presets"][0]["accents"][0][0] = 1
        with self.assertRaises(ValueError): generate_presets.generate(self.data)
    def test_duplicate_identity_rejected(self):
        self.data["presets"][3]["id"] = 2
        with self.assertRaises(ValueError): generate_presets.generate(self.data)

class TelemetryTests(unittest.TestCase):
    def test_key_and_new_split_record_kinds(self):
        for text in ("[HPF] key raw=15 down=1 cb=45",
                     "[HPF] system board=2 memory=0 loader=0",
                     "[HPF] outputs dac1=1024 dac2=4095 enabled=1",
                     "[HPF] queues mdrops=0 sdrops=0"):
            self.assertIsNotNone(parse_telemetry.parse_line(text))
    def test_nontelemetry_ignored(self):
        self.assertIsNone(parse_telemetry.parse_line("USB enumerated"))
    def test_duplicate_field_rejected(self):
        with self.assertRaises(ValueError):
            parse_telemetry.parse_line("[HPF] key raw=0 raw=1 down=1")
    def test_key_outside_hardware_rejected(self):
        with self.assertRaises(ValueError):
            parse_telemetry.parse_line("[HPF] key raw=16 down=1")
    def test_unknown_record_rejected(self):
        with self.assertRaises(ValueError):
            parse_telemetry.parse_line("[HPF] measured_volts pass=1")
    def test_summary_preserves_invalid_status(self):
        s = parse_telemetry.summarize(["[HPF] key raw=2 down=1\n",
             "[HPF] timing over=0 late=1\n", "[HPF] key raw=99 down=1\n"])
        self.assertEqual(s["classification"], "DERIVED")
        self.assertEqual(s["invalid_records"], 1)
        self.assertEqual(s["pressed_raw_indices"], [2])
        self.assertIn("cold boot", s["not_proven"])
    def test_logger_buffer_maximum_widths(self):
        # Analyze the actual PrintLine format strings, not a separate copied list.
        sources = "\n".join((ROOT / f).read_text() for f in (
            "firmware/FieldAdapter.h", "firmware/FieldTruth.cpp", "firmware/HydraPulse.cpp"))
        formats = re.findall(r'PrintLine\(\s*"([^"]*)"', sources)
        self.assertGreaterEqual(len(formats), 14)
        token = re.compile(r'%(\.\d+)?(l)?([dus])')
        for fmt in formats:
            def replace(m):
                if m[3] == "s":
                    self.assertIsNotNone(m[1], "unbounded logger string")
                    return "S" * int(m[1][1:])
                return "-2147483648" if m[3] == "d" else "4294967295"
            worst = token.sub(replace, fmt)
            self.assertNotIn("%", worst, "unhandled format specifier")
            self.assertLessEqual(len(worst.encode()) + 3, 128, worst)

class EvidenceTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.base = Path(self.temp.name)
        (self.base / "trace.csv").write_text("sample,value\n0,0\n")
        self.record = {"id":"P0-TEST", "classification":"VERIFIED", "domain":"hardware",
            "claim":"Fixture test only", "fixture":"temporary fixture", "operator":"test runner",
            "utc":"2026-09-07T00:00:00Z", "method":"synthetic validator fixture; not real P0 evidence",
            "result":"schema test", "firmware_sha256":"a"*64,
            "artifacts":[{"path":"trace.csv","sha256":common.sha256(self.base/"trace.csv")}]}
    def validate(self, record=None):
        return validate_evidence.validate({"schema_version":1,
            "records":[self.record if record is None else record]}, self.base)
    def test_not_run_template_is_valid_metadata(self):
        self.assertEqual(validate_evidence.validate(
            json.loads((ROOT/"hardware/p0_record.template.json").read_text()),
            ROOT/"hardware"), [])
    def test_complete_synthetic_record_is_valid_metadata(self):
        self.assertEqual(self.validate(), [])
    def test_missing_hardware_fixture_rejected(self):
        self.record.pop("fixture")
        self.assertTrue(self.validate())
    def test_invalid_utc_rejected(self):
        self.record["utc"] = "yesterday"
        self.assertTrue(self.validate())
    def test_missing_artifact_rejected(self):
        self.record["artifacts"] = []
        self.assertTrue(self.validate())
    def test_modified_raw_artifact_rejected(self):
        (self.base/"trace.csv").write_text("changed\n")
        self.assertTrue(self.validate())
    def test_unsafe_raw_artifact_path_rejected(self):
        self.record["artifacts"][0]["path"] = "../trace.csv"
        self.assertTrue(self.validate())
    def test_nonfinite_measurement_rejected(self):
        self.record["measurements"] = {"rms":float("nan")}
        self.assertTrue(self.validate())
    def test_bad_type_and_duplicate_records_rejected(self):
        self.record["classification"] = []
        self.assertTrue(self.validate())
        self.assertTrue(validate_evidence.validate([], self.base))
        self.record["classification"] = "NOT_RUN"
        self.assertTrue(validate_evidence.validate(
            {"schema_version":1,"records":[self.record, self.record]}, self.base))

class InstallTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.base = Path(self.temp.name)
        self.source, self.dest = self.base/"source", self.base/"destination"
        self.source.mkdir(); self.dest.mkdir()
        self.name = "field/HydraPulse/example.txt"
        p=self.source/self.name; p.parent.mkdir(parents=True); p.write_text("candidate\n")
        self.records={self.name: common.sha256(p)}
        (self.dest/"unrelated.txt").write_text("retain me\n")
    def test_addition_and_idempotence_preserve_unrelated(self):
        result=install_additive.compare(self.source, self.dest, self.records)
        self.assertEqual(result["new"], [self.name])
        install_additive.apply(self.source, self.dest, result)
        self.assertEqual((self.dest/"unrelated.txt").read_text(), "retain me\n")
        again=install_additive.compare(self.source, self.dest, self.records)
        self.assertEqual(again["identical"], [self.name])
        self.assertEqual(again["new"], [])
    def test_conflict_preflight_writes_nothing(self):
        p=self.dest/self.name; p.parent.mkdir(parents=True); p.write_text("remote change\n")
        second="field/HydraPulse/second.txt"
        (self.source/second).write_text("new\n")
        self.records[second]=common.sha256(self.source/second)
        result=install_additive.compare(self.source, self.dest, self.records)
        with self.assertRaises(ValueError): install_additive.apply(self.source, self.dest, result)
        self.assertFalse((self.dest/second).exists())
        self.assertEqual(p.read_text(), "remote change\n")
    def test_destination_symlink_is_conflict(self):
        (self.dest/"field").symlink_to(self.source/"field", target_is_directory=True)
        result=install_additive.compare(self.source, self.dest, self.records)
        self.assertEqual(result["conflicts"], [self.name])
    def test_parent_file_is_conflict(self):
        (self.dest/"field").write_text("not a directory")
        self.assertEqual(install_additive.compare(self.source,self.dest,self.records)["conflicts"],[self.name])
    def test_inventory_hash_mismatch_rejected(self):
        manifest=self.source/install_additive.MANIFEST
        manifest.write_text(json.dumps({"files":{self.name:"0"*64}}))
        with self.assertRaises(ValueError): install_additive.inventory(self.source)
    def test_inventory_out_of_scope_rejected(self):
        manifest=self.source/install_additive.MANIFEST
        manifest.write_text(json.dumps({"files":{"README.md":"0"*64}}))
        with self.assertRaises(ValueError): install_additive.inventory(self.source)
    def test_source_symlink_and_traversal_rejected(self):
        (self.source/"alias").symlink_to(self.source/"field", target_is_directory=True)
        for name in ("../x", "/absolute", "C:\\x", "alias/HydraPulse/example.txt", 123):
            with self.subTest(name=name):
                with self.assertRaises(ValueError): common.safe_file(self.source, name)

class DependencyAndRepoTests(unittest.TestCase):
    def test_missing_toolchain_fails_loudly(self):
        with mock.patch.object(build_target.shutil, "which", return_value=None):
            with self.assertRaisesRegex(ValueError,"missing tools"): build_target.require_tools()
    def test_short_dependency_pin_rejected(self):
        lock=json.loads((ROOT/"deps.lock.json").read_text())
        lock["dependencies"][0]["ref"]="cc146d5"
        with self.assertRaisesRegex(ValueError,"full commit"): verify_dependencies.verify(ROOT,lock)
    def test_wrong_real_checkout_head_rejected(self):
        lock=json.loads((ROOT/"deps.lock.json").read_text())
        with mock.patch.object(verify_dependencies, "git", return_value="0"*40):
            with self.assertRaisesRegex(ValueError,"HEAD mismatch"):
                verify_dependencies.verify(ROOT,lock)
    def test_uninitialized_gitlink_rejected(self):
        lock=json.loads((ROOT/"deps.lock.json").read_text())
        dep=lock["dependencies"][0]
        responses=iter([dep["ref"], "", dep["license_blob_sha1"], "-"+"a"*40+" child"])
        with mock.patch.object(verify_dependencies, "git", side_effect=lambda *a: next(responses)):
            with self.assertRaisesRegex(ValueError,"uninitialized"):
                verify_dependencies.verify(ROOT,lock)
    def test_repository_and_preserved_archive(self):
        self.assertEqual(validate_repo.validate(ROOT), [])
    def test_missing_required_readme_is_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            fixture=Path(d)/"project"
            shutil.copytree(ROOT,fixture,ignore=shutil.ignore_patterns("__pycache__",".deps","build*","artifacts"))
            (fixture/"README.md").unlink()
            self.assertTrue(any("README.md" in x for x in validate_repo.validate(fixture)))
    def test_corrupted_preserved_r0_is_rejected(self):
        with tempfile.TemporaryDirectory() as d:
            fixture=Path(d)/"project"
            shutil.copytree(ROOT,fixture,ignore=shutil.ignore_patterns("__pycache__",".deps","build*","artifacts"))
            (fixture/"references/r0/README.md").write_text("changed")
            self.assertTrue(any("R0 byte mismatch" in x for x in validate_repo.validate(fixture)))
    def test_source_fingerprint_is_sorted_and_stable(self):
        values=common.runtime_sources(ROOT)
        self.assertEqual(list(values), sorted(values))
        self.assertEqual(common.manifest_digest(values), common.manifest_digest(dict(reversed(list(values.items())))))

if __name__ == "__main__":
    unittest.main()
