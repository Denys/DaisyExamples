# Local Apply Status

This file confirms the triple oscillator subtractive synth changes are present locally.

## Applied locally

- Main project folder exists: `MyProjects/_projects/tripple_osc_subtractive`
- Main source exists: `tripple_osc_subtractive.cpp`
- Documentation exists: `README.md`, `CONTROLS.md`, `Dependencies.md`
- Build file exists: `Makefile`

## Legacy experiment path synchronization

To avoid ambiguity from patch tools reporting a skipped path, the legacy experiment file is synchronized with the new project source:

- `MyProjects/_experiments/MyKeyboardTest/dual_osc_subtractive.cpp`

Current intent:

- `_projects/tripple_osc_subtractive` is the canonical project location.
- `_experiments/MyKeyboardTest/dual_osc_subtractive.cpp` is kept in sync for compatibility/history.

## Quick local verification commands

```bash
rg --files MyProjects/_projects/tripple_osc_subtractive
cmp -s MyProjects/_projects/tripple_osc_subtractive/tripple_osc_subtractive.cpp \
       MyProjects/_experiments/MyKeyboardTest/dual_osc_subtractive.cpp && echo "sources in sync"
```
