window.DelayBundleModel = {
  generatedAt: "2026-06-05",
  figjamUrl:
    "https://www.figma.com/board/kX5emp9EAmxTxOjb1ecj4u?utm_source=other&utm_content=edit_in_figjam&oai_id=&request_id=0ab4552c-85b5-4b64-811b-b5d9d7db05e3",
  sources: [
    {
      label: "Field_delay_bundle README",
      path: "MyProjects/_projects/Field_delay_bundle/README.md",
    },
    {
      label: "Field_delay_bundle controls report",
      path: "MyProjects/_projects/Field_delay_bundle/CONTROLS.md",
    },
    {
      label: "Shared Field adapter",
      path: "MyProjects/_projects/Field_delay_shared/FieldDelayFieldApp.h",
    },
    {
      label: "Shared delay core header",
      path: "DaisyHost/include/daisyhost/DaisyDelayFxCore.h",
    },
    {
      label: "Shared delay core implementation",
      path: "DaisyHost/src/DaisyDelayFxCore.cpp",
    },
    {
      label: "Source-verified research report",
      path:
        "C:/Users/denko/Codex/_weekly/Embedded_DSP_GitHub_Digest/docs/reports/2026-06-03-daisy-delay-source-verified-research.md",
    },
    {
      label: "FuzzyLotus/Phantasmagoria",
      path: "https://github.com/FuzzyLotus/Phantasmagoria",
    },
    {
      label: "Phantasmagoria source",
      path: "https://github.com/FuzzyLotus/Phantasmagoria/blob/main/phantasmagoria.cpp",
    },
  ],
  coreFacts: [
    "DaisyDelayFxCore exposes four source choices through DaisyDelayFxSource.",
    "Every algorithm shares 24 stable parameter slots, three knob layers, and eight knobs.",
    "The audio callback reads parameters while the foreground UI writes them, so parameter storage is rebuilt in place rather than reallocated.",
    "The Field adapter separates physical keyboard scan order from LED order because the observed input and LED rows differ.",
    "Bundle mode routes audio input, external MIDI, and B-row generated notes into the selected delay algorithm.",
  ],
  controls: {
    layers: [
      {
        id: "base",
        label: "Base",
        access: "No switch held",
        knobs: [
          ["K1", "Mix", "%"],
          ["K2", "Delay Time or Long Time", "ms"],
          ["K3", "Feedback or Decay", "%"],
          ["K4", "Tone or HF Damp", "%"],
          ["K5", "Grit, Tank Color, or Texture", "%"],
          ["K6", "Mod or Drift", "%"],
          ["K7", "Input Drive", "dB"],
          ["K8", "Output", "dB"],
        ],
      },
      {
        id: "shift1",
        label: "Shift 1",
        access: "Hold SW1",
        knobs: [
          ["K1", "Pre Delay", "ms"],
          ["K2", "Width", "%"],
          ["K3", "Spread or Diffusion", "%"],
          ["K4", "Damping", "%"],
          ["K5", "Rhythm or Tap Mode", "%"],
          ["K6", "Synth Bright", "%"],
          ["K7", "Synth Decay", "%"],
          ["K8", "Synth Level", "%"],
        ],
      },
      {
        id: "shift2",
        label: "Shift 2",
        access: "Hold SW2",
        knobs: [
          ["K1", "Range or Tank Size", "%"],
          ["K2", "Density or Grain Density", "%"],
          ["K3", "Low Cut", "Hz"],
          ["K4", "High Cut", "Hz"],
          ["K5", "Smear or Spectral Smear", "%"],
          ["K6", "Warp or Interp Warp", "%"],
          ["K7", "MIDI Attack", "ms"],
          ["K8", "MIDI Release", "ms"],
        ],
      },
    ],
    keys: [
      ["A1", "Select Tape [multifx]"],
      ["A2", "Select Tank [reverb]"],
      ["A3", "Select Texture [FunBox]"],
      ["A4", "Select Long [sdram]"],
      ["A5", "Internal synth: off, pluck, pad"],
      ["A6", "Hold: momentary, latch, drone"],
      ["A7", "Octave down"],
      ["A8", "Octave up"],
      ["B1-B8", "White keys C4 D4 E4 F4 G4 A4 B4 C5"],
      ["SW1", "Hold for layer 1"],
      ["SW2", "Hold for layer 2"],
    ],
  },
  algorithms: [
    {
      id: "tape",
      label: "Tape [multifx]",
      short: "Tape",
      sourceProject: "balazsbencs/daisy-multifx-pedal",
      coreFunction: "ProcessMultiFx",
      sourceEnum: "kMultiFxPedal",
      family: "Tape and modulated circular delay",
      basedOn:
        "SDRAM-style delay line with smoothed read time, flutter modulation, feedback tone shaping, saturation grit, and optional freeze behavior.",
      behavior: [
        "Averages stereo input to one driven signal.",
        "Smooths target delay time before reading the left and right delay lines.",
        "Offsets the right delay by width and applies LFO flutter in opposite directions.",
        "Filters feedback with a one-pole tone stage.",
        "Blends clean feedback and FastTanh saturation using the grit parameter.",
        "Can hold or partially hold buffer contents through freeze-style behavior outside bundle synth mappings.",
      ],
      strengths: [
        "Most pedal-like and immediately recognizable.",
        "Best for demonstrating modulation, saturation, tone, and feedback color.",
        "Readable in a single signal-flow diagram.",
      ],
      risks: [
        "High feedback and grit need careful gain limiting.",
        "Delay-time modulation can click or pitch-bend if smoothing is wrong.",
      ],
      keyParameters: [
        "Mix",
        "Delay Time",
        "Feedback",
        "Tone",
        "Grit",
        "Mod",
        "Width",
        "Rhythm",
      ],
      extractedAlgorithms: [
        {
          name: "Delay-time smoother",
          category: "Timing",
          role: "Moves target delay toward the knob/rhythm value before reads.",
          inputs: ["Delay Time", "Rhythm", "Sample rate"],
          outputs: ["Smoothed L/R read positions"],
        },
        {
          name: "Flutter LFO",
          category: "Modulation",
          role: "Offsets left and right reads in opposite directions for tape-like movement.",
          inputs: ["Mod", "LFO phase"],
          outputs: ["Flutter sample offset"],
        },
        {
          name: "Feedback tone filter",
          category: "Filtering",
          role: "Applies a one-pole color stage to the wet feedback path.",
          inputs: ["Tone", "Mono wet feedback"],
          outputs: ["Tone-shaped feedback"],
        },
        {
          name: "FastTanh grit blend",
          category: "Nonlinear",
          role: "Blends clean feedback with saturation for tape/pedal character.",
          inputs: ["Grit", "Driven input", "Tone state"],
          outputs: ["Saturated write signal"],
        },
        {
          name: "Freeze-aware delay writer",
          category: "Buffer state",
          role: "Controls whether new input is written or existing buffer content is held.",
          inputs: ["Freeze amount", "Feedback", "Read taps"],
          outputs: ["Delay line writes"],
        },
      ],
      graphNodes: [
        ["tapeInput", "Driven mono input", "input"],
        ["tapeLfo", "Flutter LFO", "control"],
        ["tapeSmooth", "Delay smoothing", "control"],
        ["tapeRead", "Stereo delay reads", "buffer"],
        ["tapeTone", "Feedback tone filter", "process"],
        ["tapeSaturate", "FastTanh grit blend", "process"],
        ["tapeWrite", "Freeze-aware writes", "buffer"],
        ["tapeWet", "Wet L/R", "output"],
      ],
      graphLinks: [
        ["tapeInput", "tapeSmooth", "sets time"],
        ["tapeLfo", "tapeRead", "modulates"],
        ["tapeSmooth", "tapeRead", "read positions"],
        ["tapeRead", "tapeTone", "mono feedback"],
        ["tapeTone", "tapeSaturate", "colors"],
        ["tapeSaturate", "tapeWrite", "writes"],
        ["tapeRead", "tapeWet", "outputs"],
        ["tapeWrite", "tapeRead", "next sample"],
      ],
    },
    {
      id: "tank",
      label: "Tank [reverb]",
      short: "Tank",
      sourceProject: "Farmer2K5/daisy-reverb-playground",
      coreFunction: "ProcessReverbPlayground",
      sourceEnum: "kReverbPlayground",
      family: "FDN and tank reverb-delay",
      basedOn:
        "Four scaled delay lines with damping, diffusion, and a Hadamard-like feedback mixing network.",
      behavior: [
        "Reads four base delays at 37, 53, 71, and 89 ms multiplied by tank size.",
        "Applies one-pole damping to each tap.",
        "Combines the four taps through sum and difference terms before feedback writes.",
        "Uses diffusion to scale the injected input.",
        "Builds stereo output from paired tank taps and width.",
        "Adds a pre-delay-like early component from the input path.",
      ],
      strengths: [
        "Best view of delay-as-space rather than delay-as-echo.",
        "Shows feedback matrix structure clearly.",
        "Useful bridge between delay and reverb terminology.",
      ],
      risks: [
        "Dense feedback networks hide cause and effect in simple flowcharts.",
        "Stability depends on decay, damping, and feedback matrix gain.",
      ],
      keyParameters: [
        "Mix",
        "Delay Time",
        "Decay",
        "HF Damp",
        "Tank Color",
        "Diffusion",
        "Damping",
        "Tank Size",
      ],
      extractedAlgorithms: [
        {
          name: "Four-tap tank reader",
          category: "Delay network",
          role: "Reads 37, 53, 71, and 89 ms base taps scaled by tank size.",
          inputs: ["Tank Size", "Sample rate", "Four delay lines"],
          outputs: ["Raw tank taps"],
        },
        {
          name: "Per-line damping filters",
          category: "Filtering",
          role: "Filters each tank tap before it enters the feedback matrix.",
          inputs: ["Damping", "Raw tank taps"],
          outputs: ["Damped tap states"],
        },
        {
          name: "Diffusion injector",
          category: "Diffusion",
          role: "Scales the incoming signal before reinjecting it into the tank.",
          inputs: ["Driven input", "Diffusion"],
          outputs: ["Diffuse input feed"],
        },
        {
          name: "Hadamard feedback matrix",
          category: "Feedback matrix",
          role: "Combines four damped taps with sum/difference terms for dense tank feedback.",
          inputs: ["Damped tap states", "Decay"],
          outputs: ["Four feedback write signals"],
        },
        {
          name: "Stereo width and early mix",
          category: "Output mix",
          role: "Builds stereo output from paired tank taps and a small early input component.",
          inputs: ["Width", "Pre Delay", "Damped tap states"],
          outputs: ["Wet L/R"],
        },
      ],
      graphNodes: [
        ["tankInput", "Driven mono input", "input"],
        ["tankSize", "Size-scaled base delays", "control"],
        ["tankReads", "Four delay-line reads", "buffer"],
        ["tankDamp", "Per-line damping", "process"],
        ["tankMatrix", "Hadamard feedback mix", "process"],
        ["tankDiffuse", "Diffusion injection", "process"],
        ["tankWrites", "Four feedback writes", "buffer"],
        ["tankStereo", "Width and early output", "output"],
      ],
      graphLinks: [
        ["tankInput", "tankDiffuse", "scales"],
        ["tankSize", "tankReads", "sets taps"],
        ["tankReads", "tankDamp", "filters"],
        ["tankDamp", "tankMatrix", "mixes"],
        ["tankMatrix", "tankWrites", "feeds back"],
        ["tankDiffuse", "tankWrites", "injects"],
        ["tankDamp", "tankStereo", "stereo pairs"],
        ["tankWrites", "tankReads", "next sample"],
      ],
    },
    {
      id: "texture",
      label: "Texture [FunBox]",
      short: "Texture",
      sourceProject: "GuitarML/FunBox",
      coreFunction: "ProcessFunBox",
      sourceEnum: "kFunBox",
      family: "Texture, granular, reverse, and hold delay",
      basedOn:
        "FunBox-inspired delay family combining normal stereo taps, a grain tap, a reverse-like tap, drift modulation, smear, and freeze/hold behavior.",
      behavior: [
        "Drives stereo input independently.",
        "Smooths normal left and right tap positions with slow LFO drift.",
        "Reads a grain delay from a density and smear dependent position.",
        "Reads a reverse accent tap from an opposite-moving position.",
        "Cross-feeds wet left and wet right into the write path.",
        "Uses texture and reverse state to blend normal, grain, and reverse material.",
      ],
      strengths: [
        "Most expressive creative delay in the bundle.",
        "Best for showing multiple parallel reads from shared buffer memory.",
        "Good candidate for state and interaction diagrams.",
      ],
      risks: [
        "Harder to infer sound from a static block diagram.",
        "Freeze, reverse, smear, and density are more readable with animated or interactive diagrams.",
      ],
      keyParameters: [
        "Mix",
        "Delay Time",
        "Feedback",
        "Texture",
        "Drift",
        "Grain Density",
        "Smear",
        "Warp",
      ],
      extractedAlgorithms: [
        {
          name: "Drifted normal taps",
          category: "Delay taps",
          role: "Reads normal L/R taps with slow LFO drift and ratio control.",
          inputs: ["Delay Time", "Drift", "Tap Mode", "Width"],
          outputs: ["Normal wet L/R"],
        },
        {
          name: "Grain tap",
          category: "Granular texture",
          role: "Reads a density and smear dependent tap for granular delay texture.",
          inputs: ["Grain Density", "Smear", "Texture", "Drift"],
          outputs: ["Grain signal"],
        },
        {
          name: "Reverse accent tap",
          category: "Reverse texture",
          role: "Reads a moving accent tap and blends it toward the right channel.",
          inputs: ["Reverse state", "Drift", "Delay Time"],
          outputs: ["Reverse accent"],
        },
        {
          name: "Texture blend",
          category: "Blend",
          role: "Blends normal, grain, and reverse material into wet left and wet right.",
          inputs: ["Texture", "Reverse amount", "Normal taps", "Grain", "Reverse accent"],
          outputs: ["Wet L/R"],
        },
        {
          name: "Cross-feedback writer",
          category: "Feedback",
          role: "Writes left from right wet feedback and right from left wet feedback.",
          inputs: ["Feedback", "Freeze", "Driven stereo input", "Wet L/R"],
          outputs: ["Cross-fed delay writes"],
        },
      ],
      graphNodes: [
        ["textureInput", "Driven stereo input", "input"],
        ["textureLfo", "Slow drift LFO", "control"],
        ["textureNormal", "Normal L/R taps", "buffer"],
        ["textureGrain", "Grain tap", "buffer"],
        ["textureReverse", "Reverse accent tap", "buffer"],
        ["textureBlend", "Texture and reverse blend", "process"],
        ["textureCross", "Cross-feedback writes", "buffer"],
        ["textureWet", "Wet L/R", "output"],
      ],
      graphLinks: [
        ["textureInput", "textureNormal", "writes"],
        ["textureLfo", "textureNormal", "drifts"],
        ["textureLfo", "textureGrain", "smears"],
        ["textureNormal", "textureBlend", "normal"],
        ["textureGrain", "textureBlend", "grain"],
        ["textureReverse", "textureBlend", "reverse"],
        ["textureBlend", "textureCross", "cross feeds"],
        ["textureBlend", "textureWet", "outputs"],
        ["textureCross", "textureNormal", "next sample"],
      ],
    },
    {
      id: "long",
      label: "Long [sdram]",
      short: "Long",
      sourceProject: "Farmer2K5/daisy-sdram-delaylines",
      coreFunction: "ProcessSdramDelaylines",
      sourceEnum: "kSdramDelaylines",
      family: "Long fractional stereo delay and SDRAM primitive",
      basedOn:
        "Long external-buffer delay-line behavior with fractional reads, slow modulation, warp taps, smear blend, and ping-pong cross feedback.",
      behavior: [
        "Uses a longer native delay-time range than the other bundle modes.",
        "Smooths left and right delay positions slowly for stable long echoes.",
        "Applies small LFO modulation proportional to base delay and modulation depth.",
        "Reads secondary warp taps at independent proportions of the base delay.",
        "Cross-mixes left and right feedback through the texture parameter.",
        "Blends primary reads and secondary taps with smear.",
      ],
      strengths: [
        "Most direct visualization of the reusable delay-line primitive.",
        "Best for explaining memory pressure and long external buffers.",
        "Clean comparison baseline for the more colored algorithms.",
      ],
      risks: [
        "Less distinctive as a product effect unless paired with long buffer and ping-pong diagrams.",
        "Large delay ranges need exact storage size and interpolation discipline.",
      ],
      keyParameters: [
        "Mix",
        "Long Time",
        "Feedback",
        "Tone",
        "Grit/Cross",
        "Mod",
        "Range",
        "Warp",
      ],
      extractedAlgorithms: [
        {
          name: "Long base-time mapper",
          category: "Timing",
          role: "Maps long delay time to a fractional base delay in external-buffer space.",
          inputs: ["Long Time", "Sample rate", "Delay storage size"],
          outputs: ["Base delay samples"],
        },
        {
          name: "Slow modulated fractional reads",
          category: "Fractional delay",
          role: "Reads primary left and right taps with slow LFO modulation.",
          inputs: ["Base delay", "Mod", "Width", "LFO phase"],
          outputs: ["Primary read L/R"],
        },
        {
          name: "Secondary warp taps",
          category: "Multi-tap delay",
          role: "Reads extra taps at warped proportions of the base delay.",
          inputs: ["Warp", "Base delay"],
          outputs: ["Warp tap L/R"],
        },
        {
          name: "Ping-pong cross feedback",
          category: "Feedback",
          role: "Cross-mixes left and right feedback to create stereo ping-pong behavior.",
          inputs: ["Feedback", "Grit/Cross", "Driven stereo input"],
          outputs: ["Delay line writes"],
        },
        {
          name: "Smear blend",
          category: "Output mix",
          role: "Blends primary reads with secondary warp taps.",
          inputs: ["Smear", "Primary reads", "Warp taps"],
          outputs: ["Wet L/R"],
        },
      ],
      graphNodes: [
        ["longInput", "Driven stereo input", "input"],
        ["longBase", "Long base delay", "control"],
        ["longLfo", "Slow LFO modulation", "control"],
        ["longPrimary", "Primary L/R reads", "buffer"],
        ["longWarp", "Secondary warp taps", "buffer"],
        ["longCross", "Ping-pong feedback", "process"],
        ["longSmear", "Smear blend", "process"],
        ["longWet", "Wet L/R", "output"],
      ],
      graphLinks: [
        ["longInput", "longCross", "drives"],
        ["longBase", "longPrimary", "sets time"],
        ["longLfo", "longPrimary", "modulates"],
        ["longBase", "longWarp", "sets tap range"],
        ["longPrimary", "longCross", "feedback"],
        ["longWarp", "longCross", "secondary feedback"],
        ["longCross", "longPrimary", "writes"],
        ["longPrimary", "longSmear", "primary"],
        ["longWarp", "longSmear", "secondary"],
        ["longSmear", "longWet", "outputs"],
      ],
    },
  ],
  referenceProjects: [
    {
      id: "phantasmagoria",
      label: "Phantasmagoria [FuzzyLotus]",
      sourceProject: "FuzzyLotus/Phantasmagoria",
      sourceUrl: "https://github.com/FuzzyLotus/Phantasmagoria",
      sourceFile: "phantasmagoria.cpp",
      platform: "Daisy Seed / PedalPCB Terrarium",
      summary:
        "Atmospheric spectral delay and echo chamber with reverse delay, smear, erosion, freeze evolution, reverb taps, and tape warble. This extraction lists delay-related algorithms only.",
      controls: [
        ["K1", "Delay Time"],
        ["K2", "Feedback"],
        ["K3", "Reverb Mix"],
        ["K4", "Tape Warble Depth"],
        ["K5", "LFO Speed"],
        ["K6", "Dry/Wet Mix"],
        ["SW1", "Time Direction: forward/reverse"],
        ["SW2", "Smear: multi-tap diffusion"],
        ["SW3", "Erosion: repeat aging"],
        ["SW4", "Freeze Evolution"],
        ["FS1", "Bypass"],
        ["FS2", "Freeze toggle/accumulate"],
      ],
      extractedAlgorithms: [
        {
          name: "Tri-LFO Tape Warble",
          category: "Modulation",
          role:
            "Combines three sine LFOs at related rates to modulate delay read time with tape-like instability.",
          controls: ["K4 Tape Depth", "K5 LFO Speed"],
          evidence: "lfo1/lfo2/lfo3 drive modMs and modSamps before delay reads.",
          inputs: ["LFO phases", "Tape depth", "LFO speed"],
          outputs: ["Delay modulation samples"],
        },
        {
          name: "Main Spectral Delay Line",
          category: "Delay core",
          role:
            "Writes dry plus feedback into the main SDRAM delay and reads a smoothed forward tap.",
          controls: ["K1 Delay Time", "K2 Feedback"],
          evidence: "mainDelay.Write(delIn), smoothed sDelay, and mainDelay.Read(fwdS).",
          inputs: ["Dry input", "Feedback signal", "Smoothed delay time"],
          outputs: ["Forward delay read"],
        },
        {
          name: "Reverse Dual-Grain Reader",
          category: "Reverse delay",
          role:
            "Uses two windowed grain phases to sweep a reverse-like read region, then crossfades with the forward delay.",
          controls: ["SW1 REV", "K4 Tape Depth"],
          evidence: "GrainReader::Process reads grainA/grainB; delRd crossfades fwd and rev by sSw1.",
          inputs: ["Main delay buffer", "Grain phase", "Tape modulation"],
          outputs: ["Reverse grain read"],
        },
        {
          name: "Smear Multi-Tap Diffusion",
          category: "Diffusion",
          role:
            "Adds widened +10 ms and +25 ms taps to the audible wet bus and feedback path for temporal smear.",
          controls: ["SW2 Smear", "K2 Feedback"],
          evidence: "SW2 reads fwdS + 500 and fwdS + 1200 samples, then blends tapA/tapB into delRd.",
          inputs: ["Forward read position", "Feedback", "Smear switch"],
          outputs: ["Diffused delay read"],
        },
        {
          name: "Erosion Repeat Aging Filter",
          category: "Aging filter",
          role:
            "Darkens and attenuates the audible delay read before it enters feedback, so repeats age progressively.",
          controls: ["SW3 Erosion", "K2 Feedback"],
          evidence: "erosionLpf sweeps from 7500 Hz toward 1200 Hz and attenuates delRd.",
          inputs: ["Delay read", "Feedback", "Erosion switch"],
          outputs: ["Darkened delay read"],
        },
        {
          name: "Echo Chamber Reverb Taps",
          category: "Reverb",
          role:
            "Runs a separate reverb delay with fixed taps at 83, 151, 227, and 311 ms.",
          controls: ["K3 Reverb Mix"],
          evidence: "REV_TAP_A/B/C/D are read from revDelay and averaged into rvSum.",
          inputs: ["Delay read", "Reverb feedback", "Reverb mix"],
          outputs: ["Echo chamber wet signal"],
        },
        {
          name: "Freeze Voice Bank",
          category: "Freeze",
          role:
            "Uses three independent FreezeVoice delay buffers to hold or accumulate frozen audio layers.",
          controls: ["FS2 Freeze", "Hold accumulate"],
          evidence: "fzVoice[3] processes dryGt, holdGt, and fzMod with 97, 149, and 199 ms buffers.",
          inputs: ["Dry input", "Freeze gate", "Accumulate gate"],
          outputs: ["Frozen voice sum"],
        },
        {
          name: "Freeze Evolution Drift",
          category: "Freeze modulation",
          role:
            "Adds ultra-slow drift to frozen voices when freeze and SW4 evolution are active.",
          controls: ["SW4 EVOL", "FS2 Freeze"],
          evidence: "evolvePhase increments slowly and adds evolveDrift to fzMod.",
          inputs: ["Freeze hold", "Evolution switch", "Slow phase"],
          outputs: ["Living freeze modulation"],
        },
      ],
      excludedSupportBlocks: [
        {
          name: "Hi-Fi Dynamics Soft Clip",
          reason:
            "Dynamics and limiting support stage, not a distinct delay algorithm.",
        },
        {
          name: "Constant-Power Dry/Wet Mixer",
          reason:
            "Output integration and perceived-mix curve, not a delay algorithm.",
        },
      ],
    },
  ],
  literatureReviews: [
    {
      id: "phantasmagoria-equivalence",
      title: "Phantasmagoria Delay Algorithms Review",
      scope:
        "Review extracted FuzzyLotus/Phantasmagoria delay-related algorithms at the same practical depth as the bundle's Tape, Tank, Texture, and Long modes. Pure dynamics and output-mix support blocks are documented as exclusions, not candidate delay algorithms.",
      reviewType: "Source-verified technical literature review",
      searchDate: "2026-06-05",
      researchQuestion:
        "Which Phantasmagoria delay algorithms are reusable or conceptually equivalent to the Field delay bundle's Tape, Tank, Texture, and Long families?",
      evidenceSources: [
        {
          label: "Phantasmagoria README",
          url: "https://github.com/FuzzyLotus/Phantasmagoria",
          role: "Declared feature set and controls.",
        },
        {
          label: "Phantasmagoria phantasmagoria.cpp",
          url: "https://github.com/FuzzyLotus/Phantasmagoria/blob/main/phantasmagoria.cpp",
          role: "Primary DSP implementation evidence.",
        },
        {
          label: "Daisy Delay source-verified research report",
          url:
            "C:/Users/denko/Codex/_weekly/Embedded_DSP_GitHub_Digest/docs/reports/2026-06-03-daisy-delay-source-verified-research.md",
          role: "Existing baseline for the four bundle source projects.",
        },
        {
          label: "Physical Audio Signal Processing: fractional delay",
          url:
            "https://www.dsprelated.com/freebooks/pasp/Fractional_Delay_Filtering_Linear.html",
          role: "Background for fractional/interpolated delay reads.",
        },
        {
          label: "Physical Audio Signal Processing: artificial reverberation",
          url: "https://www.dsprelated.com/freebooks/pasp/Artificial_Reverberation.html",
          role: "Background for delay-line reverberation networks.",
        },
        {
          label: "Granular synthesis overview",
          url: "https://www.sfu.ca/~truax/gran.html",
          role: "Background for grain/window based audio processing.",
        },
      ],
      inclusionCriteria: [
        "Must be present in Phantasmagoria README or phantasmagoria.cpp.",
        "Must be delay-line, reverse/grain, diffusion/reverb-tap, freeze-buffer, delay-time-modulation, or repeat-aging behavior.",
        "Must be comparable to Tape, Tank, Texture, or Long in structure or purpose.",
        "Must have enough implementation evidence to name inputs and outputs.",
      ],
      exclusions: [
        "Hi-Fi Dynamics Soft Clip is excluded from the delay algorithm list because it is a limiting/dynamics stage.",
        "Constant-Power Dry/Wet Mixer is excluded from the delay algorithm list because it is output integration, not an effect identity.",
      ],
      synthesisThemes: [
        {
          theme: "Delay-line substrate and fractional access",
          finding:
            "Phantasmagoria's main delay line and interpolated read path align most closely with Long [sdram], while its modulation-dependent read offsets pull it toward Tape [multifx].",
          mappedAlgorithms: ["Main Spectral Delay Line", "Tri-LFO Tape Warble"],
          closestBundleModes: ["Long [sdram]", "Tape [multifx]"],
        },
        {
          theme: "Reverse and granular time motion",
          finding:
            "The Reverse Dual-Grain Reader is most comparable to Texture [FunBox], because both use moving non-primary taps to create time-texture rather than a plain echo.",
          mappedAlgorithms: ["Reverse Dual-Grain Reader"],
          closestBundleModes: ["Texture [FunBox]"],
        },
        {
          theme: "Diffusion, reverb, and space",
          finding:
            "Smear Multi-Tap Diffusion and Echo Chamber Reverb Taps are the closest Phantasmagoria equivalents to Tank [reverb], but they use fixed taps and wet-bus diffusion rather than the bundle's four-line feedback matrix.",
          mappedAlgorithms: ["Smear Multi-Tap Diffusion", "Echo Chamber Reverb Taps"],
          closestBundleModes: ["Tank [reverb]"],
        },
        {
          theme: "Freeze and evolving held memory",
          finding:
            "Freeze Voice Bank and Freeze Evolution Drift extend the same creative family as Texture [FunBox]'s freeze/hold behavior, but use independent short buffers instead of only holding the active delay path.",
          mappedAlgorithms: ["Freeze Voice Bank", "Freeze Evolution Drift"],
          closestBundleModes: ["Texture [FunBox]", "Long [sdram]"],
        },
        {
          theme: "Delay-repeat coloration",
          finding:
            "Erosion Repeat Aging Filter is kept as a delay algorithm because it processes the audible delay read and feedback path so each repeat darkens and decays as part of the delay behavior.",
          mappedAlgorithms: ["Erosion Repeat Aging Filter"],
          closestBundleModes: ["Tape [multifx]", "Tank [reverb]"],
        },
      ],
      equivalenceMatrix: [
        {
          extracted: "Tri-LFO Tape Warble",
          closest: "Tape [multifx]",
          reusePriority: "High",
          implementationNotes:
            "Adopt as a richer modulation source for Tape and optionally Long; needs smoothing and modulation-depth limits.",
        },
        {
          extracted: "Main Spectral Delay Line",
          closest: "Long [sdram]",
          reusePriority: "Medium",
          implementationNotes:
            "Conceptually overlaps existing long delay storage; useful as a comparative long-delay architecture rather than a new mode.",
        },
        {
          extracted: "Reverse Dual-Grain Reader",
          closest: "Texture [FunBox]",
          reusePriority: "High",
          implementationNotes:
            "Strong candidate for a named Texture sub-mode or separate Reverse Texture mode.",
        },
        {
          extracted: "Smear Multi-Tap Diffusion",
          closest: "Tank [reverb]",
          reusePriority: "High",
          implementationNotes:
            "Good lightweight diffusion alternative to a full FDN; useful for Tank and Texture.",
        },
        {
          extracted: "Erosion Repeat Aging Filter",
          closest: "Tape [multifx]",
          reusePriority: "Medium",
          implementationNotes:
            "Good character module for feedback paths; must be gain-checked to avoid dull or unstable repeats.",
        },
        {
          extracted: "Echo Chamber Reverb Taps",
          closest: "Tank [reverb]",
          reusePriority: "Medium",
          implementationNotes:
            "A fixed-tap reverb branch can be shown alongside the Tank FDN as a simpler room model.",
        },
        {
          extracted: "Freeze Voice Bank",
          closest: "Texture [FunBox]",
          reusePriority: "High",
          implementationNotes:
            "Best candidate for upgrading freeze/hold from buffer hold to layered held memory.",
        },
        {
          extracted: "Freeze Evolution Drift",
          closest: "Texture [FunBox]",
          reusePriority: "High",
          implementationNotes:
            "Pairs naturally with Freeze Voice Bank; use low-depth independent drift to avoid pitch chaos.",
        },
      ],
      gaps: [
        "No hardware CPU or memory measurement for Phantasmagoria was performed in this dashboard task.",
        "Phantasmagoria targets Terrarium/Petal-style controls, so Field control mapping would need a separate adaptation pass.",
        "Reverse/grain behavior was source-reviewed but not auditioned.",
        "GPL-3.0 license constraints require care before directly porting source code into non-GPL projects.",
      ],
      recommendation:
        "Treat Reverse Dual-Grain Reader, Smear Multi-Tap Diffusion, Freeze Voice Bank, and Freeze Evolution Drift as the highest-value candidates for a future expanded delay bundle. Treat Main Spectral Delay Line, Tri-LFO Tape Warble, Erosion Repeat Aging Filter, and Echo Chamber Reverb Taps as strong comparative delay architecture references.",
    },
  ],
  supportSystems: [
    {
      id: "fieldControls",
      label: "Field control adapter",
      nodes: [
        ["scan", "ProcessAllControls", "input"],
        ["layer", "SW1/SW2 layer selection", "control"],
        ["touch", "Until-touched knob gate", "process"],
        ["write", "SetParameterValue", "process"],
        ["zoom", "OLED zoom record", "output"],
        ["leds", "LED values and blinking", "output"],
      ],
      links: [
        ["scan", "layer", "reads switches"],
        ["scan", "touch", "reads knobs"],
        ["layer", "touch", "sets context"],
        ["touch", "write", "only after movement"],
        ["write", "zoom", "formats value"],
        ["scan", "leds", "updates keys"],
      ],
    },
    {
      id: "internalSynth",
      label: "Internal pluck/pad synth",
      nodes: [
        ["midi", "External MIDI", "input"],
        ["brow", "B1-B8 C4-C5", "input"],
        ["mode", "A5 synth mode", "control"],
        ["hold", "A6 hold mode", "control"],
        ["voices", "8 SynthVoice slots", "process"],
        ["tone", "Pluck or pad body", "process"],
        ["sum", "Mono synth injection", "output"],
      ],
      links: [
        ["midi", "voices", "note events"],
        ["brow", "voices", "test notes"],
        ["mode", "tone", "selects engine"],
        ["hold", "voices", "release policy"],
        ["voices", "tone", "per voice"],
        ["tone", "sum", "adds to input"],
      ],
    },
  ],
  visualizationApproaches: [
    {
      id: "mermaid",
      label: "Markdown Mermaid",
      bestFor:
        "Canonical repo docs, line-reviewable algorithm explanations, and lightweight architecture sketches.",
      notBestFor:
        "Large interactive exploration or diagrams that need manual spatial editing.",
      suitability: 9,
      maintenance: 9,
      comprehension: 8,
      recommendation:
        "Use as the source-controlled reference for each algorithm and every control-flow invariant.",
    },
    {
      id: "gojs",
      label: "GoJS web explorer",
      bestFor:
        "Interactive comparison, filtering by algorithm, future expansion, and teaching how the same core changes per mode.",
      notBestFor:
        "Minimal docs where a static flowchart is enough.",
      suitability: 10,
      maintenance: 7,
      comprehension: 9,
      recommendation:
        "Use as the most comprehensible reader-facing explorer once the data model is kept in one file.",
    },
    {
      id: "figma",
      label: "FigJam generated diagram",
      bestFor:
        "Workshop discussion, editable stakeholder diagrams, and fast visual handoff.",
      notBestFor:
        "Exact long-term source of truth because edits can drift away from code.",
      suitability: 7,
      maintenance: 5,
      comprehension: 8,
      recommendation:
        "Use for presentation and review sessions, then fold accepted changes back into Mermaid/data files.",
    },
    {
      id: "graphify",
      label: "Graphify knowledge graph",
      bestFor:
        "Entity relationship discovery across documents and code summaries.",
      notBestFor:
        "Sample-accurate signal flow or precise feedback loop timing.",
      suitability: 6,
      maintenance: 8,
      comprehension: 7,
      recommendation:
        "Use as an ingestion/discovery layer; curate its output before treating it as an engineering diagram.",
    },
  ],
};
