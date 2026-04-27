import importlib.util
import pathlib
import sys
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[1]
MODULE_PATH = ROOT / "tools" / "suggest_next_wp.py"


def load_module():
    spec = importlib.util.spec_from_file_location("suggest_next_wp", MODULE_PATH)
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


class NextWpSuggesterTest(unittest.TestCase):
    def test_prefers_first_ready_parallel_start_after_completed_work(self):
        module = load_module()
        tracker = """
## Product Value Workstreams

| ID | Workstream | What it unlocks | Depends on | Parallel-safe with | Percent complete | Status |
|---|---|---|---|---|---|---|
| `WS10` | External state / debug surface | Debug tooling | **First slice implemented:** additive CLI debugState exists. | `TF11` | `25%` | First slice implemented |
| `WS11` | Hub scenarios | Scenario launch | **Blocked / not ready:** board-generic expectations remain partial. | `TF9` | `0%` | Planned after TF9 |

## Technical Foundation Workstreams

| ID | Workstream | What it unlocks | Depends on | Parallel-safe with | Percent complete | Status |
|---|---|---|---|---|---|---|
| `TF9` | Board-generic editor surface | Removes Patch assumptions | **Good to go:** board seam exists. | `TF8` | `35%` | First cleanup implemented |
| `TF14` | CLI gate diagnostics | Structured gate evidence | **Ready after TF13:** activity-log exists. | `TF15` | `0%` | Essential; planned |

## Recommended Parallel Start

Start these first if staffing exists:

- `TF9`
- `TF14` / `TF15` only after TF13 activity-log evidence is mirrored
"""

        recommendation = module.recommend_next_work_package(tracker)

        self.assertEqual(recommendation.recommended.id, "TF9")
        self.assertEqual(recommendation.runner_up.id, "TF14")
        self.assertIn("Good to go", recommendation.recommended.dependency_status)
        self.assertIn("Low", recommendation.overlap_risk)

    def test_live_tracker_has_decision_ready_recommendation(self):
        module = load_module()
        tracker = (ROOT / "WORKSTREAM_TRACKER.md").read_text(encoding="utf-8")

        recommendation = module.recommend_next_work_package(tracker)

        self.assertTrue(recommendation.recommended.id)
        self.assertNotEqual(recommendation.recommended.percent_complete, 100)
        self.assertNotIn("Blocked / not ready",
                         recommendation.recommended.dependency_status)
        self.assertTrue(recommendation.first_safe_slice)
        self.assertTrue(recommendation.overlap_risk)


if __name__ == "__main__":
    unittest.main()
