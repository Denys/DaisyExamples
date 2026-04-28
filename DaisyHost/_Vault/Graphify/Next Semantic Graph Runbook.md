---
title: Next Semantic Graph Runbook
source: graphify refinement pass
generated_at: 2026-04-28
tags: ["daisyhost/vault", "daisyhost/graphify"]
---

# Next Semantic Graph Runbook

## Recommended Next Run

Run semantic graphify on `_Vault` first. This keeps the corpus small and focuses extraction on curated domain labels and relationships.

Expected outcome:

- named concept nodes instead of `Community X`,
- meaningful semantic edges instead of mostly `contains [EXTRACTED]`,
- source-backed links from architecture concepts to code modules,
- a graph overlay that can be compared with the AST graph.

## Full Repo Caution

The repo-level detector found more than 200 supported files and media files. A full semantic pass across the entire `DaisyHost` directory should be treated as a larger extraction job.

## Validation Command

```powershell
& "C:\Users\denko\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe" -c "from graphify.detect import detect; from pathlib import Path; import json; print(json.dumps(detect(Path('_Vault')), indent=2))"
```
