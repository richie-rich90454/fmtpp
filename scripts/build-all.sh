#!/usr/bin/env bash
# Cross-compile fmtpp for all supported platforms using zig cc.
# Produces prebuilt binaries under npm/<platform-arch>/.
#
# Easter egg: the build script that turns one codebase into many binaries

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

TARGETS=(
    linux-x64
    linux-arm64
    macos-x64
    macos-arm64
    windows-x64
)

cd "${PROJECT_DIR}"

for target in "${TARGETS[@]}"; do
    echo "Building fmtpp for ${target}..."

    build_dir="build-${target}"
    rm -rf "${build_dir}"
    mkdir -p "${build_dir}"

    cmake -S . -B "${build_dir}" \
        -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
        -DTARGET="${target}" \
        -DCMAKE_BUILD_TYPE=Release

    cmake --build "${build_dir}" --config Release -j"$(nproc 2>/dev/null || echo 4)"

    out_dir="npm/${target}"
    if [ "${target}" = "windows-x64" ]; then
        # ponytail: launcher platform keys use win-, not windows-
        out_dir="npm/win-x64"
    fi
    mkdir -p "${out_dir}"

    if [ "${target}" = "windows-x64" ]; then
        cp "${build_dir}/fmtpp.exe" "${out_dir}/fmtpp.exe"
        chmod +x "${out_dir}/fmtpp.exe"
    else
        cp "${build_dir}/fmtpp" "${out_dir}/fmtpp"
        chmod +x "${out_dir}/fmtpp"
    fi

    echo "  -> ${out_dir}/fmtpp built successfully"
done

echo ""
echo "All targets built. Easter egg: fmtpp is now everywhere at once"
