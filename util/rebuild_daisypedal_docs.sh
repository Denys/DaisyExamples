#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
DOXYFILE="${PROJECT_ROOT}/Doxyfile"
DOC_BUILD_DIR="${PROJECT_ROOT}/build/docs/daisypedal"
LATEX_DIR="${DOC_BUILD_DIR}/latex"
PDF_OUT="${PROJECT_ROOT}/docs/daisypedal_reference.pdf"

need_tool() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "[ERROR] Required tool not found in PATH: $1" >&2
        exit 1
    fi
}

need_tool doxygen

if command -v pdflatex >/dev/null 2>&1 && command -v makeindex >/dev/null 2>&1; then
    LATEX_ENGINE="pdflatex"
elif command -v tectonic >/dev/null 2>&1; then
    LATEX_ENGINE="tectonic"
else
    echo "[ERROR] Required LaTeX toolchain not found in PATH. Install pdflatex+makeindex or tectonic." >&2
    exit 1
fi

rm -rf "${DOC_BUILD_DIR}"

pushd "${PROJECT_ROOT}" >/dev/null
echo "[1/4] Running Doxygen"
doxygen "${DOXYFILE}"
popd >/dev/null

if [[ ! -f "${LATEX_DIR}/refman.tex" ]]; then
    echo "[ERROR] Doxygen did not generate ${LATEX_DIR}/refman.tex" >&2
    exit 1
fi

pushd "${LATEX_DIR}" >/dev/null
echo "[2/4] Building LaTeX reference"
if [[ "${LATEX_ENGINE}" == "pdflatex" ]]; then
    pdflatex -interaction=nonstopmode refman.tex >/dev/null
    makeindex refman.idx >/dev/null
    echo "[3/4] Finalizing PDF"
    pdflatex -interaction=nonstopmode refman.tex >/dev/null
    pdflatex -interaction=nonstopmode refman.tex >/dev/null
else
    : > refman.ind
    tectonic refman.tex >/dev/null
fi
popd >/dev/null

cp -f "${LATEX_DIR}/refman.pdf" "${PDF_OUT}"
echo "[4/4] Wrote ${PDF_OUT}"
