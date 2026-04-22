# AGENTS.md

Repo-level instructions for coding agents working in `DaisyExamples/`.

This file is intentionally narrow. It captures durable repo workflow, verification,
and safety rules. For volatile current-project context, read `LATEST_PROJECTS.md`.
For project-specific doctrine, follow the nearest local docs such as `README.md`,
`CHECKPOINT.md`, `CONTROLS.md`, `DAISY_QAE/*`, or a dated plan in `docs/plans/`.

Thin tool-specific wrappers such as `CODEX.md`, `CLAUDE.md`, `CHATGPT.md`,
`GEMINI.md`, `OPENCODE.md`, and `KILO.md` should defer to this file plus
`LATEST_PROJECTS.md` rather than duplicating repo workflow.
Keep wrapper files thin: they may add tool-specific invocation details, but
they should not restate repo workflow that already lives here or in
`LATEST_PROJECTS.md`.

## Scope

- Default assumption: this repo is a mixed workspace containing upstream Daisy
  examples, local custom firmware projects, submodules, generated build output,
  and auxiliary tooling.
- Prefer the smallest possible change set. Do not normalize unrelated examples
  or "clean up" the workspace unless the task explicitly asks for it.
- If a task spans more than one pinned workspace root, name the primary root
  and dependent roots before editing, then validate the smallest relevant
  target in each affected surface.

## Instruction Priority

When instructions conflict, resolve by descending priority:

1. Explicit user instruction in the current session
2. Nearest local `AGENTS.md` for the target subproject
3. This root `AGENTS.md`
4. Nearest local docs such as `README.md`, `CHECKPOINT.md`, and `CONTROLS.md`
5. `LATEST_PROJECTS.md` as volatile orientation, not authority
6. `docs/plans/` as design context, not implementation proof
7. DaisyBrain as strategic memory only; it does not override local source of
   truth

## Start Here

1. Identify the exact target directory before editing.
2. If the target is not obvious, inspect the pinned workspace roots below and
   optionally run `py -3 ./ci/list_recent_project_roots.py` before assuming the
   active work lives only under `MyProjects/_projects`.
3. Read the nearest `README.md`.
4. If a sibling or parent `CHECKPOINT.md` exists, read it before changing code.
   If no `CHECKPOINT.md` exists, proceed and note the missing milestone context.
5. If the target is part of the recent custom-project working set, read the
   matching entry in `LATEST_PROJECTS.md`.
   If `LATEST_PROJECTS.md` is missing or clearly stale for the target, treat it
   as orientation only and fall back to local docs plus
   `py -3 ./ci/list_recent_project_roots.py`.
6. If the target project has a matching plan in `docs/plans/`, use it as design
   context but do not treat it as proof that implementation is current.
7. If the task changes control architecture, project scaffolding, or Daisy coding
   patterns, consult `DAISY_QAE/DAISY_DEVELOPMENT_STANDARDS.md` and related QAE
   docs before inventing a new workflow.
8. If current repo state, cross-project strategy, or recent architecture context
   is still unclear after local docs, consult DaisyBrain using
   `docs/notebooklm/README.md`, the repo-local
   `docs/notebooklm/daisybrain.ps1` bridge, and the layered context pack under
   `docs/notebooklm/context/`. Treat DaisyBrain as a strategic memory edge, not
   a substitute for local source-of-truth docs.
9. If DaisyBrain or its local CLI path is unavailable, continue with the
   nearest local docs plus `AGENTS.md` and `LATEST_PROJECTS.md`, and state that
   the strategic-memory path could not be used.

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
- `DaisyHost/`: host-side JUCE/CMake workspace for the virtual Daisy Patch
  plugin and standalone app. Treat it as a first-party project with its own
  local `README.md`, `AGENTS.md`, `CHECKPOINT.md`, and `CHANGELOG.md`.
- `DaisyDAFX/`: canonical library-style workspace with its own CMake, CTest,
  and Doxygen reference flow. Do not assume the default Make-based firmware
  workflow there.
- `MyProjects/DAFX_2_Daisy_lib/`: deprecated transitional copy of the old DAFX
  library workspace. Prefer `DaisyDAFX/` when the two differ.

## Pinned Workspace Roots

These roots are first-party work areas and must be considered before narrowing
attention to a smaller recent-project list:

- `DaisyDAFX/`
- `DaisyHost/`
- `pedal/`
- `DAISY_QAE/`
- `MyProjects/_projects/`

When refreshing `LATEST_PROJECTS.md`, do not derive "recent" from a single
subtree. Use repo-wide candidate discovery and file-level modification times.

## Ignore By Default

Do not use these directories as primary evidence unless the task is explicitly
about them:

- `.worktrees/`
- `.tmp/`
- `dist/`
- `build/`
- `__pycache__/`
- nested `node_modules/`

Also, when ranking recency, ignore generated output inside these directories so
compiled artifacts do not overshadow source/docs activity.

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

### DaisyDAFX validation

`DaisyDAFX/` is a host-side library workflow, not a board-example workflow.
When `DaisyDAFX/` changes, prefer:

```sh
cmake -S DaisyDAFX -B DaisyDAFX/build -DBUILD_EXAMPLES=OFF
cmake --build DaisyDAFX/build --config Release --target unit_tests
ctest --test-dir DaisyDAFX/build -C Release --output-on-failure
```

If the task involves its generated API reference, use the local rebuild helper
from `DaisyDAFX/util/` rather than inventing a new doc flow.

### DaisyHost validation

`DaisyHost/` is a host-side JUCE/CMake workflow. When `DaisyHost/` changes,
prefer:

```sh
cmake -S DaisyHost -B DaisyHost/build
cmake --build DaisyHost/build --config Release --target unit_tests DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir DaisyHost/build -C Release --output-on-failure
```

When the Patch firmware adapter changes, also rebuild:

```sh
make
```

from `patch/MultiDelay/`.

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

Adjust the relative path as needed.
Treat any validator `ERROR` output or non-zero exit as failed validation.
Surface `WARN` output if present; warnings are advisory unless the user says
otherwise.
If you cannot run the validator, say so.

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
- For `DaisyHost/`, read the local `README.md`, `AGENTS.md`, `CHECKPOINT.md`,
  and `CHANGELOG.md` before editing. The workspace is parallelized; respect the
  local ownership slices when multiple agents are active.
- For `DaisyDAFX/`, prefer the root library copy over
  `MyProjects/DAFX_2_Daisy_lib/`, and treat the `MyProjects` copy as archival or
  compatibility-only unless the user explicitly asks to update it.
- Use DaisyBrain when you need repo-wide memory, strategy recall, or a quick
  synthesis across multiple workspaces, but always confirm actionable details
  against the nearest repo/local docs before editing.
- If hardware validation is needed but unavailable, state that the result is
  `build-verified only - hardware validation not performed`.
