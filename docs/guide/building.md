# Building

## Native build (any OS)

```sh
cmake -S . -B build
cmake --build build --config Release
```

Tested compilers: GCC 5+, Clang 6+, MSVC 2019+. The project targets
C++11 (`CMAKE_CXX_STANDARD 11`); the three toml++-including TUs compile
as C++17 via per-file flags (`-std=c++17` / `/std:c++17`).

## Portability notes

- **Linking.** Fully-static `-static` is applied on Linux only. macOS
  (linker forbids it) and Windows/MSVC get no extra flags; Windows/MinGW
  gets `-static-libstdc++ -static-libgcc`. `ws2_32` links on Windows only.
- **SIMD.** toml++ uses SSE2 intrinsics on x86-64. A configure-time
  compile+link test (`FMTPP_HAVE_WORKING_SSE2`) detects toolchains that
  cannot link them and sets `TOML_ENABLE_SIMD=0` (scalar fallback) for
  those builds only. Working toolchains keep SIMD automatically — no
  manual flags, no arch enumeration.
- **UB sanitizer.** `cmake -S . -B build-sanitize -DFMTPP_SANITIZE_UB=ON`
  builds with `-fsanitize=undefined` for QA runs.

## Cross-compilation (npm targets)

```sh
bash scripts/build-all.sh
```

Uses `zig cc` via `toolchain.cmake` for `linux-x64`, `linux-arm64`,
`macos-x64`, `macos-arm64`, `windows-x64`. Output lands in
`npm/<platform-arch>/`. Single target:

```sh
cmake -S . -B build-cross -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -DTARGET=linux-arm64
cmake --build build-cross --config Release
```

## npm layout

`index.js` resolves `npm/<platform-arch>/fmtpp[.exe]` via
`getPlatformKey()` (`win`/`macos`/`linux` × `x64`/`arm64`), else compiles
from source with `zig cc`. `FMTPP_BINARY` overrides resolution.
