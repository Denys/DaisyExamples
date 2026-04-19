# DaisyExamples Repo Map And Workflow

## Identity

`DaisyExamples/` is a mixed workspace, not a single product tree. It contains:

- upstream Make-based Daisy board examples
- first-party host-side workspaces
- first-party library workspaces
- local custom firmware projects and experiments
- Daisy quality-assurance and engineering doctrine
- plans, reference docs, and imported third-party material

This means agents should not assume a single build system, a single board, or a
single "active project."

## Pinned Workspace Roots

These roots are first-party and should be considered before narrowing to a
smaller active subset:

- `DaisyHost/`
- `DaisyDAFX/`
- `pedal/`
- `DAISY_QAE/`
- `MyProjects/_projects/`

## Main Trees

- `seed/`, `pod/`, `field/`, `patch/`, `patch_sm/`, `pedal/`, `petal/`,
  `versio/`, `legio/`
  - upstream board examples
- `MyProjects/_projects/`
  - local custom firmware portfolio
- `MyProjects/_experiments/`
  - scratch and exploratory work
- `DaisyHost/`
  - host-side JUCE/CMake virtual Patch workspace
- `DaisyDAFX/`
  - canonical first-party portable DSP library workspace
- `DAISY_QAE/`
  - local standards, validators, and Daisy development guidance
- `libDaisy/`, `DaisySP/`
  - upstream submodules / platform libraries
- `third_party/`, `stmlib/`
  - external dependencies and imported engines
- `docs/plans/`
  - dated design and implementation intent

## Memory Hierarchy

Use repo memory in this order:

1. nearest local docs inside the target project
2. `AGENTS.md` for durable repo workflow
3. `LATEST_PROJECTS.md` for recent active context
4. `docs/plans/` for dated design intent
5. DaisyBrain for repo-wide synthesis and strategic recall

## Default Validation Paths

Embedded firmware target:

```sh
make
```

`DaisyHost/`:

```sh
cmake -S DaisyHost -B DaisyHost/build
cmake --build DaisyHost/build --config Release --target unit_tests DaisyHostPatch_VST3 DaisyHostPatch_Standalone
ctest --test-dir DaisyHost/build -C Release --output-on-failure
```

`DaisyDAFX/`:

```sh
cmake -S DaisyDAFX -B DaisyDAFX/build -DBUILD_EXAMPLES=OFF
cmake --build DaisyDAFX/build --config Release --target unit_tests
ctest --test-dir DaisyDAFX/build -C Release --output-on-failure
```

## Operational Rules That Matter Strategically

- expect a dirty worktree
- preserve unrelated edits
- do not casually rewrite submodules
- do not flash hardware unless explicitly asked
- prefer targeted validation over repo-wide validation

These rules are stable enough to belong in repo memory and should not be
re-explained ad hoc by each agent.
