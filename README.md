# fmtpp — Format++

A professional-grade, multi-language code formatter written in C++.
One static binary, eight languages, zero runtime dependencies.

## Supported languages

| Language   | Extensions                    | Formatter          | Reference style       |
|------------|-------------------------------|--------------------|-----------------------|
| JavaScript | `.js` `.mjs` `.cjs`           | `JSFormatter`      | Prettier              |
| TypeScript | `.ts` `.tsx` `.mts`           | `TSFormatter`      | Prettier              |
| Java       | `.java`                       | `JavaFormatter`    | google-java-format    |
| C          | `.c` `.h`                     | `CFormatter`       | clang-format          |
| C++        | `.cpp` `.cc` `.cxx` `.hpp`…   | `CppFormatter`     | clang-format          |
| C#         | `.cs`                         | `CSharpFormatter`  | `dotnet format`       |
| Go         | `.go`                         | `GoFormatter`      | gofmt                 |
| Rust       | `.rs`                         | `RustFormatter`    | rustfmt               |

Language is detected from the file extension. Unknown extensions fall back
to JavaScript formatting.

## Install

### npm (prebuilt binary, no compilation)

```sh
npm install fmtpp
npx fmtpp --version
```

Prebuilt static binaries ship for `linux-x64`, `linux-arm64`,
`macos-x64`, `macos-arm64` and `win-x64` under `npm/`.
If no prebuilt binary matches your platform, `index.js` falls back to
compiling from source with `zig cc`. Override the binary path with:

```sh
FMTPP_BINARY=/path/to/fmtpp npx fmtpp format main.js
```

### Build from source

Requirements: CMake ≥ 3.16, Ninja (or Make), a C++11-capable compiler
(GCC 5+, Clang 6+, MSVC 2019+). Dependencies (CLI11, toml++) are vendored
under `lib/` and fetched automatically when the network is available.

```sh
cmake -S . -B build
cmake --build build --config Release
./build/fmtpp --version        # Linux / macOS
.\build\fmtpp.exe --version    # Windows
```

Cross-compile all npm targets with:

```sh
bash scripts/build-all.sh      # needs zig cc, see toolchain.cmake
```

The build is portable across Linux, macOS and Windows on x64 and arm64:
the SSE2 fast path in toml++ is feature-tested at configure time and
disabled automatically on toolchains that cannot link it.

## Usage

```sh
fmtpp format <file...>          # format one or more files (stdout for one file)
fmtpp <file>                    # shorthand: format a single file
fmtpp bench <file> [--runs N] [--compare <oxfmt|prettier>]
fmtpp learn <file> [--output .fmtpp.toml]
fmtpp config                    # interactive TUI configurator
fmtpp --version | --help
```

Hidden flags: `--easter-egg`, `--cowbell`, `--kittens`, `--unicorn`.

## Configuration

`fmtpp` reads `.fmtpp.toml` (TOML). Generate one from an existing file:

```sh
fmtpp learn --from style-example.js        # writes .fmtpp.toml
fmtpp config                               # edit it interactively
```

```toml
printWidth = 80
useTabs = false
tabWidth = 2
semi = true
singleQuote = false
bracketSpacing = true
braceStyle = "same-line"  # "same-line" (K&R) or "next-line" (Allman)
trailingComma = "es5"     # "none", "es5" or "all"
endOfLine = "lf"          # "lf", "crlf", "cr" or "auto"
```

## Testing

18 CTest suites cover every formatter plus infra (pool, buffers, parser,
thread pool, config, benchmark). Each supported language has a dedicated
suite (`tests/test_<lang>formatter.cpp`) and a sample file under
`tests/fixtures/sample.<ext>`.

```sh
cmake -S . -B build && cmake --build build --config Release
ctest --test-dir build --output-on-failure
npm test   # npm packaging tests (tests/test_npm_packaging.js)
```

## Project layout

```
include/fmtpp/   public headers (formatters, AST, infra)
src/             implementation (C++11, Java-style OOP)
tests/           CTest suites + tests/fixtures/ language samples
docs/            VitePress documentation site (npm run docs:dev)
lib/             vendored CLI11 + toml++ mirrors
npm/             prebuilt binary slots per platform-arch
scripts/         cross-compilation (zig cc) helpers
```

## Architecture in 30 seconds

`SourceBuffer` (mmap/RAII) → `Parser` (AST via `MemoryPool` bump
allocator + `ASTNodeFactory`) → language `Formatter` (visitor over the
AST, output into `OutputBuffer`) → stdout. Multi-file runs fan out over
a `ThreadPool`; each job owns its buffer and parser. See
`docs/` and `fmtpp.txt` (the full engineering blueprint) for details.

## Current status

Formatter output, config/learn/TUI, benchmark and npm packaging are
implemented and tested. `Parser` is currently a fallback stub producing a
minimal AST — tree-sitter integration is the pending item before
real-world file formatting matches the unit-tested formatter output.

## License

MIT — see `package.json` (`fmtpp` on npm).
