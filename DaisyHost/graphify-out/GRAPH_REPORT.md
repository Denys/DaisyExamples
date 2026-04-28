# Graph Report - DaisyHost  (2026-04-28)

## Corpus Check
- 135 files · ~261,302 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 1559 nodes · 3873 edges · 31 communities detected
- Extraction: 80% EXTRACTED · 20% INFERRED · 0% AMBIGUOUS · INFERRED: 772 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [[_COMMUNITY_Community 0|Community 0]]
- [[_COMMUNITY_Community 1|Community 1]]
- [[_COMMUNITY_Community 2|Community 2]]
- [[_COMMUNITY_Community 3|Community 3]]
- [[_COMMUNITY_Community 4|Community 4]]
- [[_COMMUNITY_Community 5|Community 5]]
- [[_COMMUNITY_Community 6|Community 6]]
- [[_COMMUNITY_Community 7|Community 7]]
- [[_COMMUNITY_Community 8|Community 8]]
- [[_COMMUNITY_Community 9|Community 9]]
- [[_COMMUNITY_Community 10|Community 10]]
- [[_COMMUNITY_Community 11|Community 11]]
- [[_COMMUNITY_Community 12|Community 12]]
- [[_COMMUNITY_Community 13|Community 13]]
- [[_COMMUNITY_Community 14|Community 14]]
- [[_COMMUNITY_Community 15|Community 15]]
- [[_COMMUNITY_Community 16|Community 16]]
- [[_COMMUNITY_Community 17|Community 17]]
- [[_COMMUNITY_Community 18|Community 18]]
- [[_COMMUNITY_Community 19|Community 19]]
- [[_COMMUNITY_Community 20|Community 20]]
- [[_COMMUNITY_Community 21|Community 21]]
- [[_COMMUNITY_Community 22|Community 22]]
- [[_COMMUNITY_Community 23|Community 23]]
- [[_COMMUNITY_Community 24|Community 24]]
- [[_COMMUNITY_Community 25|Community 25]]
- [[_COMMUNITY_Community 26|Community 26]]
- [[_COMMUNITY_Community 27|Community 27]]
- [[_COMMUNITY_Community 29|Community 29]]
- [[_COMMUNITY_Community 31|Community 31]]
- [[_COMMUNITY_Community 69|Community 69]]

## God Nodes (most connected - your core abstractions)
1. `Clear()` - 46 edges
2. `TEST()` - 38 edges
3. `StringVar()` - 37 edges
4. `RunMultiNodeRenderScenario()` - 37 edges
5. `DaisyHostPatchAudioProcessorEditor()` - 37 edges
6. `RunRenderScenario()` - 35 edges
7. `processBlock()` - 32 edges
8. `RefreshCoreStateFromIdleHostChange()` - 32 edges
9. `GetPatchBindings()` - 29 edges
10. `ResetToDefaultState()` - 29 edges

## Surprising Connections (you probably didn't know these)
- `TEST()` --calls--> `GetSourceCount()`  [INFERRED]
  tests\test_subharmoniq_core.cpp → src\DaisySubharmoniqCore.cpp
- `TEST()` --calls--> `GetDryWetPercent()`  [INFERRED]
  tests\test_multidelay_core.cpp → src\apps\MultiDelayCore.cpp
- `TEST()` --calls--> `GetFeedback()`  [INFERRED]
  tests\test_multidelay_core.cpp → src\apps\MultiDelayCore.cpp
- `TEST()` --calls--> `GetDelayTargetSamples()`  [INFERRED]
  tests\test_multidelay_core.cpp → src\apps\MultiDelayCore.cpp
- `TEST()` --calls--> `GetTriggerCount()`  [INFERRED]
  tests\test_subharmoniq_core.cpp → src\apps\SubharmoniqCore.cpp

## Communities

### Community 0 - "Community 0"
Cohesion: 0.02
Nodes (262): DaisyFieldKnobLayoutMode(), AccentColour(), ApplyWindowIconIfNeeded(), BackgroundColour(), BoardPanelColour(), BoardPanelShadow(), BodyFont(), buttonClicked() (+254 more)

### Community 1 - "Community 1"
Cohesion: 0.05
Nodes (104): GetActivePageBinding(), SetActivePage(), TriggerGate(), TriggerMomentaryAction(), BuildDisplay(), BuildMenuModel(), CaptureStatefulParameterValues(), FormatOnOff() (+96 more)

### Community 2 - "Community 2"
Cohesion: 0.05
Nodes (91): TickUi(), daisyhost(), GetTestInputModeName(), SetTestInputMode(), CreateHostedAppCore(), Assign(), Bindings(), Deserialize() (+83 more)

### Community 3 - "Community 3"
Cohesion: 0.05
Nodes (73): BraidsCore(), BuildDisplay(), BuildMenuModel(), CaptureStatefulParameterValues(), Clamp01(), FormatPageText(), FormatPercent(), FormatTune() (+65 more)

### Community 4 - "Community 4"
Cohesion: 0.07
Nodes (70): AbsMax(), ApplyParameterValue(), Clamp01(), ClampInt(), ClearEffectiveParameterOverrides(), DeriveMetaControllerValue(), FindMetaControllerById(), FindParameterById() (+62 more)

### Community 5 - "Community 5"
Cohesion: 0.06
Nodes (67): BuildDisplay(), BuildMenuModel(), CaptureStatefulParameterValues(), Clamp01(), FieldKeyDetailLabel(), FieldKeyMenuItemSuffix(), FormatPageLabel(), FormatPercent() (+59 more)

### Community 6 - "Community 6"
Cohesion: 0.06
Nodes (49): BuildDisplay(), BuildMenuModel(), CaptureStatefulParameterValues(), Clamp01(), FormatPercent(), GetControlValue(), GetEffectiveParameterValue(), GetParameterValue() (+41 more)

### Community 7 - "Community 7"
Cohesion: 0.08
Nodes (53): BuildDisplay(), BuildMenuModel(), CaptureStatefulParameterValues(), Clamp01(), ClampInt(), CloudSeedCore(), FormatArpPatternText(), FormatArpRateText() (+45 more)

### Community 8 - "Community 8"
Cohesion: 0.09
Nodes (55): AudioInputVar(), AutomationSlotVar(), CapabilitiesVar(), ControlKindName(), ControlSpecVar(), CvInputVar(), DebugControlTargetsVar(), DebugNodeVar() (+47 more)

### Community 9 - "Community 9"
Cohesion: 0.09
Nodes (44): AssignedParameterNormalizedValue(), AssignmentForControl(), AssignmentToParameterIndex(), BaseStepForMenuItem(), ChoiceName(), Clamp01(), CompactMenuDisplayValue(), ControlNameFromAssignment() (+36 more)

### Community 10 - "Community 10"
Cohesion: 0.07
Nodes (28): AdvanceClockPulse(), Clamp01(), ClampInt(), DecodeSequencerStepId(), GetSequencerCv(), GetSequencerStepRatio(), GetSequencerStepSemitones(), GetSourceCount() (+20 more)

### Community 11 - "Community 11"
Cohesion: 0.1
Nodes (41): BuildDisplay(), BuildMenuModel(), CaptureStatefulParameterValues(), Clamp01(), FormatPageText(), FormatPercent(), GetControlValue(), GetEffectiveParameterValue() (+33 more)

### Community 12 - "Community 12"
Cohesion: 0.1
Nodes (41): RuntimeError, ensure_executable_not_locked(), ensure_executable_not_running(), format_command(), main(), parse_args(), query_processes_for_path(), Raised when a smoke check fails. (+33 more)

### Community 13 - "Community 13"
Cohesion: 0.09
Nodes (37): GetFieldPublicParameterBindings(), SetFieldDrawerPage(), StepFieldDrawerPage(), AssignPortBinding(), BuildDaisyFieldControlMapping(), BuildDaisyFieldPublicParameterList(), BuildMirroredParameterIds(), BuildRankedPublicParameters() (+29 more)

### Community 14 - "Community 14"
Cohesion: 0.14
Nodes (30): GetDefaultHostedAppId(), BuildDatasetJobJson(), BuildHubLaunchPlan(), BuildRenderScenarioJson(), ClearHubStartupRequest(), DiscoverDefaultHubToolPaths(), ExecuteHubLaunchPlan(), FindActivityRegistration() (+22 more)

### Community 15 - "Community 15"
Cohesion: 0.14
Nodes (25): ApplyRackNodeModulation(), BuildModulationSnapshots(), BuildModulationSourceValues(), GetModulationLaneDisplayText(), ApplyDaisyFieldExternalControlSafetyFloor(), Clamp01(), EvaluateHostModulation(), HostModulationSourceFromString() (+17 more)

### Community 16 - "Community 16"
Cohesion: 0.15
Nodes (25): BoardEditorTraceMode(), CreateBoardProfile(), GetSupportedBoardIds(), MakeControl(), MakeDaisyFieldProfile(), MakeDaisyPatchProfile(), MakeDecoration(), MakeIndicator() (+17 more)

### Community 17 - "Community 17"
Cohesion: 0.11
Nodes (13): Clamp01(), DetuneSemitones(), MakeDefaultParameters(), MidiNoteToFrequency(), Process(), RestoreStatefulParameterValues(), SecondsFromAttack(), SecondsFromRelease() (+5 more)

### Community 18 - "Community 18"
Cohesion: 0.12
Nodes (15): AttackSeconds(), Clamp01(), DetuneSemitones(), GetWaveLabel(), MidiNoteToFrequency(), Panic(), Process(), ReleaseSeconds() (+7 more)

### Community 19 - "Community 19"
Cohesion: 0.13
Nodes (14): Clamp01(), FindParameter(), FormatSeedValue(), GetEffectiveParameterValue(), GetParameterValue(), GetSeedSummary(), HasMeaningfulChange(), Prepare() (+6 more)

### Community 20 - "Community 20"
Cohesion: 0.17
Nodes (18): load_module(), NextWpSuggesterTest, first_safe_slice_for(), is_candidate(), is_waiting(), main(), overlap_risk_for(), parse_percent() (+10 more)

### Community 21 - "Community 21"
Cohesion: 0.19
Nodes (18): AddBlocker(), BlockersVar(), BuildGateDiagnostics(), BuildPhases(), ClassifyBlockers(), ContainsCaseSensitive(), CtestVar(), DefaultGateTargets() (+10 more)

### Community 22 - "Community 22"
Cohesion: 0.22
Nodes (17): GetRackNodeRoleLabelCompat(), GetRackNodeRoleLabel(), BuildLiveRackTopologyConfig(), GetLiveRackNodeRoleDisplayLabel(), GetLiveRackTopologyDisplayLabel(), IsSupportedNodeId(), MakeCanonicalRoutes(), MakeEndpoint() (+9 more)

### Community 23 - "Community 23"
Cohesion: 0.26
Nodes (12): BuildCommandLine(), DoctorItem(), HasFlag(), main(), PrintPayload(), PrintUsage(), QuoteArgument(), ReadOption() (+4 more)

### Community 24 - "Community 24"
Cohesion: 0.27
Nodes (10): daisyhost(), BuildDaisyFieldCvGeneratorCardLayout(), BuildDaisyFieldKeyMappingLegendLayout(), DropTop(), Inset(), TakeTop(), BottomOf(), Overlaps() (+2 more)

### Community 25 - "Community 25"
Cohesion: 0.48
Nodes (5): AdjustEditorBoundsForHiddenMuteBanner(), BottomOf(), IsStandaloneMuteBannerCandidate(), RightOf(), TEST()

### Community 26 - "Community 26"
Cohesion: 0.33
Nodes (2): DaisyHostHubContent, DaisyHostHubWindow()

### Community 27 - "Community 27"
Cohesion: 0.67
Nodes (5): make_work_dir(), run_python(), test_audit_classifies_shared_core_adapter_as_portable_ready(), test_audit_reports_raw_field_firmware_needs_core_extraction(), test_generator_emits_multidelay_field_adapter()

### Community 29 - "Community 29"
Cohesion: 0.5
Nodes (2): ResolveStartupTestInputMode(), TEST()

### Community 31 - "Community 31"
Cohesion: 1.0
Nodes (2): find_build_dir(), main()

### Community 69 - "Community 69"
Cohesion: 1.0
Nodes (1): DaisyHostHubApplication

## Knowledge Gaps
- **3 isolated node(s):** `SimpleSvfLowpass`, `DaisyHostHubApplication`, `Raised when a smoke check fails.`
  These have ≤1 connection - possible missing edges or undocumented components.
- **Thin community `Community 26`** (6 nodes): `closeButtonPressed()`, `DaisyHostHubContent`, `.DaisyHostHubContent()`, `DaisyHostHubWindow()`, `DaisyHostHubWindow.cpp`, `DaisyHostHubWindow.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 29`** (4 nodes): `HostStartupPolicy.cpp`, `ResolveStartupTestInputMode()`, `test_host_startup_policy.cpp`, `TEST()`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 31`** (3 nodes): `find_build_dir()`, `main()`, `run_unit_test_payload.py`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Community 69`** (2 nodes): `DaisyHostHubApplication`, `DaisyHostHubMain.cpp`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `Clear()` connect `Community 0` to `Community 1`, `Community 2`, `Community 3`, `Community 4`, `Community 5`, `Community 6`, `Community 7`, `Community 8`, `Community 9`, `Community 11`, `Community 13`, `Community 22`?**
  _High betweenness centrality (0.292) - this node is a cross-community bridge._
- **Why does `TEST()` connect `Community 1` to `Community 11`, `Community 10`, `Community 2`, `Community 5`?**
  _High betweenness centrality (0.081) - this node is a cross-community bridge._
- **Why does `RunRenderScenario()` connect `Community 2` to `Community 0`, `Community 1`, `Community 13`, `Community 23`?**
  _High betweenness centrality (0.064) - this node is a cross-community bridge._
- **Are the 45 inferred relationships involving `Clear()` (e.g. with `SerializeAppDescriptionPayloadJson()` and `SerializeBoardDescriptionPayloadJson()`) actually correct?**
  _`Clear()` has 45 INFERRED edges - model-reasoned connections that need verification._
- **Are the 36 inferred relationships involving `TEST()` (e.g. with `Prepare()` and `GetSourceCount()`) actually correct?**
  _`TEST()` has 36 INFERRED edges - model-reasoned connections that need verification._
- **Are the 16 inferred relationships involving `RunMultiNodeRenderScenario()` (e.g. with `Prepare()` and `ResetToDefaultState()`) actually correct?**
  _`RunMultiNodeRenderScenario()` has 16 INFERRED edges - model-reasoned connections that need verification._
- **Are the 13 inferred relationships involving `DaisyHostPatchAudioProcessorEditor()` (e.g. with `GetTopControlLabel()` and `GetTopControlId()`) actually correct?**
  _`DaisyHostPatchAudioProcessorEditor()` has 13 INFERRED edges - model-reasoned connections that need verification._