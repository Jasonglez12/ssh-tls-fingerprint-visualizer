#!/usr/bin/env bash
# freeze_artifacts.sh - Run the reproducible capture pipeline and stage outputs for release.
#
# Usage: ./scripts/freeze_artifacts.sh
# Runs the demo capture pipeline, gathers PCAPs/logs/CSV/figures into
# release/artifacts subfolders, and writes a manifest with hashes and metadata.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${REPO_ROOT}"

CAPTURE_CMD="make clean && make up && make demo"
DATA_DIR="${REPO_ROOT}/data"
ARTIFACT_ROOT="${REPO_ROOT}/release/artifacts"
LOG_DIR="${ARTIFACT_ROOT}/logs"
CSV_DIR="${ARTIFACT_ROOT}/csv"
PCAP_DIR="${ARTIFACT_ROOT}/pcaps"
FIGURE_DIR="${ARTIFACT_ROOT}/figures"
MANIFEST_PATH="${ARTIFACT_ROOT}/manifest.json"

run_capture() {
  echo "Running capture pipeline: ${CAPTURE_CMD}" 
  bash -lc "cd \"${REPO_ROOT}\" && ${CAPTURE_CMD}"
}

copy_if_present() {
  local pattern="$1" dest="$2"
  shopt -s nullglob
  local matches=($pattern)
  shopt -u nullglob
  if (( ${#matches[@]} > 0 )); then
    mkdir -p "${dest}"
    for file in "${matches[@]}"; do
      cp "${file}" "${dest}/"
    done
  fi
}

gather_artifacts() {
  mkdir -p "${ARTIFACT_ROOT}" "${LOG_DIR}" "${CSV_DIR}" "${PCAP_DIR}" "${FIGURE_DIR}"

  copy_if_present "${DATA_DIR}"/*.pcap "${PCAP_DIR}"
  copy_if_present "${DATA_DIR}"/*.pcapng "${PCAP_DIR}"

  copy_if_present "${DATA_DIR}"/*.log "${LOG_DIR}"
  copy_if_present "${ARTIFACT_ROOT}"/*.log "${LOG_DIR}"

  copy_if_present "${DATA_DIR}"/*.csv "${CSV_DIR}"
  copy_if_present "${ARTIFACT_ROOT}"/*.csv "${CSV_DIR}"

  if [ -d "${DATA_DIR}/figures" ]; then
    find "${DATA_DIR}/figures" -maxdepth 1 -type f -print0 | xargs -0 -r -I{} cp "{}" "${FIGURE_DIR}/"
  fi
}

generate_manifest() {
  local git_commit command_escaped
  git_commit=$(git -C "${REPO_ROOT}" rev-parse HEAD)
  command_escaped=${CAPTURE_CMD//"/\\"}

  local files=()
  while IFS= read -r -d '' file; do
    files+=("${file}")
  done < <(cd "${ARTIFACT_ROOT}" && find . -type f ! -name "$(basename "${MANIFEST_PATH}")" -print0 | sort -z)

  {
    echo "{"
    echo "  \"git_commit\": \"${git_commit}\"," 
    echo "  \"command\": \"${command_escaped}\"," 
    echo "  \"artifacts\": ["

    for i in "${!files[@]}"; do
      local file="${files[$i]}"
      local rel="${file#./}"
      local hash
      hash=$(cd "${ARTIFACT_ROOT}" && sha256sum "${rel}" | awk '{print $1}')
      local suffix="," 
      if [ "$i" -eq $((${#files[@]} - 1)) ]; then
        suffix=""
      fi
      echo "    {\"file\": \"${rel//"/\\"}\", \"sha256\": \"${hash}\"}${suffix}"
    done

    echo "  ]"
    echo "}"
  } > "${MANIFEST_PATH}"
}

run_capture
gather_artifacts
generate_manifest

echo "Artifacts frozen in ${ARTIFACT_ROOT}"
