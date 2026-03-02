# File Dependencies - Field Wavetable Morph Synth

## Dependency Flowchart

```mermaid
flowchart TB
    subgraph External["External Libraries"]
        daisy_field["daisy_field.h"]
        daisysp["daisysp.h"]
        daisy["daisy.h"]
    end

    subgraph Main["Entry Point"]
        main["main.cpp"]
    end

    subgraph Voice_Module["Voice Module"]
        voice_h["voice.h"]
        voice_cpp["voice.cpp"]
    end

    subgraph UI_Module["UI Module"]
        ui_handler_h["ui_handler.h"]
        ui_handler_cpp["ui_handler.cpp"]
        field_ux_h["field_ux.h"]
        field_ux_cpp["field_ux.cpp"]
    end

    subgraph MIDI_Module["MIDI Module"]
        midi_handler_h["midi_handler.h"]
        midi_handler_cpp["midi_handler.cpp"]
    end

    subgraph DSP_Components["DSP Components"]
        wavetable_osc_h["wavetable_osc.h"]
        wavetable_osc_cpp["wavetable_osc.cpp"]
        morph_processor_h["morph_processor.h"]
        morph_processor_cpp["morph_processor.cpp"]
        wavetables_h["wavetables.h"]
        wavetables_cpp["wavetables.cpp"]
    end

    %% main.cpp dependencies
    main --> daisy_field
    main --> daisysp
    main --> voice_h
    main --> ui_handler_h
    main --> midi_handler_h
    main --> wavetables_h

    %% voice.h dependencies
    voice_h --> wavetable_osc_h
    voice_h --> morph_processor_h
    voice_h --> daisysp

    %% ui_handler.h dependencies
    ui_handler_h --> daisy_field
    ui_handler_h --> daisysp
    ui_handler_h --> voice_h
    ui_handler_h --> morph_processor_h
    ui_handler_h --> wavetables_h
    ui_handler_h --> field_ux_h

    %% midi_handler.h dependencies
    midi_handler_h --> daisy_field
    midi_handler_h --> voice_h

    %% wavetables.h dependencies
    wavetables_h --> daisy

    %% field_ux.h dependencies
    field_ux_h --> daisy_field
    field_ux_h --> daisysp

    %% Implementation files
    voice_cpp -.-> voice_h
    ui_handler_cpp -.-> ui_handler_h
    midi_handler_cpp -.-> midi_handler_h
    wavetable_osc_cpp -.-> wavetable_osc_h
    morph_processor_cpp -.-> morph_processor_h
    wavetables_cpp -.-> wavetables_h
    field_ux_cpp -.-> field_ux_h
```

**Legend:**
- **Solid arrows (→)**: `#include` dependency
- **Dashed arrows (⤏)**: Implementation includes its header

---

## Dependency Table

| File | Dependencies |
|------|--------------|
| `main.cpp` | `daisy_field.h`, `daisysp.h`, `voice.h`, `ui_handler.h`, `midi_handler.h`, `wavetables.h` |
| `voice.h` | `wavetable_osc.h`, `morph_processor.h`, `daisysp.h` |
| `ui_handler.h` | `daisy_field.h`, `daisysp.h`, `voice.h`, `morph_processor.h`, `wavetables.h`, `field_ux.h` |
| `midi_handler.h` | `daisy_field.h`, `voice.h` |
| `wavetables.h` | `daisy.h` |
| `field_ux.h` | `daisy_field.h`, `daisysp.h` |
| `wavetable_osc.h` | `<stdint.h>` only |
| `morph_processor.h` | `<stdint.h>` only |
