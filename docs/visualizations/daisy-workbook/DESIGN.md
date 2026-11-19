---
name: Daisy Workbook
description: An offline repository reference in the inherited docs-page visual system.
colors:
  primary: "#a34228"
  paper: "#fafaf9"
  ink: "#292825"
  muted: "#625e57"
  border: "#dedbd4"
  surface: "#fff"
  inline-code: "#f2f1ed"
  code-ink: "#ece9e2"
  code-surface: "#282825"
  button-hover: "#f2ede6"
  navigation-hover: "#eeeae3"
  table-heading: "#efede7"
  table-hover: "#faf7f1"
  verified: "#326b53"
  derived: "#245b89"
  unverified: "#9b3928"
typography:
  display:
    fontFamily: "Georgia, serif"
    fontSize: "clamp(32px, 4.2vw, 58px)"
    fontWeight: 600
    lineHeight: 1.08
    letterSpacing: "-.025em"
  headline:
    fontFamily: "Segoe UI, system-ui, sans-serif"
    fontSize: "25px"
    lineHeight: 1.2
    letterSpacing: "-.015em"
  body:
    fontFamily: "Segoe UI, system-ui, sans-serif"
    fontSize: "15px"
    lineHeight: 1.65
  lead:
    fontFamily: "Segoe UI, system-ui, sans-serif"
    fontSize: "20px"
    lineHeight: 1.5
  code:
    fontFamily: "Consolas, ui-monospace, monospace"
    fontSize: "12px"
    lineHeight: 1.7
  label:
    fontFamily: "Segoe UI, system-ui, sans-serif"
    fontSize: "11px"
    lineHeight: 1.7
    letterSpacing: ".02em"
rounded:
  container: "8px"
  control: "6px"
  navigation: "5px"
  label: "4px"
  inline-code: "3px"
spacing:
  button: "7px 12px"
  field: "8px 12px"
  table-cell: "13px 14px"
  container: "18px"
  code: "20px"
components:
  button:
    backgroundColor: "{colors.surface}"
    textColor: "{colors.ink}"
    typography: "{typography.body}"
    rounded: "{rounded.control}"
    padding: "{spacing.button}"
  button-hover:
    backgroundColor: "{colors.button-hover}"
  search-input:
    backgroundColor: "{colors.paper}"
    textColor: "{colors.ink}"
    typography: "{typography.body}"
    rounded: "{rounded.control}"
    padding: "{spacing.field}"
  navigation-current:
    backgroundColor: "{colors.primary}"
    textColor: "{colors.surface}"
    rounded: "{rounded.navigation}"
    padding: "6px 10px"
  evidence-label:
    typography: "{typography.label}"
    rounded: "{rounded.label}"
    padding: "2px 6px"
  reference-container:
    backgroundColor: "{colors.surface}"
    rounded: "{rounded.container}"
    padding: "{spacing.container}"
  code-block:
    backgroundColor: "{colors.code-surface}"
    textColor: "{colors.code-ink}"
    typography: "{typography.code}"
    rounded: "{rounded.container}"
    padding: "{spacing.code}"
---

# Design System: Daisy Workbook

## Overview

**Creative North Star: "Inherited docs-page reference"**

This is a record of the implemented reading interface, not a new visual identity. Its authority is the existing docs-page template at `C:/Users/denko/Claude/Projects/html-report-kit/templates/docs-page/example.html`, adapted by this folder's `theme.css`, `build.py`, and `app.js`.

The quiet paper surface, restrained terracotta links, serif page titles, and compact sans-serif navigation support complete repository consultation. The document body carries the reading; the two side rails provide orientation. Source paths, evidence labels, and hashes stay visually legible without competing with the content.

**Key Characteristics:**
- Light paper and white surfaces with thin warm dividers.
- Georgia page titles, Segoe UI reading text, and Consolas source code.
- Sidebar, body, and right-hand table of contents on wide screens.
- Read-oriented tables, original file snapshots, and explicit source labels.

## Colors

### Primary

Terracotta is the single navigation accent: ordinary links, the current chapter, caret, and keyboard focus. It is not a maturity indicator.

### Neutral

Paper is the page background; white marks the header, controls, tables, and bounded reference surfaces. Ink carries primary text, muted ink carries provenance and secondary explanations, and the warm border separates regions. Inline code uses a light neutral surface; complete source readers use the dark code surface and pale code ink.

### Evidence roles

Verified green, derived blue, and unverified red-brown accompany written evidence labels. NOT_RUN shares the unverified color. These semantic roles are separate from the primary accent and never substitute for the written state.

## Typography

The frontmatter records the screen roles. Georgia is reserved for ordinary page titles; source-reader titles use Consolas with a smaller responsive size (`clamp(22px, 2.4vw, 32px)`) and a relaxed line height (1.3). Segoe UI supplies body text, controls, navigation, and headings below the page title. Consolas distinguishes source code and paths.

Reading paragraphs cap at 74ch and lead paragraphs at 65ch. Sidebar links are compact (14px); tables and file lists use 13px; provenance generally uses 12px. Numbers and counts use tabular figures. On narrow screens, the page title becomes 38px, the lead 18px, and second-level headings 23px.

Print uses the final stylesheet overrides: body and project paragraphs at 12px, tables at 11px, provenance and code blocks at 10px. Earlier smaller print declarations are superseded.

## Layout

The wide layout is a three-column grid: sidebar (224px), flexible body (`minmax(0, 1fr)`), and right table of contents (188px). The header is sticky with a minimum height of 74px. The sidebar and TOC scroll independently; body padding is 44px vertically at the top and `clamp(24px, 4vw, 64px)` horizontally.

At 1150px and below, the TOC is hidden, the sidebar becomes 210px, body padding becomes 36px 30px, and distribution rows form one column. At 720px and below, the header wraps with a minimum height of 126px, body padding becomes 32px 20px 60px, and the sidebar opens from an Index button as a fixed drawer (285px, capped at 85vw). Jump lists and include lists become single-column.

**The Contained Reading Rule.** Long tables, diagrams, and source lines scroll inside their own containers. They must not widen the document viewport.

Mobile catalog and source-register tables retain minimum widths (630px and 600px) inside horizontal scroll containers. Complete source lines preserve whitespace and line numbers; the code viewport scrolls horizontally and caps its screen height at 75vh. Paths and hashes wrap. Technical diagrams also retain a minimum readable width inside a local scrolling canvas.

Print uses A4 with margins of 14mm vertically and 13mm horizontally. Reading becomes one column; navigation, interactive code actions, pager, and footer are hidden. The print index and chapter boundaries use explicit page breaks, table headers repeat, and code wraps onto the printed page.

## Elevation & Depth

The reading surface is flat. Warm borders and surface changes separate content. Shadows exist only on the search overlay (`0 14px 24px rgb(40 40 35 / 10%)`) and mobile sidebar (`6px 12px 24px rgb(30 30 25 / 15%)`). There is no general card elevation or hover lift.

Anchor scrolling is smooth unless reduced motion is requested. The drawer changes visibility and transform without an authored animation transition.

## Shapes

Containers and code blocks have gently rounded corners; controls are slightly tighter, and evidence labels tighter still. Borders are thin and functional. Print removes table and figure rounding. Preserve the observed radius roles in the frontmatter rather than imposing one radius on every element.

## Components

- **Controls:** white outlined buttons, light-paper search input and type selector. Hover changes button surface. Keyboard focus uses a terracotta outline (3px, offset 4px). Disabled buttons use muted text.
- **Navigation:** compact sidebar links with a neutral hover surface and a filled terracotta current-page state. The right TOC uses smaller muted links. On mobile, Index toggles the drawer; Escape closes it and search, and chapter selection closes the drawer.
- **Global search:** an inline header field and source-type selector expose a fixed results panel below the header. Search is local, accent-insensitive, and includes projects, modules, code, documents, and chapters. Results show a title and source-kind excerpt; a written status reports the count and the 60-result display limit. Ctrl/Cmd+K focuses the field. This record does not introduce a separate command-palette design.
- **Evidence labels:** outlined text badges show VERIFIED, DERIVED, NOT_RUN, or UNVERIFIED. Source notes and limitations remain adjacent to the claim, in muted reading text.
- **Tables and disclosures:** thin bounded tables use a tinted header, compact cell padding, and wrapped text; catalog rows have a quiet hover surface. Disclosures retain native summaries and expose includes, files, and verbatim source excerpts.
- **File snapshots:** the source reader presents filename, path, line and byte counts, SHA-256, evidence boundary, Copy, original Download, and Provenance links. Anchored line numbers are muted; hovered and targeted lines receive darker tonal highlights. A frozen file identity is not a build or hardware result.
- **Reference containers and figures:** white bounded callouts, chapter indexes, and technical figures use the same restrained container language. Figure captions preserve source and boundary text. Repository distribution bars describe inventory counts, not quality or performance.

## Do's and Don'ts

### Do:
- **Do** preserve the inherited light docs-page world and its reading hierarchy.
- **Do** keep tables, diagrams, and unwrapped source lines inside local scroll containers.
- **Do** pair every evidence color with its written source state and nearby provenance.
- **Do** preserve the final print type overrides and single-column reading flow.

### Don't:
- **Don't** introduce a new palette, decorative card grid, or marketing surface into this reference.
- **Don't** treat snapshot hashes or inventory labels as evidence of firmware execution.
- **Don't** replace original source formatting with wrapped screen code that obscures line identity.

Recorded from current source and existing screenshots under `artifact/screenshots/`. Responsive values and interaction descriptions above are source-backed; full browser behavior, print pagination, and live-panel snippet rendering are verified separately by the workbook's artifact checks, not by this design record. No tonal ramp is implemented, so the sidecar adds no synthetic palette.
