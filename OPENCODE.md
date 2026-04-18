# OPENCODE.md

Read `AGENTS.md` first, then `LATEST_PROJECTS.md`.

Repo-specific priorities:

- Work from the exact target directory.
- Read the nearest `README.md`, `CHECKPOINT.md`, and `CONTROLS.md` before
  editing behavior.
- Prefer targeted validation such as local `make`, project tests, or QAE lint
  over repo-wide builds.
- Do not treat `ci/build_examples.py` as evidence for `MyProjects/*`.
- Treat `libDaisy/` and `DaisySP/` as submodules, not cleanup targets.

If the task touches a recent custom project, start from its entry in
`LATEST_PROJECTS.md`.
