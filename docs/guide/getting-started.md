# Getting started

## Install from npm (recommended)

Prebuilt static binaries ship for `linux-x64`, `linux-arm64`,
`macos-x64`, `macos-arm64` and `win-x64`:

```sh
npm install fmtpp
npx fmtpp --version
```

If no prebuilt binary matches your platform, the `fmtpp` launcher
compiles from source with `zig cc` automatically. You can also pin a
binary explicitly:

```sh
FMTPP_BINARY=/path/to/fmtpp npx fmtpp format main.js
```

## Build from source

Requirements: CMake ≥ 3.16, Ninja (or Make), a C++11-capable compiler
(GCC 5+, Clang 6+, MSVC 2019+). CLI11 and toml++ are vendored under
`lib/` and need no separate install.

```sh
cmake -S . -B build
cmake --build build --config Release
./build/fmtpp --version
```

See [Building](./building.md) for platform notes and cross-compilation.

## First format

```sh
fmtpp format src/main.js
fmtpp src/main.js          # shorthand for a single file
fmtpp format a.js b.go c.rs  # multi-file (parallel) run
```

Single-file output goes to stdout; redirect it to rewrite in place:

```sh
fmtpp format main.js > main.js.tmp && mv main.js.tmp main.js
```

## Next steps

- [CLI reference](./cli.md) — every command and flag
- [Configuration](./configuration.md) — `.fmtpp.toml`, `learn`, `config`
- [Languages](./languages.md) — per-language style notes
