# DaisyHost Training And Dataset Rendering

Phase 3 adds a headless renderer and a small Python orchestration layer for
offline sweeps.

`DaisyHost Hub.exe` can also dispatch render and dataset jobs through these same
entrypoints. The hub does not replace this workflow; it just prepares or selects
the JSON input and launches the commands below.

## Renderer

Build the CLI:

```sh
cmake -S DaisyHost -B DaisyHost/build
cmake --build DaisyHost/build --config Release --target DaisyHostRender
```

Render one scenario:

```sh
DaisyHost/build/Release/DaisyHostRender.exe DaisyHost/training/examples/multidelay_smoke.json
```

Optional explicit output directory:

```sh
DaisyHost/build/Release/DaisyHostRender.exe DaisyHost/training/examples/torus_smoke.json --output-dir DaisyHost/training/out/torus_smoke
```

Each run writes:

- `audio.wav`
- `manifest.json`

## Automated Smoke Harness

The host gate now includes a direct-entrypoint smoke harness for the standalone
and render binaries. Run it manually with:

```sh
py -3 DaisyHost/tests/run_smoke.py --mode all --build-dir DaisyHost/build --source-dir DaisyHost --config Release
```

Or run the render-only path with:

```sh
py -3 DaisyHost/tests/run_smoke.py --mode render --build-dir DaisyHost/build --source-dir DaisyHost --config Release
```

The render smoke uses the checked-in app and Field scenario JSON files, writes
fresh harness-owned output directories under `DaisyHost/build/smoke/`, verifies
`audio.wav` plus `manifest.json`, checks repeated `multidelay` render
determinism via `audioChecksum`, and validates Field shell/native/extended
surface plus selected-node surface evidence.

## Scenario Schema

Top-level fields:

- `appId`
- `renderConfig`
- `seed`
- `initialParameterValues`
- `audioInput`
- `timeline`

`renderConfig` fields:

- `sampleRate`
- `blockSize`
- `durationSeconds`
- `outputChannelCount`

`audioInput` fields:

- `mode`: `host_in`, `sine`, `triangle`, `square`, `saw`, `noise`, `impulse`
- `level`: `0..10`
- `frequencyHz`: `20..5000`

Timeline event types:

- `parameter_set`
- `cv_set`
- `gate_set`
- `midi`
- `audio_input_config`
- `impulse`
- `menu_rotate`
- `menu_press`
- `menu_set_item`

## Dataset Job Schema

`render_dataset.py` expands a base scenario into multiple run directories.

Top-level fields:

- `jobName`
- `outputDir`
- either `baseScenario` or `baseScenarioPath`
- `sweeps`

Supported sweep kinds:

- `seed`
- `parameter`
- `audio_input_mode`
- `audio_input_level`
- `audio_input_frequency`

Example:

```json
{
  "jobName": "multidelay_frequency_sweep",
  "baseScenarioPath": "examples/multidelay_smoke.json",
  "outputDir": "out/multidelay_frequency_sweep",
  "sweeps": [
    {
      "kind": "audio_input_frequency",
      "values": [110.0, 220.0, 440.0]
    },
    {
      "kind": "parameter",
      "parameterId": "node0/param/dry_wet",
      "values": [0.25, 0.75]
    }
  ]
}
```

Run the dataset job:

```sh
py -3 DaisyHost/training/render_dataset.py DaisyHost/training/examples/dataset_job_example.json
```
