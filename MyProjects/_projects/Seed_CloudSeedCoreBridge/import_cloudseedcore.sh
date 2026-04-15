#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
DEST_DIR="${ROOT_DIR}/third_party/CloudSeedCore"

mkdir -p "${ROOT_DIR}/third_party"

if [[ -d "${DEST_DIR}/.git" ]]; then
  echo "CloudSeedCore already present at ${DEST_DIR}"
  exit 0
fi

echo "Cloning CloudSeedCore into ${DEST_DIR} ..."
git clone https://github.com/GhostNoteAudio/CloudSeedCore "${DEST_DIR}"

echo "Done. Next: wire source files into MyProjects/_projects/Seed_CloudSeedCoreBridge/Makefile"
