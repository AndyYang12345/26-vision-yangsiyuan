#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

JOBS="${JOBS:-}"
if [[ -z "${JOBS}" ]]; then
  if command -v nproc >/dev/null 2>&1; then
    JOBS="$(nproc)"
  else
    JOBS="4"
  fi
fi

if [[ -f "/opt/intel/openvino_2025/setupvars.sh" ]]; then
  # OpenVINO runtime (newer layout). setupvars.sh isn't nounset-safe.
  set +u
  # shellcheck source=/dev/null
  source "/opt/intel/openvino_2025/setupvars.sh"
  set -u
elif [[ -f "/opt/intel/openvino_2025.4.0/setupvars.sh" ]]; then
  # OpenVINO runtime (versioned layout). setupvars.sh isn't nounset-safe.
  set +u
  # shellcheck source=/dev/null
  source "/opt/intel/openvino_2025.4.0/setupvars.sh"
  set -u
fi

build_cmake() {
  local src_dir="$1"
  local build_dir="$2"
  cmake -S "${src_dir}" -B "${build_dir}"
  cmake --build "${build_dir}" -j "${JOBS}"
}

build_cmake "${ROOT_DIR}/task1" "${ROOT_DIR}/task1/build"
build_cmake "${ROOT_DIR}/task2/CMake_I" "${ROOT_DIR}/task2/CMake_I/build"
build_cmake "${ROOT_DIR}/task2/CMake_II" "${ROOT_DIR}/task2/CMake_II/build"
build_cmake "${ROOT_DIR}/task3" "${ROOT_DIR}/task3/build"

echo "Build complete."
echo "Task1: ${ROOT_DIR}/task1/build"
echo "Task2(CMake_I): ${ROOT_DIR}/task2/CMake_I/build"
echo "Task2(CMake_II): ${ROOT_DIR}/task2/CMake_II/build"
echo "Task3: ${ROOT_DIR}/task3/build"
