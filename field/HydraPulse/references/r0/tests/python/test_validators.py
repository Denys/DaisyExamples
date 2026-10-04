import subprocess
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


class ValidatorTests(unittest.TestCase):
    def run_tool(self, relative_path: str) -> None:
        result = subprocess.run(
            [sys.executable, str(ROOT / relative_path)],
            cwd=ROOT,
            capture_output=True,
            text=True,
            check=False,
        )
        self.assertEqual(
            result.returncode,
            0,
            msg=f"{relative_path} failed\nstdout:\n{result.stdout}\nstderr:\n{result.stderr}",
        )

    def test_repository_structure(self):
        self.run_tool("tools/validate_repo.py")

    def test_provenance(self):
        self.run_tool("tools/validate_provenance.py")


if __name__ == "__main__":
    unittest.main()
