# Skill Evaluation: Expand NotebookLM Skill Or Create A New One?

## Recommendation

Expand the existing generic `notebooklm` skill with a small "repo-memory overlay"
pattern rather than creating a brand-new skill right now.

## Why Expansion Is Better Right Now

The existing NotebookLM skill already covers:

- CLI vs MCP routing
- local authentication
- source ingestion
- notebook querying
- artifact-generation workflows

What is missing is not a new NotebookLM capability. The missing piece is a
repo-specific policy for how a notebook should be used as persistent memory.

That policy belongs in:

- repo docs
- `AGENTS.md`
- DaisyBrain companion context files

Creating a new skill immediately would add maintenance overhead and duplicate
generic NotebookLM mechanics that already exist.

## What To Add To The Existing NotebookLM Skill

If the shared skill is expanded later, add a short section covering:

- **repository-memory mode**
  - consult local docs first
  - use the notebook for cross-project synthesis
  - prefer layered curated context packs over repo dumps
- **source curation heuristics**
  - favor stable workflow docs, current-state docs, and roadmap docs
  - avoid low-signal generated files or raw code dumps unless explicitly needed
- **refresh triggers**
  - when to update notebook sources after strategic repo changes

## When A New Skill *Would* Be Justified

Create a new repo-specific skill only if all of the following become true:

1. agents repeatedly need a standard memory-curation workflow for this repo
2. DaisyBrain operations become more than simple NotebookLM usage
3. there is a repeatable, repo-specific sequence such as:
   - refresh context pack
   - update notebook sources
   - run validation query set
   - update checkpoint docs

If that happens, the new skill should not replace `notebooklm`; it should wrap
it. A better future name would be something like:

- `daisyexamples-memory`
- or `daisybrain-curation`

## Current Decision

For now:

- keep using the existing `notebooklm` skill
- strengthen repo-local guidance
- keep the new context pack as the durable repository-memory layer
