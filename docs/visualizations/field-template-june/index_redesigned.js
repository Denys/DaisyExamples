const mermaidConfig = {
  startOnLoad: false,
  securityLevel: "loose"
};

function getMermaidConfig() {
  const isBright = document.documentElement.classList.contains("theme-bright");
  return {
    ...mermaidConfig,
    theme: isBright ? "default" : "dark",
    themeVariables: isBright ? {
      fontFamily: "Plus Jakarta Sans, sans-serif",
      primaryColor: "#f1f5f9",
      primaryTextColor: "#0f172a",
      primaryBorderColor: "#cbd5e1",
      lineColor: "#64748b",
      secondaryColor: "#e0f2fe",
      tertiaryColor: "#fef3c7"
    } : {
      fontFamily: "Plus Jakarta Sans, sans-serif",
      primaryColor: "rgba(30, 41, 59, 0.75)",
      primaryTextColor: "#f8fafc",
      primaryBorderColor: "rgba(20, 184, 166, 0.4)",
      lineColor: "#94a3b8",
      secondaryColor: "rgba(14, 165, 233, 0.15)",
      tertiaryColor: "rgba(245, 158, 11, 0.15)"
    }
  };
}

if (window.mermaid) {
  window.mermaid.initialize(getMermaidConfig());
}

function escapeHtml(value) {
  return value.replace(/[&<>"']/g, char => ({
    "&": "&amp;",
    "<": "&lt;",
    ">": "&gt;",
    "\"": "&quot;",
    "'": "&#39;"
  }[char]));
}

function captureMermaidSources() {
  document.querySelectorAll(".mermaid").forEach(diagram => {
    if (!diagram.dataset.source) {
      diagram.dataset.source = diagram.textContent.trim();
    }
  });
}

let mermaidCounter = 0;

async function renderMermaidInPage(page) {
  if (!window.mermaid || !page) {
    return;
  }
  const diagrams = page.querySelectorAll(".mermaid");
  for (const diagram of diagrams) {
    const source = diagram.dataset.source || diagram.textContent.trim();
    const existingSvg = diagram.querySelector("svg");
    if (diagram.dataset.rendered === "true" && existingSvg) {
      continue;
    }
    try {
      const renderId = `field-template-june-${Date.now()}-${mermaidCounter++}`;
      // Re-initialize with matching theme variables
      window.mermaid.initialize(getMermaidConfig());
      const rendered = await window.mermaid.render(renderId, source);
      diagram.innerHTML = rendered.svg;
      diagram.dataset.rendered = "true";
      if (rendered.bindFunctions) {
        rendered.bindFunctions(diagram);
      }
      applyMermaidZoom(diagram);
    } catch (error) {
      console.error("Mermaid render failed", error);
      diagram.dataset.rendered = "error";
      diagram.innerHTML = `<div style="padding:10px; border:1px solid var(--rose); background:var(--rose-soft); border-radius:6px; font-size:12px; margin-bottom:10px;">Mermaid render failed. Source retained below.</div><pre style="background:rgba(0,0,0,0.3); padding:10px; border-radius:6px; overflow:auto; font-size:12px;">${escapeHtml(source)}</pre>`;
    }
  }
}

function applyMermaidZoom(diagram) {
  if (!diagram) {
    return;
  }
  const zoom = Number(diagram.dataset.zoom || 100);
  const svg = diagram.querySelector("svg");
  if (svg) {
    svg.style.width = `${zoom}%`;
    svg.style.maxWidth = "none";
    svg.style.height = "auto";
  }
}

function setMermaidZoom(button, nextZoom) {
  const box = button.closest(".diagram-box");
  const diagram = box ? box.querySelector(".mermaid") : null;
  if (!diagram) {
    return;
  }
  const boundedZoom = Math.max(65, Math.min(240, nextZoom));
  diagram.dataset.zoom = String(boundedZoom);
  applyMermaidZoom(diagram);
}

function setArchitectureView(name) {
  const selected = name === "synth" ? "synth" : "software";
  document.querySelectorAll("[data-arch-view]").forEach(button => {
    button.classList.toggle("is-active", button.dataset.archView === selected);
  });
  document.querySelectorAll("[data-arch-panel]").forEach(panel => {
    panel.classList.toggle("is-active", panel.dataset.archPanel === selected);
  });
  const panel = document.querySelector(`[data-arch-panel="${selected}"]`);
  if (panel) {
    renderMermaidInPage(panel);
  }
  setGraph(selected);
}

function activeBlockMap() {
  return document.querySelector(".block-view.is-active .synth-map");
}

function setBlockView(name) {
  const selected = name === "software" ? "software" : "synth";
  document.querySelectorAll("[data-block-view]").forEach(button => {
    button.classList.toggle("is-active", button.dataset.blockView === selected);
  });
  document.querySelectorAll("[data-block-panel]").forEach(panel => {
    panel.classList.toggle("is-active", panel.dataset.blockPanel === selected);
  });
  if (selected === "synth") {
    window.setTimeout(fitActiveBlockMapOnce, 30);
  }
}

function setBlockZoom(nextZoom) {
  const map = activeBlockMap();
  if (!map) {
    return;
  }
  const bounded = Math.max(0.55, Math.min(1.65, nextZoom));
  map.dataset.blockZoom = String(Math.round(bounded * 100));
  map.style.setProperty("--map-scale", bounded.toFixed(2));
  const viewport = map.closest(".synth-map-viewport");
  if (viewport) {
    viewport.style.minHeight = `${Math.max(520, Math.ceil(760 * bounded) + 28)}px`;
  }
}

function fitActiveBlockMapOnce() {
  const map = activeBlockMap();
  if (!map || map.dataset.fitApplied === "true") {
    return;
  }
  const viewport = map.closest(".synth-map-viewport");
  if (!viewport || viewport.clientWidth < 320) {
    return;
  }
  const fitScale = Math.min(1, (viewport.clientWidth - 32) / 1660);
  setBlockZoom(fitScale);
  map.dataset.fitApplied = "true";
}

function currentBlockZoom() {
  const map = activeBlockMap();
  return map ? Number(map.dataset.blockZoom || 100) / 100 : 1;
}

function setBlockExpanded(button) {
  const card = button.closest(".block-diagram-card");
  const activePanel = card ? card.querySelector(".block-view.is-active") : null;
  if (!card || !activePanel) {
    return;
  }
  let shell = document.querySelector(".map-expanded-shell");
  if (shell) {
    const content = shell.firstElementChild;
    shell.replaceWith(content);
    document.body.classList.remove("has-expanded");
    document.querySelectorAll("[data-block-action='expand']").forEach(item => {
      item.textContent = "Expand";
    });
    return;
  }
  shell = document.createElement("div");
  shell.className = "map-expanded-shell";
  activePanel.replaceWith(shell);
  shell.appendChild(activePanel);
  document.body.classList.add("has-expanded");
  button.textContent = "Exit Full";
}

function delayDiagramImage(button) {
  const card = button.closest(".delay-diagram-card");
  return card ? card.querySelector("[data-delay-image]") : null;
}

function currentDelayDiagramZoom(button) {
  const image = delayDiagramImage(button);
  return image ? Number(image.dataset.delayZoom || 100) / 100 : 1;
}

function setDelayDiagramZoom(button, nextZoom) {
  const image = delayDiagramImage(button);
  if (!image) {
    return;
  }
  const bounded = Math.max(0.35, Math.min(1.8, nextZoom));
  const baseWidth = Number(image.dataset.delayWidth || image.naturalWidth || 1660);
  image.dataset.delayZoom = String(Math.round(bounded * 100));
  image.style.width = `${Math.round(baseWidth * bounded)}px`;
}

function fitDelayDiagram(button) {
  const image = delayDiagramImage(button);
  const frame = image ? image.closest(".delay-diagram-frame") : null;
  if (!image || !frame || frame.clientWidth < 320) {
    return;
  }
  const baseWidth = Number(image.dataset.delayWidth || image.naturalWidth || 1660);
  const fitScale = Math.min(1, (frame.clientWidth - 28) / baseWidth);
  setDelayDiagramZoom(button, fitScale);
}

function fitDelayDiagramsInPage(page) {
  page.querySelectorAll(".delay-diagram-card [data-delay-action='fit']").forEach(button => {
    fitDelayDiagram(button);
  });
}

function setDelayDiagramExpanded(button) {
  const card = button.closest(".delay-diagram-card");
  if (!card) {
    return;
  }
  const expanded = !card.classList.contains("is-expanded");
  document.querySelectorAll(".delay-diagram-card.is-expanded").forEach(item => {
    if (item !== card) {
      item.classList.remove("is-expanded");
      const expandButton = item.querySelector("[data-delay-action='expand']");
      if (expandButton) {
        expandButton.textContent = "Expand";
      }
    }
  });
  card.classList.toggle("is-expanded", expanded);
  document.body.classList.toggle("has-expanded", expanded);
  button.textContent = expanded ? "Exit Full" : "Expand";
  window.setTimeout(() => fitDelayDiagram(button), 40);
}

/* Redesigned High-Visibility GoJS Color Schemes */
const nodeStyles = {
  input: { fill: "#1e293b", stroke: "#94a3b8", chip: "#94a3b8" },
  loop: { fill: "#1e3a8a", stroke: "#3b82f6", chip: "#3b82f6" },
  state: { fill: "#581c87", stroke: "#a855f7", chip: "#a855f7" },
  visual: { fill: "#115e59", stroke: "#14b8a6", chip: "#14b8a6" },
  audio: { fill: "#064e3b", stroke: "#10b981", chip: "#10b981" },
  guard: { fill: "#7f1d1d", stroke: "#ef4444", chip: "#ef4444" },
  control: { fill: "#1e3a8a", stroke: "#3b82f6", chip: "#3b82f6" },
  generator: { fill: "#7c2d12", stroke: "#ea580c", chip: "#ea580c" },
  mod: { fill: "#581c87", stroke: "#a855f7", chip: "#a855f7" },
  filter: { fill: "#115e59", stroke: "#14b8a6", chip: "#14b8a6" },
  output: { fill: "#7f1d1d", stroke: "#f43f5e", chip: "#f43f5e" },
  neutral: { fill: "#334155", stroke: "#64748b", chip: "#64748b" }
};

const lightNodeStyles = {
  input: { fill: "#f1f5f9", stroke: "#64748b", chip: "#64748b" },
  loop: { fill: "#dbeafe", stroke: "#2563eb", chip: "#2563eb" },
  state: { fill: "#f3e8ff", stroke: "#7c3aed", chip: "#7c3aed" },
  visual: { fill: "#ccfbf1", stroke: "#0d9488", chip: "#0d9488" },
  audio: { fill: "#dcfce7", stroke: "#16a34a", chip: "#16a34a" },
  guard: { fill: "#fee2e2", stroke: "#dc2626", chip: "#dc2626" },
  control: { fill: "#dbeafe", stroke: "#2563eb", chip: "#2563eb" },
  generator: { fill: "#ffedd5", stroke: "#ea580c", chip: "#ea580c" },
  mod: { fill: "#f3e8ff", stroke: "#7c3aed", chip: "#7c3aed" },
  filter: { fill: "#ccfbf1", stroke: "#0d9488", chip: "#0d9488" },
  output: { fill: "#fee2e2", stroke: "#f43f5e", chip: "#f43f5e" },
  neutral: { fill: "#f8fafc", stroke: "#94a3b8", chip: "#94a3b8" }
};

const linkStyles = {
  control: { stroke: "#3b82f6", width: 2, dash: null },
  state: { stroke: "#a855f7", width: 2, dash: [6, 4] },
  event: { stroke: "#ea580c", width: 2, dash: null },
  schedule: { stroke: "#94a3b8", width: 1.5, dash: [3, 5] },
  visual: { stroke: "#14b8a6", width: 2, dash: null },
  audio: { stroke: "#10b981", width: 2.8, dash: null },
  mod: { stroke: "#a855f7", width: 2.2, dash: [2, 3] },
  guard: { stroke: "#ef4444", width: 2.5, dash: null }
};

const lightLinkStyles = {
  control: { stroke: "#2563eb", width: 2, dash: null },
  state: { stroke: "#7c3aed", width: 2, dash: [6, 4] },
  event: { stroke: "#ea580c", width: 2, dash: null },
  schedule: { stroke: "#64748b", width: 1.5, dash: [3, 5] },
  visual: { stroke: "#0d9488", width: 2, dash: null },
  audio: { stroke: "#16a34a", width: 2.8, dash: null },
  mod: { stroke: "#7c3aed", width: 2.2, dash: [2, 3] },
  guard: { stroke: "#dc2626", width: 2.5, dash: null }
};

function getThemeNodeStyle(kind, prop) {
  const isBright = document.documentElement.classList.contains("theme-bright");
  const themeSet = isBright ? lightNodeStyles : nodeStyles;
  return (themeSet[kind] || themeSet.neutral)[prop];
}

function getThemeLinkStyle(kind, prop) {
  const isBright = document.documentElement.classList.contains("theme-bright");
  const themeSet = isBright ? lightLinkStyles : linkStyles;
  return (themeSet[kind] || themeSet.control)[prop];
}

const graphData = {
  software: {
    caption: "Software runtime explorer: actual function ownership, stored-state handoff, visual-rate caps, and audio callback boundaries.",
    legend: ["input", "loop", "state", "visual", "audio", "guard"],
    groups: [
      { key: "gInputs", text: "Inputs", kind: "input" },
      { key: "gMain", text: "1 ms Main Loop", kind: "loop" },
      { key: "gState", text: "Stored State", kind: "state" },
      { key: "gVisual", text: "Visual Outputs", kind: "visual" },
      { key: "gAudio", text: "Audio Callback", kind: "audio" }
    ],
    nodes: [
      { key: "knobs", text: "K1-K8 ADC knobs", role: "Raw physical values", group: "gInputs", kind: "input", cadence: "sampled in main loop", owner: "ProcessAllControls", controls: "K1-K8", detail: "Raw ADC values are never treated as the displayed parameter value until the active context captures movement." },
      { key: "switches", text: "SW1 / SW2", role: "Mode and modifier buttons", group: "gInputs", kind: "input", cadence: "edge processed", owner: "ProcessUi", controls: "SW1 toggles Main/Alt. SW2 toggles B-row Controls/Performance and acts as Level hold while pressed.", detail: "SW1 is a toggle, not hold. SW2 has both a rising-edge mode toggle and a held hidden Level modifier." },
      { key: "keybed", text: "A/B keybed", role: "Wave, transpose, modes, performance notes", group: "gInputs", kind: "input", cadence: "main loop", owner: "HandleKeybedControls", controls: "A1-A8, B1-B8", detail: "A-row controls synth modes. B-row is controls or performance notes depending on SW2 state." },
      { key: "midiIn", text: "External MIDI", role: "Note, velocity, sustain", group: "gInputs", kind: "input", cadence: "bounded event drain", owner: "ProcessMidiEvent", controls: "MIDI note on/off, CC64", detail: "The main loop drains at most 16 MIDI events per tick to bound non-audio work." },
      { key: "serialIn", text: "USB serial", role: "Test and telemetry commands", group: "gInputs", kind: "input", cadence: "main loop", owner: "ProcessSerialCommands", controls: "HELP, STATUS, SNAP, SELFTEST", detail: "Serial commands are main-loop only and do not run in the audio callback." },

      { key: "scan", text: "ProcessAllControls", role: "Read Field hardware state", group: "gMain", kind: "loop", cadence: "1 ms", owner: "main()", detail: "Scans knobs, switches, and keybed before UI processing." },
      { key: "serialCmd", text: "ProcessSerialCommands", role: "Non-audio command parser", group: "gMain", kind: "loop", cadence: "1 ms", owner: "main()", detail: "Handles diagnostic commands and can mark display/LED state dirty." },
      { key: "midiTick", text: "MIDI event tick", role: "Drain up to 16 events", group: "gMain", kind: "loop", cadence: "1 ms cap", owner: "main()", detail: "Prevents unbounded MIDI processing from stretching the main loop." },
      { key: "processUi", text: "ProcessUi", role: "Context, banks, key logic, dirty flags", group: "gMain", kind: "loop", cadence: "1 ms", owner: "main()", detail: "Samples raw knobs, handles SW1/SW2 behavior, applies until-touched gates, and updates UI outputs." },
      { key: "touchGate", text: "Until-touched gate", role: "Anchor + capture mask guard", group: "gMain", kind: "guard", cadence: "per active control", owner: "ProcessBankKnobs / ProcessOutputLevel", controls: "All banked knobs and hidden Level", detail: "A value writes only after movement exceeds 0.012 from the context-entry anchor; further writes use 0.0005 deadband." },
      { key: "keyLogic", text: "Keybed controls", role: "Mode state and test notes", group: "gMain", kind: "loop", cadence: "1 ms", owner: "HandleKeybedControls", controls: "A1-A8, B1-B8", detail: "Updates waveform, velocity mode, key tracking, LFO target, glide mode, transpose, reset/panic, or B-row performance notes." },
      { key: "applyVoice", text: "ApplyVoiceSetup", role: "Setter-only synth update", group: "gMain", kind: "loop", cadence: "only when voice_dirty", owner: "ProcessUi", detail: "Updates oscillator wave/PW/amp, ADSR times, and LFO frequency outside the audio callback when mode/state changes require it." },
      { key: "visualSched", text: "UpdateUiOutputs", role: "Dirty visual scheduler", group: "gMain", kind: "visual", cadence: "1 ms checks", owner: "ProcessUi", detail: "Runs display/LED work only when dirty or when the cap interval has elapsed." },
      { key: "loopDelay", text: "System::Delay(1)", role: "Loop pacing", group: "gMain", kind: "loop", cadence: "1 ms", owner: "main()", detail: "The main loop is intentionally paced; audio is driven by its callback, not by this delay." },

      { key: "banks", text: "ParamBankSet", role: "Main / Alt / hidden Level", group: "gState", kind: "state", cadence: "stored logical values", owner: "ProcessUi", controls: "K1-K8 Main, K1-K8 Alt, SW2+K8 Level", detail: "The displayed and heard parameters come from stored bank values, not from raw knob position." },
      { key: "anchors", text: "Entry anchors", role: "Raw reference positions", group: "gState", kind: "state", cadence: "context entry", owner: "RecordBankAnchors / ResetOutputLevelAnchor", detail: "Separate anchors prevent old physical knob positions from jumping newly selected parameters." },
      { key: "synthState", text: "SynthState", role: "Wave, modes, transpose, level", group: "gState", kind: "state", cadence: "event updates", owner: "Keybed and UI logic", detail: "Holds waveform, velocity mode, key tracking, LFO target, glide mode, transpose, and output level." },
      { key: "midiState", text: "MidiState", role: "Note, gate, velocity, sustain", group: "gState", kind: "state", cadence: "MIDI/event updates", owner: "MIDI and performance keys", detail: "The audio callback reads current note/gate/velocity and sustain state from here." },
      { key: "dirtyFlags", text: "Dirty flags", role: "Voice / display / LEDs", group: "gState", kind: "state", cadence: "set on changes", owner: "UI logic", detail: "Dirty flags decouple event changes from expensive visual or setter work." },

      { key: "oled", text: "OLED renderer", role: "Overview and focus zoom", group: "gVisual", kind: "visual", cadence: "50 ms cap", owner: "RenderDisplay", detail: "Displays actual stored parameter values, SW2 mode, focus, and mode selections." },
      { key: "knobLeds", text: "Knob LEDs", role: "Stored value bars", group: "gVisual", kind: "visual", cadence: "16 ms cap", owner: "UpdateKnobLeds", detail: "Shows logical bank values after capture, not raw knob locations." },
      { key: "keyLeds", text: "Key LEDs", role: "Selections and tri-state modes", group: "gVisual", kind: "visual", cadence: "16 ms cap", owner: "UpdateKeyLeds", detail: "Shows waveform, transpose, velocity/keytrack/LFO/glide selections, and performance mode state." },

          { key: "audioCb", text: "AudioCallback", role: "Audio-only execution lane", group: "gAudio", kind: "audio", cadence: "48 samples", owner: "Daisy audio engine", detail: "Reads stored state and performs DSP. It does not scan controls, parse serial commands, or draw OLED." },
          { key: "voiceDsp", text: "Mono synth DSP", role: "Osc, sub, noise, env, LFO, filter, drive", group: "gAudio", kind: "audio", cadence: "per sample", owner: "AudioCallback", detail: "Computes frequency glide, LFO, envelope, filter cutoff/resonance, tanh drive, and stereo output." },
          { key: "dac", text: "Stereo DAC out", role: "Audio output", group: "gAudio", kind: "audio", cadence: "per sample", owner: "AudioCallback", detail: "Writes identical shaped signal to left and right output channels." }
    ],
    links: [
          { from: "knobs", to: "scan", text: "raw sample", kind: "control" },
          { from: "switches", to: "scan", text: "edge state", kind: "control" },
          { from: "keybed", to: "scan", text: "button state", kind: "control" },
          { from: "scan", to: "serialCmd", text: "loop order", kind: "schedule" },
          { from: "serialIn", to: "serialCmd", text: "commands", kind: "event" },
          { from: "serialCmd", to: "midiTick", text: "loop order", kind: "schedule" },
          { from: "midiIn", to: "midiTick", text: "queued MIDI", kind: "event" },
          { from: "midiTick", to: "midiState", text: "note/gate/cc64", kind: "state" },
          { from: "midiTick", to: "processUi", text: "loop order", kind: "schedule" },
          { from: "processUi", to: "anchors", text: "record context entry", kind: "state" },
          { from: "anchors", to: "touchGate", text: "compare raw movement", kind: "guard" },
          { from: "processUi", to: "touchGate", text: "active bank/modifier", kind: "control" },
          { from: "touchGate", to: "banks", text: "write after movement", kind: "guard" },
          { from: "processUi", to: "keyLogic", text: "A/B actions", kind: "control" },
          { from: "keyLogic", to: "synthState", text: "modes and transpose", kind: "state" },
          { from: "banks", to: "dirtyFlags", text: "param changed", kind: "state" },
          { from: "keyLogic", to: "dirtyFlags", text: "mode changed", kind: "event" },
          { from: "dirtyFlags", to: "applyVoice", text: "voice_dirty", kind: "event" },
          { from: "applyVoice", to: "synthState", text: "setter state", kind: "state" },
          { from: "dirtyFlags", to: "visualSched", text: "display/LED dirty", kind: "visual" },
          { from: "visualSched", to: "oled", text: "50 ms cap", kind: "visual" },
          { from: "visualSched", to: "knobLeds", text: "16 ms cap", kind: "visual" },
          { from: "visualSched", to: "keyLeds", text: "16 ms cap", kind: "visual" },
          { from: "visualSched", to: "loopDelay", text: "loop tail", kind: "schedule" },
          { from: "banks", to: "audioCb", text: "stored params", kind: "state" },
          { from: "synthState", to: "audioCb", text: "modes/level", kind: "state" },
          { from: "midiState", to: "audioCb", text: "note/gate", kind: "state" },
          { from: "audioCb", to: "voiceDsp", text: "per-sample DSP", kind: "audio" },
          { from: "voiceDsp", to: "dac", text: "stereo samples", kind: "audio" }
    ]
  },
  synth: {
    caption: "Synth voice explorer: actual signal path plus mapped Field controls for generators, modulators, filter, amp, drive, and output.",
    legend: ["input", "generator", "mod", "filter", "audio", "output", "control"],
    groups: [
      { key: "gNote", text: "Note and Gate Sources", kind: "input" },
      { key: "gControl", text: "Mapped UI Parameters", kind: "loop" },
      { key: "gGen", text: "Sound Generators", kind: "generator" },
      { key: "gMod", text: "Modulators", kind: "mod" },
      { key: "gShape", text: "Filter, Amp, Drive", kind: "filter" },
      { key: "gOut", text: "Output and Feedback", kind: "output" }
    ],
    nodes: [
          { key: "midiNotes", text: "MIDI notes", role: "External note, velocity, sustain", group: "gNote", kind: "input", cadence: "event-driven", controls: "MIDI NoteOn/Off, CC64", owner: "ProcessMidiEvent", detail: "Provides primary playable note source and velocity. Sustain can hold gate after note release." },
          { key: "perfNotes", text: "B-row performance notes", role: "Built-in test keyboard", group: "gNote", kind: "input", cadence: "main loop", controls: "SW2 Performance; B1-B8 = C4-C5", owner: "HandleKeybedControls", detail: "A test-performance mode routes B-row keys to MIDI note behavior for quick hardware checks." },
          { key: "pitch", text: "Pitch and glide", role: "Target frequency smoothing", group: "gNote", kind: "state", cadence: "per sample", controls: "B1-B4 transpose, A8 mode, K4 Alt Glide", owner: "AudioCallback", detail: "Frequency slews toward target using the current glide mode and amount; transpose shifts note source." },

          { key: "waveKeys", text: "Wave select", role: "Oscillator waveform", group: "gControl", kind: "loop", controls: "A1 Sine, A2 Tri, A3 Saw, A4 Square", owner: "Keybed controls", detail: "Sets the main oscillator waveform; sub waveform follows with square preserved, otherwise saw." },
          { key: "color", text: "Color", role: "Pulse width and source tint", group: "gControl", kind: "loop", controls: "K8 Main", owner: "ParamBankSet Main", detail: "Sets oscillator pulse width from about 0.08 to 0.92 and scales source tone from 0.6 to 1.0." },
          { key: "subControl", text: "Sub", role: "Sub oscillator level", group: "gControl", kind: "loop", controls: "SW1 Alt, K8", owner: "ParamBankSet Alt", detail: "Controls sub oscillator amplitude and mix contribution." },
          { key: "noiseControl", text: "Noise", role: "White-noise mix", group: "gControl", kind: "loop", controls: "SW1 Alt, K7", owner: "ParamBankSet Alt", detail: "Adds up to 30 percent white noise before the filter." },
          { key: "filterControl", text: "Filter controls", role: "Cutoff, resonance, env, keytrack", group: "gControl", kind: "loop", controls: "K1/K2 Main, K1 Alt, A6", owner: "ParamBankSet + SynthState", detail: "Base cutoff and resonance are main-bank controls; EnvAmt and KeyTrack add dynamic cutoff movement." },
          { key: "envControl", text: "ADSR controls", role: "Envelope shape", group: "gControl", kind: "loop", controls: "K3-K6 Main", owner: "ParamBankSet Main", detail: "Attack, decay, sustain, and release are set in ApplyVoiceSetup when dirty." },
          { key: "lfoControl", text: "LFO controls", role: "Rate, depth, target", group: "gControl", kind: "loop", controls: "K2/K3 Alt, A7", owner: "ParamBankSet + SynthState", detail: "A7 selects Off/Pitch/Filter; K2 Alt sets rate and K3 Alt sets depth." },
          { key: "ampControl", text: "Amp controls", role: "Velocity, drive, output level", group: "gControl", kind: "loop", controls: "A5, K5 Alt, K7 Main, SW2+K8", owner: "ParamBankSet + SynthState", detail: "A5/K5 Alt shape velocity behavior; K7 Main sets tanh drive; SW2+K8 edits hidden Level." },

          { key: "osc", text: "Main oscillator", role: "Primary tone source", group: "gGen", kind: "generator", cadence: "per sample", owner: "AudioCallback", detail: "Uses selected waveform, color/PW, current frequency, and optional LFO modulation." },
          { key: "sub", text: "Sub oscillator", role: "One-octave support", group: "gGen", kind: "generator", cadence: "per sample", owner: "AudioCallback", detail: "Runs at half the main oscillator frequency and follows the main waveform family." },
          { key: "noise", text: "WhiteNoise", role: "Noise layer", group: "gGen", kind: "generator", cadence: "per sample", owner: "AudioCallback", detail: "Mixed before filtering with amount from Alt Noise." },
          { key: "premix", text: "Pre-filter mix", role: "Osc + sub + noise sum", group: "gGen", kind: "generator", cadence: "per sample", owner: "AudioCallback", detail: "Combines the sound generators before envelope amplitude and filtering." },

          { key: "env", text: "ADSR envelope", role: "Amp and cutoff contour", group: "gMod", kind: "mod", cadence: "per sample", controls: "K3-K6 Main", owner: "Adsr", detail: "Processes gate from MIDI or B-row performance notes and feeds both amplitude and filter cutoff." },
          { key: "lfo", text: "Sine LFO", role: "Pitch or filter modulation", group: "gMod", kind: "mod", cadence: "per sample", controls: "K2/K3 Alt, A7", owner: "Oscillator lfo", detail: "Modulates pitch by a small ratio or filter cutoff by a Hz offset depending on A7 target." },
          { key: "velocity", text: "Velocity scale", role: "Amplitude response", group: "gMod", kind: "mod", cadence: "per sample", controls: "A5, K5 Alt, MIDI velocity", owner: "VelocityScale", detail: "Combines MIDI velocity and selected velocity mode before final amp scaling." },

          { key: "amp", text: "Envelope amp", role: "Source * envelope * velocity", group: "gShape", kind: "audio", cadence: "per sample", owner: "AudioCallback", detail: "Applies the envelope and velocity scale before the filter." },
          { key: "filter", text: "SVF low-pass", role: "Tone shaping", group: "gShape", kind: "filter", cadence: "per sample", controls: "K1/K2 Main, K1 Alt, A6, A7", owner: "Svf", detail: "Cutoff combines base value, envelope amount, key tracking, and optional LFO. Resonance maps from 0.15 to 0.95." },
          { key: "drive", text: "tanh drive", role: "Soft clipping", group: "gShape", kind: "audio", cadence: "per sample", controls: "K7 Main", owner: "AudioCallback", detail: "Drive gain maps from 1x to 8x before tanh shaping." },
          { key: "level", text: "Output level", role: "Final gain", group: "gShape", kind: "output", cadence: "per sample", controls: "hold SW2 + K8", owner: "SynthState", detail: "Hidden Level scales the final driven signal before stereo output." },

          { key: "stereoOut", text: "Stereo output", role: "Left/right DAC samples", group: "gOut", kind: "output", cadence: "per sample", owner: "AudioCallback", detail: "The current synth voice writes the same shaped signal to both output channels." },
          { key: "uiFeedback", text: "OLED and LEDs", role: "Stored value feedback", group: "gOut", kind: "visual", cadence: "50 ms / 16 ms caps", owner: "UpdateUiOutputs", detail: "Feedback reflects stored logical values and selections, including SW2 Controls/Performance state." }
    ],
    links: [
          { from: "midiNotes", to: "pitch", text: "note/velocity", kind: "event" },
          { from: "perfNotes", to: "pitch", text: "test note", kind: "event" },
          { from: "waveKeys", to: "osc", text: "waveform", kind: "control" },
          { from: "color", to: "osc", text: "PW/tint", kind: "control" },
          { from: "subControl", to: "sub", text: "level", kind: "control" },
          { from: "noiseControl", to: "noise", text: "mix", kind: "control" },
          { from: "pitch", to: "osc", text: "freq", kind: "audio" },
          { from: "pitch", to: "sub", text: "freq / 2", kind: "audio" },
          { from: "osc", to: "premix", text: "audio", kind: "audio" },
          { from: "sub", to: "premix", text: "audio", kind: "audio" },
          { from: "noise", to: "premix", text: "audio", kind: "audio" },
          { from: "midiNotes", to: "env", text: "gate", kind: "event" },
          { from: "perfNotes", to: "env", text: "gate", kind: "event" },
          { from: "envControl", to: "env", text: "A/D/S/R", kind: "control" },
          { from: "lfoControl", to: "lfo", text: "rate/depth/target", kind: "control" },
          { from: "lfo", to: "osc", text: "pitch target", kind: "mod" },
          { from: "lfo", to: "sub", text: "pitch target", kind: "mod" },
          { from: "velocity", to: "amp", text: "velocity scale", kind: "mod" },
          { from: "ampControl", to: "velocity", text: "mode/amount", kind: "control" },
          { from: "premix", to: "amp", text: "source", kind: "audio" },
          { from: "env", to: "amp", text: "amplitude", kind: "mod" },
          { from: "amp", to: "filter", text: "audio", kind: "audio" },
          { from: "filterControl", to: "filter", text: "cutoff/res/env/keytrack", kind: "control" },
          { from: "env", to: "filter", text: "cutoff env", kind: "mod" },
          { from: "lfo", to: "filter", text: "filter target", kind: "mod" },
          { from: "filter", to: "drive", text: "low-pass", kind: "audio" },
          { from: "ampControl", to: "drive", text: "drive", kind: "control" },
          { from: "drive", to: "level", text: "shaped audio", kind: "audio" },
          { from: "ampControl", to: "level", text: "hidden level", kind: "control" },
          { from: "level", to: "stereoOut", text: "stereo samples", kind: "audio" },
          { from: "waveKeys", to: "uiFeedback", text: "selection", kind: "visual" },
          { from: "filterControl", to: "uiFeedback", text: "stored values", kind: "visual" },
          { from: "envControl", to: "uiFeedback", text: "stored values", kind: "visual" },
          { from: "ampControl", to: "uiFeedback", text: "stored values", kind: "visual" }
    ]
  }
};

function renderFallbackGraph(container, graph) {
  if (!container || !graph) {
    return;
  }
  const nodeHtml = graph.nodes.map(node => `<div class="fallback-node" style="padding:8px; margin:4px; border-radius:6px; background:rgba(255,255,255,0.05); border:1px solid rgba(255,255,255,0.1); font-size:12px;">${escapeHtml(node.text)}</div>`).join("");
  const linkHtml = graph.links.map(link => {
    const from = graph.nodes.find(node => node.key === link.from);
    const to = graph.nodes.find(node => node.key === link.to);
    return `<div style="font-size:11.5px; color:var(--text-muted); padding:2px 0;">${escapeHtml(from ? from.text : link.from)} → ${escapeHtml(to ? to.text : link.to)} <code style="font-size:10px;">${escapeHtml(link.text || "")}</code></div>`;
  }).join("");
  container.innerHTML = `<div style="padding:15px;"><div style="display:flex; flex-wrap:wrap; margin-bottom:15px;">${nodeHtml}</div><div>${linkHtml}</div></div>`;
}

function graphNodesWithGroups(graph) {
  const groups = (graph.groups || []).map(group => ({ ...group, isGroup: true }));
  return [...groups, ...(graph.nodes || [])];
}

function graphNodeByKey(graph, key) {
  return graphNodesWithGroups(graph).find(node => node.key === key);
}

function renderGraphLegend(graph) {
  const legend = document.getElementById("gojsLegend");
  if (!legend || !graph) {
    return;
  }
  const kinds = graph.legend || [];
  legend.innerHTML = kinds.map(kind => {
    const style = getThemeNodeStyle(kind, "stroke");
    const label = kind.charAt(0).toUpperCase() + kind.slice(1);
    return `<span class="legend-chip" style="--chip-color:${style}">${escapeHtml(label)}</span>`;
  }).join("");
  const linkKinds = [...new Set((graph.links || []).map(link => link.kind || "control"))];
  legend.innerHTML += linkKinds.map(kind => {
    const style = getThemeLinkStyle(kind, "stroke");
    const label = `${kind.charAt(0).toUpperCase()}${kind.slice(1)} Link`;
    return `<span class="legend-chip" style="--chip-color:${style}">${escapeHtml(label)}</span>`;
  }).join("");
}

function renderGraphDetails(graph, selectedData) {
  const details = document.getElementById("gojsDetails");
  if (!details || !graph) {
    return;
  }
  if (!selectedData) {
    const nodeCount = graph.nodes ? graph.nodes.length : 0;
    const linkCount = graph.links ? graph.links.length : 0;
    details.innerHTML = `
      <h4>Graph Summary</h4>
      <p>${escapeHtml(graph.caption)}</p>
      <dl>
        <dt>Interactive Features</dt>
        <dd>Select any node or connecting link in the GoJS explorer above to view contextual parameters, cadences, and firmware function ownership details here.</dd>
        <dt>Graph Size</dt>
        <dd>${nodeCount} nodes, ${linkCount} links</dd>
      </dl>`;
    return;
  }
  if (selectedData.from && selectedData.to) {
    const from = graphNodeByKey(graph, selectedData.from);
    const to = graphNodeByKey(graph, selectedData.to);
    details.innerHTML = `
      <h4>Link Selection</h4>
      <p>${escapeHtml((from ? from.text : selectedData.from) + " → " + (to ? to.text : selectedData.to))}</p>
      <dl>
        <dt>Label</dt><dd>${escapeHtml(selectedData.text || "Direct routing")}</dd>
        <dt>Link Type</dt><dd>${escapeHtml(selectedData.kind || "control")}</dd>
      </dl>`;
    return;
  }
  details.innerHTML = `
    <h4>${escapeHtml(selectedData.text || "Node details")}</h4>
    <p>${escapeHtml(selectedData.role || selectedData.detail || "")}</p>
    <dl>
      <dt>Kind</dt><dd>${escapeHtml(selectedData.kind || "neutral")}</dd>
      <dt>Owner Logic</dt><dd><code>${escapeHtml(selectedData.owner || "n/a")}</code></dd>
      <dt>Sampling Rate</dt><dd>${escapeHtml(selectedData.cadence || "n/a")}</dd>
      <dt>Mapped Controls</dt><dd>${escapeHtml(selectedData.controls || "n/a")}</dd>
      <dt>Technical Role</dt><dd>${escapeHtml(selectedData.detail || "n/a")}</dd>
    </dl>`;
}

function initGoJS() {
  const container = document.getElementById("gojsDiagram");
  if (!container || !window.go) {
    return null;
  }
  const $ = go.GraphObject.make;
  const diagram = $(go.Diagram, "gojsDiagram", {
    initialAutoScale: go.Diagram.Uniform,
    layout: $(go.LayeredDigraphLayout, { direction: 0, layerSpacing: 54, columnSpacing: 34 }),
    "undoManager.isEnabled": false,
    allowCopy: false,
    allowDelete: false,
    allowMove: true,
    "toolManager.mouseWheelBehavior": go.ToolManager.WheelZoom
  });

  diagram.groupTemplate =
    $(go.Group, "Auto",
      {
        layout: $(go.LayeredDigraphLayout, { direction: 0, layerSpacing: 34, columnSpacing: 24 }),
        selectable: true,
        computesBoundsAfterDrag: true,
        handlesDragDropForMembers: true
      },
      $(go.Shape, "RoundedRectangle",
        { strokeWidth: 1.5, parameter1: 10 },
        new go.Binding("fill", "kind", kind => getThemeNodeStyle(kind, "fill")),
        new go.Binding("stroke", "kind", kind => getThemeNodeStyle(kind, "stroke"))),
      $(go.Panel, "Vertical",
        { margin: 10 },
        $(go.TextBlock,
          {
            alignment: go.Spot.Left,
            margin: new go.Margin(0, 0, 8, 0),
            font: "800 13px Plus Jakarta Sans, sans-serif",
            stroke: "currentColor"
          },
          new go.Binding("text", "text")),
        $(go.Placeholder, { padding: 12 })
      )
    );

  diagram.nodeTemplate =
    $(go.Node, "Auto",
      { minSize: new go.Size(180, 76), selectionAdorned: true },
      $(go.Shape, "RoundedRectangle",
        { strokeWidth: 1.2, parameter1: 8 },
        new go.Binding("fill", "", () => {
          const isBright = document.documentElement.classList.contains("theme-bright");
          return isBright ? "rgba(255,255,255,0.95)" : "rgba(30,41,59,0.85)";
        }),
        new go.Binding("stroke", "kind", kind => getThemeNodeStyle(kind, "stroke"))),
      $(go.Panel, "Vertical",
        { margin: new go.Margin(10, 12, 10, 12), defaultAlignment: go.Spot.Left },
        $(go.TextBlock,
          {
            wrap: go.TextBlock.WrapFit,
            maxSize: new go.Size(220, NaN),
            stroke: "currentColor",
            font: "700 12px Plus Jakarta Sans, sans-serif"
          },
          new go.Binding("text", "text")),
        $(go.TextBlock,
          {
            margin: new go.Margin(4, 0, 0, 0),
            wrap: go.TextBlock.WrapFit,
            maxSize: new go.Size(220, NaN),
            stroke: "currentColor",
            opacity: 0.7,
            font: "400 11px Plus Jakarta Sans, sans-serif"
          },
          new go.Binding("text", "role")),
        $(go.TextBlock,
          {
            margin: new go.Margin(6, 0, 0, 0),
            stroke: "#14b8a6",
            font: "500 9px JetBrains Mono, monospace"
          },
          new go.Binding("text", "cadence", cadence => cadence ? `RATE: ${cadence.toUpperCase()}` : ""),
          new go.Binding("visible", "cadence", cadence => !!cadence))
      )
    );

  diagram.linkTemplate =
    $(go.Link,
      { routing: go.Link.AvoidsNodes, corner: 8, curve: go.Link.JumpOver },
      $(go.Shape,
        { strokeWidth: 1.5 },
        new go.Binding("stroke", "kind", kind => getThemeLinkStyle(kind, "stroke")),
        new go.Binding("strokeWidth", "kind", kind => getThemeLinkStyle(kind, "width")),
        new go.Binding("strokeDashArray", "kind", kind => getThemeLinkStyle(kind, "dash"))),
      $(go.Shape,
        { toArrow: "Standard" },
        new go.Binding("stroke", "kind", kind => getThemeLinkStyle(kind, "stroke")),
        new go.Binding("fill", "kind", kind => getThemeLinkStyle(kind, "stroke"))),
      $(go.Panel, "Auto",
        $(go.Shape, "RoundedRectangle", { parameter1: 5 },
          new go.Binding("fill", "", () => {
            const isBright = document.documentElement.classList.contains("theme-bright");
            return isBright ? "rgba(241,245,249,0.95)" : "rgba(15,23,42,0.95)";
          }),
          new go.Binding("stroke", "", () => {
            const isBright = document.documentElement.classList.contains("theme-bright");
            return isBright ? "rgba(148,163,184,0.3)" : "rgba(148,163,184,0.2)";
          })
        ),
        $(go.TextBlock, { margin: new go.Margin(3, 6, 3, 6), font: "600 10px Plus Jakarta Sans, sans-serif", stroke: "currentColor", opacity: 0.8 }, new go.Binding("text", "text"))
      )
    );

  diagram.addDiagramListener("ChangedSelection", event => {
    const graph = graphData[diagram.currentGraphName || "software"] || graphData.software;
    const part = event.diagram.selection.first();
    renderGraphDetails(graph, part ? part.data : null);
  });
  return diagram;
}

let activeDiagram = null;

function setGraph(name) {
  const graphName = graphData[name] ? name : "software";
  const graph = graphData[graphName];
  const caption = document.getElementById("gojsCaption");
  if (caption) {
    caption.textContent = graph.caption;
  }
  document.querySelectorAll("[data-graph]").forEach(button => {
    button.classList.toggle("is-active", button.dataset.graph === graphName);
  });
  renderGraphLegend(graph);
  renderGraphDetails(graph, null);
  if (activeDiagram) {
    activeDiagram.currentGraphName = graphName;
    const model = new go.GraphLinksModel(graphNodesWithGroups(graph), graph.links);
    model.linkKeyProperty = "key";
    activeDiagram.model = model;
    window.setTimeout(() => {
      activeDiagram.requestUpdate();
      activeDiagram.zoomToFit();
    }, 30);
  } else {
    renderFallbackGraph(document.getElementById("gojsDiagram"), graph);
  }
}

function showPage(pageId) {
  const fallback = document.getElementById("overview");
  const target = document.getElementById(pageId) || fallback;
  document.querySelectorAll(".page").forEach(page => {
    page.classList.toggle("is-current", page === target);
  });
  document.querySelectorAll("[data-page-link]").forEach(link => {
    link.classList.toggle("active", link.dataset.pageLink === target.id);
  });
  if (target.id === "architecture" && activeDiagram) {
    activeDiagram.requestUpdate();
    activeDiagram.zoomToFit();
  }
  if (target.id === "architecture") {
    window.setTimeout(fitActiveBlockMapOnce, 30);
  }
  if (target.id === "delay-bundle") {
    window.setTimeout(() => fitDelayDiagramsInPage(target), 30);
  }
  renderMermaidInPage(target);
}

/* Dynamic Phantasmagoria Review Page Population */
function renderPhantasmagoriaPage() {
  const model = window.DelayBundleModel;
  if (!model) return;

  // 1. Render Candidate Algorithms
  const project = model.referenceProjects.find(p => p.id === "phantasmagoria");
  const candidates = project ? project.extractedAlgorithms : [];
  const candidatesGrid = document.getElementById("phantasmagoriaCandidatesGrid");
  if (candidatesGrid && candidates) {
    candidatesGrid.innerHTML = candidates.map(block => `
      <article class="card">
        <h3>${escapeHtml(block.name)}</h3>
        <span class="tag june" style="margin-bottom: 10px;">${escapeHtml(block.category)}</span>
        <p>${escapeHtml(block.role)}</p>
        <div style="margin-top: 12px; font-size: 12.5px; color: var(--text-muted);">
          <div style="margin-bottom: 2px;"><strong>Controls:</strong> ${escapeHtml(block.controls.join(", "))}</div>
          <div style="margin-bottom: 2px;"><strong>Inputs:</strong> ${escapeHtml(block.inputs.join(", "))}</div>
          <div style="margin-bottom: 2px;"><strong>Outputs:</strong> ${escapeHtml(block.outputs.join(", "))}</div>
          <div style="font-style: italic; margin-top: 6px; font-size:11.5px; opacity:0.85;">Source evidence: ${escapeHtml(block.evidence)}</div>
        </div>
      </article>
    `).join("");
  }

  // 2. Render Equivalence Table
  const review = model.literatureReviews.find(r => r.id === "phantasmagoria-equivalence");
  const tableBody = document.getElementById("phantasmagoriaTableBody");
  if (tableBody && review) {
    tableBody.innerHTML = review.equivalenceMatrix.map(row => `
      <div class="row">
        <div class="cell" style="font-weight:600; color:var(--text-primary);">${escapeHtml(row.extracted)}</div>
        <div class="cell">${escapeHtml(row.closest)}</div>
        <div class="cell"><span class="tag ${row.reusePriority === 'High' ? 'pass' : 'warn'}">${escapeHtml(row.reusePriority)}</span></div>
        <div class="cell" style="font-size:12.5px;">${escapeHtml(row.implementationNotes)}</div>
      </div>
    `).join("");
  }

  // 3. Render Synthesis Themes
  const themesGrid = document.getElementById("phantasmagoriaThemesGrid");
  if (themesGrid && review) {
    themesGrid.innerHTML = review.synthesisThemes.map(theme => `
      <div class="card" style="border-top: 3px solid var(--violet);">
        <h3>${escapeHtml(theme.theme)}</h3>
        <p>${escapeHtml(theme.finding)}</p>
        <div style="display:flex; flex-wrap:wrap; gap:6px; margin-top:12px;">
          ${theme.mappedAlgorithms.map(algo => `<span class="tag" style="font-size:11px;">${escapeHtml(algo)}</span>`).join("")}
          ${theme.closestBundleModes.map(mode => `<span class="tag pass" style="font-size:11px;">${escapeHtml(mode)}</span>`).join("")}
        </div>
      </div>
    `).join("");
  }

  // 4. Render Gaps & Recommendations
  const gapsList = document.getElementById("phantasmagoriaGapsList");
  if (gapsList && review) {
    gapsList.innerHTML = review.gaps.map(gap => `<li>${escapeHtml(gap)}</li>`).join("");
  }
  const recText = document.getElementById("phantasmagoriaRecommendation");
  if (recText && review) {
    recText.textContent = review.recommendation;
  }
}

document.addEventListener("DOMContentLoaded", () => {
  captureMermaidSources();
  activeDiagram = initGoJS();
  setGraph("software");
  renderPhantasmagoriaPage();

  // Theme Toggler
  const themeToggle = document.getElementById("themeToggle");
  const savedTheme = localStorage.getItem("theme") || "dark";

  if (savedTheme === "bright") {
    document.documentElement.classList.add("theme-bright");
  } else {
    document.documentElement.classList.remove("theme-bright");
  }

  // Re-evaluates GoJS when loading initially
  if (activeDiagram) {
    activeDiagram.requestUpdate();
  }

  themeToggle.addEventListener("click", () => {
    const isBright = document.documentElement.classList.toggle("theme-bright");
    localStorage.setItem("theme", isBright ? "bright" : "dark");

    // Refresh GoJS Diagram colors
    if (activeDiagram) {
      activeDiagram.requestUpdate();
      activeDiagram.zoomToFit();
      renderGraphLegend(graphData[activeDiagram.currentGraphName || "software"]);
    }

    // Force re-render of Mermaid diagrams in current page
    const currentPage = document.querySelector(".page.is-current");
    if (currentPage) {
      currentPage.querySelectorAll(".mermaid").forEach(diag => {
        diag.removeAttribute("data-processed");
        diag.dataset.rendered = "false";
      });
      renderMermaidInPage(currentPage);
    }
  });

  // Interaction handlers
  document.querySelectorAll("[data-block-view]").forEach(button => {
    button.addEventListener("click", () => {
      setBlockView(button.dataset.blockView);
    });
  });
  document.querySelectorAll("[data-block-action]").forEach(button => {
    button.addEventListener("click", () => {
      const action = button.dataset.blockAction;
      const map = activeBlockMap();
      if (action === "fit" && map) {
        const viewport = map.closest(".synth-map-viewport");
        const fitScale = viewport ? (viewport.clientWidth - 32) / 1660 : 0.8;
        setBlockZoom(fitScale);
      } else if (action === "readable" && map) {
        setBlockZoom(1);
      } else if (action === "zoom-in" && map) {
        setBlockZoom(currentBlockZoom() + 0.1);
      } else if (action === "zoom-out" && map) {
        setBlockZoom(currentBlockZoom() - 0.1);
      } else if (action === "expand") {
        setBlockExpanded(button);
      }
    });
  });
  document.querySelectorAll("[data-delay-action]").forEach(button => {
    button.addEventListener("click", () => {
      const action = button.dataset.delayAction;
      if (action === "fit") {
        fitDelayDiagram(button);
      } else if (action === "readable") {
        setDelayDiagramZoom(button, 1);
      } else if (action === "zoom-in") {
        setDelayDiagramZoom(button, currentDelayDiagramZoom(button) + 0.1);
      } else if (action === "zoom-out") {
        setDelayDiagramZoom(button, currentDelayDiagramZoom(button) - 0.1);
      } else if (action === "expand") {
        setDelayDiagramExpanded(button);
      }
    });
  });
  document.querySelectorAll("[data-arch-view]").forEach(button => {
    button.addEventListener("click", () => {
      setArchitectureView(button.dataset.archView);
    });
  });
  document.querySelectorAll("[data-mermaid-action]").forEach(button => {
    button.addEventListener("click", () => {
      const box = button.closest(".diagram-box");
      const diagram = box ? box.querySelector(".mermaid") : null;
      const currentZoom = Number(diagram ? diagram.dataset.zoom || 100 : 100);
      const action = button.dataset.mermaidAction;
      if (action === "fit") {
        setMermaidZoom(button, 100);
      } else if (action === "readable") {
        setMermaidZoom(button, 135);
      } else if (action === "zoom-in") {
        setMermaidZoom(button, currentZoom + 15);
      } else if (action === "zoom-out") {
        setMermaidZoom(button, currentZoom - 15);
      } else if (action === "expand" && box) {
        const expanded = !box.classList.contains("is-expanded");
        box.classList.toggle("is-expanded", expanded);
        document.body.classList.toggle("has-expanded", expanded);
        button.textContent = expanded ? "Exit Full" : "Expand";
      }
    });
  });
  document.querySelectorAll("[data-graph]").forEach(button => {
    button.addEventListener("click", () => {
      setGraph(button.dataset.graph);
    });
  });
  document.querySelectorAll("[data-gojs-action]").forEach(button => {
    button.addEventListener("click", () => {
      if (!activeDiagram) {
        return;
      }
      const action = button.dataset.gojsAction;
      if (action === "zoom-in") {
        activeDiagram.scale = Math.min(2.2, activeDiagram.scale * 1.18);
      } else if (action === "zoom-out") {
        activeDiagram.scale = Math.max(0.25, activeDiagram.scale / 1.18);
      } else if (action === "fit") {
        activeDiagram.zoomToFit();
      } else if (action === "expand") {
        const card = button.closest(".graph-card");
        if (card) {
          const expanded = !card.classList.contains("is-expanded");
          card.classList.toggle("is-expanded", expanded);
          document.body.classList.toggle("has-expanded", expanded);
          button.textContent = expanded ? "Exit Full" : "Expand";
          window.setTimeout(() => {
            activeDiagram.requestUpdate();
            activeDiagram.zoomToFit();
          }, 80);
        }
      }
    });
  });
  document.querySelectorAll("[data-page-link]").forEach(link => {
    link.addEventListener("click", event => {
      event.preventDefault();
      const pageId = link.dataset.pageLink;
      history.pushState(null, "", `#${pageId}`);
      showPage(pageId);
      window.scrollTo({ top: 0, behavior: "smooth" });
    });
  });
  window.addEventListener("popstate", () => {
    showPage((location.hash || "#overview").slice(1));
  });
  showPage((location.hash || "#overview").slice(1));
});
