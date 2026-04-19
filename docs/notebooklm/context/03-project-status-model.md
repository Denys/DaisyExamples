# Project Status Model

Use these buckets when describing projects in DaisyBrain or local context docs.

## Foundational Workspace

Definition:

- first-party project or library with durable strategic importance to the repo

Examples:

- `DaisyHost/`
- `DaisyDAFX/`
- `pedal/`
- `DAISY_QAE/`

## Active Verified

Definition:

- recently touched and explicitly build/test verified

Typical evidence:

- recent `README.md` or `CHECKPOINT.md` verification note
- passing host tests or clean firmware rebuild recorded in local docs

## Active Iterating

Definition:

- currently important and recently touched, but not fully stabilized or not yet
  fully verified end-to-end

Typical evidence:

- recent docs and source changes
- bridge or scaffolding language in README/docs
- known manual gaps

## Working Reusable

Definition:

- useful project or template that is not the current center of work but remains
  a known-good reference or scaffold

Examples:

- templates
- verified control-architecture references
- proven host or firmware helpers

## Concept / Portfolio

Definition:

- legitimate project direction or exploratory implementation that contributes
  ideas, control schemes, or DSP experiments, but should not be assumed fully
  current or build-verified without checking local docs

This is the largest bucket in `MyProjects/_projects/`.

## Experiment / Scratch

Definition:

- narrow experiments, tests, or throwaway probes
- useful for learning or reference, not for direct production assumptions

Typical locations:

- `MyProjects/_experiments/`
- obviously provisional folders

## Transitional / Legacy

Definition:

- old copies, bridges, or superseded structures retained for compatibility or
  migration history

Examples:

- `MyProjects/DAFX_2_Daisy_lib/`
- older bridge projects that now have more canonical replacements

## Finished But Not Central

Definition:

- completed enough to reuse or learn from, but not currently the main strategic
  focus

This is useful for projects that are "done enough" without being promoted to a
foundational workspace.

## Often Used

Definition:

- not a formal status bucket, but a frequency tag
- use it for templates, references, and known-good examples that agents are
  likely to revisit often

Examples:

- `Pod_MultiFX_Chain`
- `Field_Template_*`
- `DaisyHost/`
- `DAISY_QAE/`

The point of this model is to avoid binary thinking such as "working vs not
working." Most of the repo is better described by confidence, recency, and
reuse value.
