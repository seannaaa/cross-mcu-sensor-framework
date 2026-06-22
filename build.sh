#!/usr/bin/env bash

set -euo pipefail

# 这里统一用平台名来区分 build 目录，避免不同平台互相覆盖。
readonly PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

usage() {
    cat <<'EOF'
Usage:
  ./build.sh <platform> [extra cmake args...]

Examples:
  ./build.sh esp32
  ./build.sh stm32
  ./build.sh linux_native
  ./build.sh stm32 -DCMAKE_TOOLCHAIN_FILE=cmake/stm32-toolchain.cmake

Notes:
  - <platform> 会传给 CMake 变量 MCU_PLATFORM
  - 构建目录会生成在 build/<platform>/
  - 如果平台名带 /，会自动转换成 _，例如 esp32/s3 -> build/esp32_s3
EOF
}

if [[ $# -lt 1 ]]; then
    usage
    exit 1
fi

platform_raw="${1%/}"
shift

if [[ -z "${platform_raw}" ]]; then
    echo "Error: platform 不能为空"
    usage
    exit 1
fi

# 把平台名做成安全目录名，方便后续扩展到 esp32/s3 这类写法。
platform_dir="${platform_raw//\//_}"
build_dir="${PROJECT_ROOT}/build/${platform_dir}"

echo "[build.sh] PROJECT_ROOT=${PROJECT_ROOT}"
echo "[build.sh] MCU_PLATFORM=${platform_raw}"
echo "[build.sh] BUILD_DIR=${build_dir}"

cmake -S "${PROJECT_ROOT}" \
      -B "${build_dir}" \
      -DMCU_PLATFORM="${platform_raw}" \
      "$@"

cmake --build "${build_dir}"

echo "[build.sh] build finished: ${platform_raw}"
