---
title: Community 20 - Next work package recommender
community_id: 20
node_count: 22
source: graphify-out/graph.json
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/community", "daisyhost/cli-verification"]
---

# Community 20 - Next work package recommender

This note gives `Community 20` a DaisyHost domain name so graphify and human readers do not have to navigate recurrent generic community labels.

## Domain Role

Next work package recommender groups source symbols that graphify clustered together from the AST graph. Treat this community name as a semantic label, not as a new implementation claim.

## Top Source Files

- `tools\suggest_next_wp.py`: 17 graph nodes
- `tests\test_next_wp_suggester.py`: 5 graph nodes

## High-Degree Nodes

- `suggest_next_wp.py`: 16 graph edges, source `tools\suggest_next_wp.py`
- `recommend_next_work_package()`: 12 graph edges, source `tools\suggest_next_wp.py`
- `parse_work_packages()`: 6 graph edges, source `tools\suggest_next_wp.py`
- `main()`: 5 graph edges, source `tools\suggest_next_wp.py`
- `load_module()`: 3 graph edges, source `tests\test_next_wp_suggester.py`
- `NextWpSuggesterTest`: 3 graph edges, source `tests\test_next_wp_suggester.py`
- `.test_prefers_first_ready_parallel_start_after_completed_work()`: 3 graph edges, source `tests\test_next_wp_suggester.py`
- `.test_live_tracker_has_decision_ready_recommendation()`: 3 graph edges, source `tests\test_next_wp_suggester.py`
- `is_waiting()`: 3 graph edges, source `tools\suggest_next_wp.py`
- `is_candidate()`: 3 graph edges, source `tools\suggest_next_wp.py`
- `test_next_wp_suggester.py`: 2 graph edges, source `tests\test_next_wp_suggester.py`
- `WorkPackage`: 2 graph edges, source `tools\suggest_next_wp.py`
- `Recommendation`: 2 graph edges, source `tools\suggest_next_wp.py`
- `strip_markdown()`: 2 graph edges, source `tools\suggest_next_wp.py`
- `parse_percent()`: 2 graph edges, source `tools\suggest_next_wp.py`
- `split_markdown_row()`: 2 graph edges, source `tools\suggest_next_wp.py`
- `parse_recommended_parallel_start()`: 2 graph edges, source `tools\suggest_next_wp.py`
- `score_package()`: 2 graph edges, source `tools\suggest_next_wp.py`
- `first_safe_slice_for()`: 2 graph edges, source `tools\suggest_next_wp.py`
- `overlap_risk_for()`: 2 graph edges, source `tools\suggest_next_wp.py`

## Neighbor Communities

- [[Community 12 - Automation scripts smoke tests and adapter tooling]]: 1 cross-community edges

## Extraction Guidance

When this community appears in graphify output, label it as `Next work package recommender` instead of `Community 20`. For future semantic extraction, connect this community to source files, contracts, and product concepts named in the sections above.
