# Historical supplied candidate validation

This is the original 2026-09-07 authoring snapshot. Current integration/build/hardware
results are in [INTEGRATION_2026-09-08.md](INTEGRATION_2026-09-08.md).

Date: **2026-09-07**. Candidate: **HydraPulse Field 0.2.0**.
Intended destination: `Denys/DaisyExamples/field/HydraPulse`.

**Result:** implemented and host-tested source candidate. It is not yet an
ARM-linked, ready-to-flash deliverable, a measured Field instrument, or a
GitHub-published update.

Runtime-source SHA-256:

```text
7aa39d796ad68faea522189185c2f451230aa92a77b5996aa73af942c78d41f8
```

This is a source-content fingerprint, **not a remote commit SHA**. Its exact
file basis is preserved in `evidence/local/results.json`. The final package
manifest additionally binds documentation, tests and all other payload files.

## Executed checks

| Check | Evidence class / result | Scope |
|---|---|---|
| GCC 14.2 Release configure/build/CTest | VERIFIED / 14 of 14 PASS | Current core, engine, controller, DSP and explicit adapter-contract targets |
| GCC 14.2 Debug + UBSan | VERIFIED / 14 of 14 PASS | Current native executable tests |
| Clang 17 Release | VERIFIED / 14 of 14 PASS | Second-compiler build and native tests |
| Clang 17 Debug + ASan + UBSan | VERIFIED / 14 of 14 PASS | Fresh sanitizer build and execution, without disabling sandbox protections |
| Python 3.13 standard-library tests | VERIFIED / 38 of 38 PASS | Positive and negative preset, metadata, telemetry, dependency, source-preservation and installer fixtures |
| NaN helper negative test | VERIFIED / expected exit 1 | Actual CHECK_NEAR assertion observed, not a sanitizer startup failure |
| Firmware adapter C++ checks | VERIFIED / PASS | Both real firmware translation units compiled with an explicitly fake BSP; NOT real libDaisy/ARM |
| Application MIDI/callback policy | VERIFIED / PASS | Actual router/callback with fake transport: channel, velocity-zero, queue overflow, priority stop, Start, ignored Clock and CC123 |
| Original R0 preservation | VERIFIED / 29 of 29 byte matches | Directly compared supplied ZIP bytes with references/r0 |
| Factory data generation | VERIFIED / PASS | Checked-in header matches validated original JSON |
| Host sonic scenarios | VERIFIED / 8 of 8 PASS | Ten-second 48 kHz stereo PCM renders; two runs identical; metrics and actual PCM gates |
| GitHub workflow structure | VERIFIED / PASS | YAML parsed; exact dependency-ref, jobs, permissions and no-flash assertions only |
| Documentation links | VERIFIED / PASS | Active local Markdown targets exist |
| Manual PDF | VERIFIED / PASS | 18 pages; rendered layout and final panel/tutorial visual checks; page-text bounds |
| Additive package/patch transfer | See delivery receipt | Final manifest, no-conflict application and unrelated-file preservation checked at packaging |

These are fourteen distinct CTest executables, repeated across four build
configurations; do not describe them as fifty-six distinct behavioral tests.
The source history also includes ordinary four-test R0 results. Those historical
results are not being substituted for this candidate's validation.

## Repaired audible-transition failure

The first matched-trajectory test failed on an active Arc retrigger:
**1.42942 full-scale first-sample deviation** versus its untouched reference.
After replacing the modulation-depth reset with a 1 ms rise from current state,
the worst tested retrigger deviation is **0.0320582**.

Smoothing the noise-filter coefficient reduced the tested parameter-event
deviation from **0.210695** to **0.0263815**. `test_transitions` also verifies
that repeated Stop/Release does not postpone silence.

The before/after logs are retained. These are selected software regression
measurements, not an assertion that every audible click or aliasing mechanism
has been eliminated.

## Host audio measurements

Eight original tutorials were rendered twice using the actual C++ Engine.
All WAVs are 48 kHz, stereo, 16-bit PCM, ten seconds long. Bank B is requested
at 3 s, Fill held from 5 to 6 s, Panic at 7 s.

Observed maximum peak over the scenarios: **0.300022 normalized**.
All scenario fault counters are zero. All final-second PCM tails are exactly
silent. Blank Canvas is intentionally silent. The metrics file records peak,
RMS, DC, maximum adjacent-sample delta and tail level.

The renderer is not Field audio capture. A high noisy-drum adjacent-sample
delta is not itself evidence of a control click. No listening assessment,
alias-free claim, physical SNR result, analog-voltage result or host/target
numerical equivalence is inferred.

## Local holds and unexecuted checks

**Earlier GCC ASan route: HOLD.** Its binaries built, but the runtime rejected
startup because the ASan library was not first in the library list. That run
is not counted as PASS. The separately compiled Clang ASan+UBSan suite
subsequently passed all fourteen tests in the unchanged host environment.
The failure log remains visible rather than being erased.

**Real ARM build: NOT_RUN.** Running `tools/build_target.py` exited nonzero at
preflight because the `arm-none-eabi-*` tools are absent. The real dependency
verifier separately rejected the missing local libDaisy checkout. No real
target compiler or linker was executed. No firmware BIN/ELF/MAP is supplied.

**Pinned libDaisy checkout: NOT_VERIFIED in this runtime.** Full commit and
license-blob identities came from earlier repository inspection in this
conversation. Current public API/build sources were inspected, but this does
not replace compiling the exact pin. The private-checkout verifier and build
script make that a hard Codex gate.

**Mermaid parser/rendering: NOT_RUN.** Eight `.mmd`/Markdown pairs are checked
for exact source consistency and were reviewed against the implementation.
The actual CLI is absent. Do not label pair consistency as grammar/render PASS.

**Formatter: NOT_RUN.** No clang-format executable was available. C++ warnings
are treated as errors by the host build; that is not a formatting check.

**GitHub publication and CI: NOT_RUN.** No remote changes were made. The live
target fork, its current master HEAD and any extra private tools were not
qualified through this run's public retrieval route. The old dedicated
repository's historical CI result does not apply to this candidate.

**Physical P0/P1: NOT_RUN.** No Field was attached. Key/LED physical mapping,
boot/reset provenance, audio, CV/Gate levels, MIDI electrical reception and
callback margin remain bench measurements. The candidate's output-arm logic
is not hardware fail-safe protection.

**DVPE: intentionally omitted.** No valid graph/schema/export round trip was
executed. No guessed `.dvpe` or new DVPE registry block is delivered.

## Evidence files and next gate

Raw commands/output and tool versions are in `evidence/local`. Machine-readable
results bind the runtime source list; the package manifest binds delivery bytes.
The printable manual is a separate artifact generated from the active Markdown,
not a copy of either supplied commercial manual.

Use `CODEX_EXECUTION.md` for actual repository comparison, additive integration,
exact dependency checkout, target compilation, Mermaid rendering, CI and
non-destructive publication. Use `QA_PLAN.md` and the eleven NOT_RUN hardware
record templates for the subsequent bench work.

All review in this authoring context is non-independent. No formal release
or independent assurance is claimed.
