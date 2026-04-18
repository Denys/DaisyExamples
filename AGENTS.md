# AGENTS.md

Repo-level instructions for coding agents working in `DaisyExamples/`.

This file is intentionally narrow. It captures durable repo workflow, verification,
and safety rules. For project-specific doctrine, follow the nearest local docs
such as `README.md`, `CHECKPOINT.md`, `CONTROLS.md`, `DAISY_QAE/*`, or a dated
plan in `docs/plans/`.

## Scope

- Default assumption: this repo is a mixed workspace containing upstream Daisy
  examples, local custom firmware projects, submodules, generated build output,
  and auxiliary tooling.
- Prefer the smallest possible change set. Do not normalize unrelated examples
  or "clean up" the workspace unless the task explicitly asks for it.

## Start Here

1. Identify the exact target directory before editing.
2. Read the nearest `README.md`.
3. If a sibling or parent `CHECKPOINT.md` exists, read it before changing code.
4. If the target project has a matching plan in `docs/plans/`, use it as design
   context but do not treat it as proof that implementation is current.
5. If the task changes control architecture, project scaffolding, or Daisy coding
   patterns, consult `DAISY_QAE/DAISY_DEVELOPMENT_STANDARDS.md` and related QAE
   docs before inventing a new workflow.

## Repo Map

- `seed/`, `pod/`, `field/`, `patch/`, `patch_sm/`, `pedal/`, `petal/`,
  `versio/`, `legio/`: upstream Make-based Daisy examples.
- `MyProjects/_projects/` and `MyProjects/_experiments/`: local custom Daisy
  firmware. These are often the real working area and are not covered by the
  repo-wide example build script.
- `libDaisy/` and `DaisySP/`: git submodules. Treat them as external libraries
  unless the task explicitly requires library changes.
- `DAISY_QAE/`: local Daisy quality/docs/tooling hub, including
  `validate_daisy_code.py`, standards, bug logs, and scaffolding references.
- `docs/plans/`: dated implementation/design notes that often capture recent
  project intent.
- `DaisyDAFX/` and `MyProjects/DAFX_2_Daisy_lib/`: separate library-style
  workspaces with their own docs and host-side test/build flows. Do not assume
  the default Make-based firmware workflow there.

## Ignore By Default

Do not use these directories as primary evidence unless the task is explicitly
about them:

- `.worktrees/`
- `.tmp/`
- `dist/`
- `build/`
- `__pycache__/`
- nested `node_modules/`

## Safety Rules

- Expect a dirty worktree. Preserve unrelated edits.
- Do not reset, clean, or rewrite submodules casually.
- Never run `git clean`, `git reset --hard`, or recursive submodule update
  commands unless the user explicitly asks for them.
- Do not regenerate `dist/` unless the task is specifically about distributable
  binaries.
- Flashing hardware is opt-in. Only run `make program` or `make program-dfu`
  when the user asks for flashing.

## Build And Verification

Prefer targeted validation over repo-wide validation.

### Default firmware validation

From the specific project directory:

```sh
make
```

Use `make clean && make` only when a clean rebuild is needed to confirm the
change or debug stale artifacts.

### When libraries changed

If you edit `libDaisy/` or `DaisySP/`, rebuild the libraries first:

```sh
./ci/build_libs.sh
```

Then rebuild at least one affected firmware target.

### Repo-wide example validation

For broad validation of upstream board examples only:

```sh
py -3 ./ci/build_examples.py
```

Important: `ci/build_examples.py` excludes `MyProjects/`, so it does not prove
custom projects still build.

### Daisy QAE validation

If the target project follows the local QAE workflow and the validator path is
reachable from that project, run:

```sh
py -3 ../../../DAISY_QAE/validate_daisy_code.py .
```

Adjust the relative path as needed. If you cannot run the validator, say so.

### Style

- C/C++ style follows `.clang-format`.
- CI style tooling excludes `DaisySP`, `libDaisy`, `cube`, `utils`, `resources`,
  and experimental example folders. Do not mass-format excluded trees unless the
  task is specifically about those areas.

## Makefile And Scaffolding Conventions

- Most firmware projects are thin Make wrappers that set `TARGET`,
  `CPP_SOURCES`, `LIBDAISY_DIR`, and `DAISYSP_DIR`, then include
  `$(LIBDAISY_DIR)/core/Makefile`.
- Keep relative library paths consistent with project depth:
  top-level board examples usually use `../../libDaisy`, while many
  `MyProjects/_projects/*` targets use `../../../libDaisy`.
- For new Make-based Daisy projects, prefer the existing helper tooling instead
  of hand-writing a Makefile:

```sh
py -3 ./helper.py create <relative/project/path> --board <board>
```

- For Field-specific workflows that explicitly follow local QAE conventions,
  compare against `DAISY_QAE/create_field_project.sh` before scaffolding.
- If you introduce DaisySP LGPL modules such as `ReverbSc`, verify the project
  Makefile enables the required LGPL flag.

## Documentation Expectations

When behavior changes in a custom project, update the nearest relevant docs if
they exist:

- `README.md` for user-facing behavior and controls
- `CONTROLS.md` for control mapping or UI semantics
- `CHECKPOINT.md` for major milestone/state changes
- project bug log or QAE notes when fixing a non-trivial recurring issue

Do not create new planning/state files at repo root unless the task explicitly
calls for them. This repo already keeps most persistent state within subprojects
or `docs/plans/`.

## Good Agent Behavior In This Repo

- Be explicit about board target and project path in your reasoning and
  verification.
- Verify the smallest relevant target first.
- Treat host-side or CMake-based subprojects as exceptions; inspect their local
  docs before assuming embedded Make rules.
- If hardware validation is needed but unavailable, state that the result is
  build-verified only.
