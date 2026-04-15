# Daisy QAE Reasonable Controls Policy Design

**Date:** 2026-04-15

## Goal

Add a general QAE policy for all Daisy targets that encourages meaningful
control curves and ranges instead of blindly exposing algorithmic extremes.

## Scope

- Apply the policy to all Daisy targets:
  - Pod
  - Seed
  - Field
- Let QAE suggest:
  - `linear`
  - `exponential`
  - `log`
  - equivalent perceptual mappings when appropriate
- Encourage meaningful control ranges instead of absolute min/max defaults
- Explicitly call out interaction-driven instability and unusable control zones
- Add a short reminder to the common-pitfalls table
- Update version/date/changelog

## Policy Intent

The policy is advisory, not absolute.

QAE should treat control design as part of system quality:

- response curve choice affects usability
- numeric range choice affects stability and musical usefulness
- parameter interactions can make a mathematically valid range practically
  unstable or unusable

## Recommended Review Guidance

QAE should suggest:

- `linear` mapping for controls that feel naturally proportional in direct
  percentage-style space
- `exponential` or `log` mapping where perceptual resolution matters, such as
  frequency, rate, gain, decay, drive, or feedback-like controls
- meaningful limits chosen around the useful operating region rather than the
  full internal range of the DSP algorithm

QAE should flag:

- all-the-action-at-one-end mappings
- dead zones that waste most of the knob travel
- ranges that invite runaway feedback, self-oscillation, clipping, zippering,
  aliasing, or abrupt mode changes unless intentionally designed
- parameter combinations that are individually valid but unstable together

## Placement

- Add one short reminder row in `Common Pitfalls`
- Add a new global section near the platform adaptation section so the policy is
  easy to find during QAE review
- Keep the existing Seed/Field banked-controls promemoria intact

## Documentation Notes

- Mention that exposing absolute minimum and maximum values is not the default
  recommendation
- State that full extremes should be justified if intentionally exposed
- Suggest documenting final mappings and ranges in `CONTROLS.md` or the project
  README
