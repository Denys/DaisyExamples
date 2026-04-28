---
title: Community 12 - Automation scripts smoke tests and adapter tooling
community_id: 12
node_count: 45
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/cli-verification"]
---

# Community 12 - Automation scripts smoke tests and adapter tooling

This note gives `Community 12` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Automation scripts smoke tests and adapter tooling groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `tests\run_smoke.py`: 22 graph nodes
- `tools\generate_field_adapter.py`: 8 graph nodes
- `training\render_dataset.py`: 8 graph nodes
- `tools\audit_firmware_portability.py`: 6 graph nodes

## High-Degree Nodes

- `run_smoke.py`: 20 graph edges, source `tests\run_smoke.py`
- `SmokeFailure`: 16 graph edges, source `tests\run_smoke.py`
- `require_existing_file()`: 9 graph edges, source `tests\run_smoke.py`
- `run_single_standalone_smoke()`: 9 graph edges, source `tests\run_smoke.py`
- `run_render_smoke()`: 8 graph edges, source `tests\run_smoke.py`
- `run_single_render()`: 7 graph edges, source `tests\run_smoke.py`
- `generate_field_adapter.py`: 7 graph edges, source `tools\generate_field_adapter.py`
- `render_dataset.py`: 7 graph edges, source `training\render_dataset.py`
- `parse_args()`: 6 graph edges, source `tests\run_smoke.py`
- `write_generated_adapter()`: 6 graph edges, source `tools\generate_field_adapter.py`
- `main()`: 6 graph edges, source `training\render_dataset.py`
- `run_command()`: 5 graph edges, source `tests\run_smoke.py`
- `query_processes_for_path()`: 5 graph edges, source `tests\run_smoke.py`
- `main()`: 5 graph edges, source `tests\run_smoke.py`
- `audit_firmware_portability.py`: 5 graph edges, source `tools\audit_firmware_portability.py`
- `format_command()`: 4 graph edges, source `tests\run_smoke.py`
- `ensure_executable_not_running()`: 4 graph edges, source `tests\run_smoke.py`
- `verify_render_manifest()`: 4 graph edges, source `tests\run_smoke.py`
- `verify_manifest_parameter_values()`: 4 graph edges, source `tests\run_smoke.py`
- `verify_manifest_field_surface()`: 4 graph edges, source `tests\run_smoke.py`

## Neighbor Communities

- [[Community 20 - Next work package recommender]]: 1 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `Automation scripts smoke tests and adapter tooling` instead of `Community 12`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
