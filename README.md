# fmtpp — Format++

A professional-grade, multi-language code formatter written in C++.
One static binary, eight languages, zero runtime dependencies.

- [Supported languages](#supported-languages)
- [Install](#install)
- [Formatting files](#formatting-files)
- [Configuration](#configuration)
- [CLI reference](#cli-reference)
- [Running the tests](#running-the-tests)
- [Building from source](#building-from-source)
- [Project layout](#project-layout)
- [How it works](#how-it-works)
- [Troubleshooting](#troubleshooting)
- [Current status and limitations](#current-status-and-limitations)
- [License](#license)

Full guides with the same content live on the docs site: `docs/`
(run `npm run docs:dev` inside `docs/`).

## Supported languages

The formatter is picked from the file extension. Unknown extensions
fall back to JavaScript.

| Language   | Extensions                  | Formatter          | Reference style    |
|------------|-----------------------------|--------------------|--------------------|
| JavaScript | `.js` `.mjs` `.cjs`         | `JSFormatter`      | Prettier           |
| TypeScript | `.ts` `.tsx` `.mts`         | `TSFormatter`      | Prettier           |
| Java       | `.java`                     | `JavaFormatter`    | google-java-format |
| C          | `.c` `.h`                   | `CFormatter`       | clang-format       |
| C++        | `.cpp` `.cc` `.cxx` `.hpp` `.hh` `.hxx` | `CppFormatter` | clang-format |
| C#         | `.cs`                       | `CSharpFormatter`  | `dotnet format`    |
| Go         | `.go`                       | `GoFormatter`      | gofmt              |
| Rust       | `.rs`                       | `RustFormatter`    | rustfmt            |

## Install

### Option A — npm (prebuilt binary, no compilation)

```sh
npm install fmtpp
npx fmtpp --version
```

Prebuilt static binaries ship for `linux-x64`, `linux-arm64`,
`macos-x64`, `macos-arm64` and `win-x64` under `npm/`. The `fmtpp`
launcher (`index.js`) picks the right one for your OS and CPU.

Two escape hatches:

```sh
# Use an exact binary (e.g. your own build)
FMTPP_BINARY=/path/to/fmtpp npx fmtpp format main.js

# No matching prebuilt binary? The launcher compiles from source
# automatically using zig cc — no action needed.
```

### Option B — build from source

You need CMake ≥ 3.16, Ninja (or Make) and a C++11-capable compiler
(GCC 5+, Clang 6+, MSVC 2019+). CLI11 and toml++ are vendored under
`lib/`, nothing else to install. Details in
[Building from source](#building-from-source).

```sh
cmake -S . -B build
cmake --build build --config Release
./build/fmtpp --version        # Linux / macOS
.\build\fmtpp.exe --version    # Windows
```

## Formatting files

### Format one file (prints to stdout)

```sh
fmtpp format main.js
fmtpp main.js                  # shorthand, same thing
```

The formatted code goes to **stdout**, the original file is never
touched. To rewrite a file in place, redirect:

```sh
# POSIX
fmtpp format main.js > main.js.tmp && mv main.js.tmp main.js

# PowerShell
fmtpp format main.js | Set-Content main.js.tmp; Move-Item main.js.tmp main.js -Force
```

### Format several files at once

```sh
fmtpp format a.js b.go c.rs
```

```text
Formatting 3 files with 16 threads...
  Formatted: a.js
  Formatted: b.go
  Formatted: c.rs
Done: 3 succeeded, 0 failed
```

This path is parallel (one file per thread) and prints a **status
report**, not the formatted code. Single-file mode prints code;
multi-file mode prints progress. Files that cannot be read or parsed
are listed as `Failed:` and the command exits with code `1`.

### Which config is used?

`format` looks for `.fmtpp.toml` starting in the **formatted file's own
directory**, then each parent directory (same rule as Prettier). The
first config found wins; if none exists (or it is broken), built-in
defaults apply. See [Configuration](#configuration).

### Benchmark a file

```sh
fmtpp bench app.js                     # 10 iterations + warmup
fmtpp bench app.js --runs 100          # custom iteration count
fmtpp bench app.js --runs 20 --compare oxfmt   # also time Oxfmt/Prettier
```

Prints total/average/min/max/median/P95 time, throughput and a verdict
against the 12 ms budget.

## Configuration

`fmtpp` reads `.fmtpp.toml` (TOML). Three ways to get one:

```sh
fmtpp learn example.js                 # detect style, write ./.fmtpp.toml
fmtpp learn example.js --output custom.toml
fmtpp config                           # interactive TUI editor (needs a terminal)
```

`learn` runs `StyleDetector` over the file (indent, brace placement,
semicolons, trailing commas, print width, spacing, final newline) and
writes the result as TOML:

```toml
printWidth = 80
useTabs = false
tabWidth = 2
semi = true
singleQuote = false
trailingComma = "es5"     # "none", "es5" or "all"
bracketSpacing = true
arrowParens = "always"
endOfLine = "lf"          # "lf", "crlf", "cr" or "auto"
braceStyle = "same-line"  # "same-line" (K&R) or "next-line" (Allman)
spaceBeforeBrace = true
spaceAfterKeyword = true  # space after if/for/while
spaceAroundOperators = true
```

Defaults mirror Prettier. Unknown keys are ignored; a missing or
unparseable file silently falls back to defaults.

## CLI reference

```text
fmtpp [OPTIONS] [FILE]
fmtpp <command> [options]

  fmtpp format <file...>   format files (code to stdout for 1 file, report for N)
  fmtpp <file>             shorthand for formatting a single file
  fmtpp bench <file> [--runs N] [--compare <oxfmt|prettier>]
  fmtpp learn <file> [--output .fmtpp.toml]
  fmtpp config             interactive TUI configurator
  fmtpp --version, -v
  fmtpp --help, -h
```

Hidden flags (just for fun): `--easter-egg`, `--cowbell`, `--kittens`,
`--unicorn`. Running `fmtpp` with no arguments prints usage.

Exit codes: `0` on success, `1` when a file cannot be read/parsed or any
file in a multi-file run fails.

## Running the tests

### C++ suites (the real coverage)

18 suites under `ctest`: one formatter suite per language plus infra
(pool, buffers, parser, thread pool, config, benchmark).

```sh
# From the repo root:
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

Useful variants (run from `build/`):

```sh
ctest -R javaformatter --output-on-failure   # one suite only
ctest -R "formatter"                         # all 8 formatter suites
./test_goformatter.exe                       # run a suite binary directly
```

Each language suite is `tests/test_<lang>formatter.cpp`, with a matching
real-world sample in `tests/fixtures/sample.<ext>`:

| Suite | Fixture | What it checks |
|---|---|---|
| `test_jsformatter` | `sample.js` | `let/const`, functions, imports, classes, arrows, arrays, objects, exports |
| `test_tsformatter` | `sample.ts` | return types, interfaces, enums, type aliases, `extends` |
| `test_javaformatter` | `sample.java` | `public` methods, `extends`/`implements`, imports, `switch`+`break`, try/catch |
| `test_cformatter` | `sample.c` | decls, pointers, `#include`/`#define`, `if`/`for`/`while`, comments |
| `test_cppformatter` | `sample.cpp` | `Ret name()`, `: public Base`, `namespace`, forward decls |
| `test_csharpformatter` | `sample.cs` | `public class : Base, IFace`, methods, `namespace`, interfaces |
| `test_goformatter` | `sample.go` | `func`, `:=`, no semicolons, structs, `iota`, `panic`, `nil` |
| `test_rustformatter` | `sample.rs` | `fn ->`, `let [mut]`, `use`, `match`, `panic!`, `vec!`, closures, traits |

Suites share one pattern: build ASTs with `ASTNodeFactory` on a local
`MemoryPool`, format into an `OutputBuffer`, assert on the output,
print `N passed, M failed`, return nonzero on failure. To add coverage,
copy the closest suite and register its name in `FMTPP_TESTS` in
`CMakeLists.txt` — no other wiring needed.

### npm packaging tests

```sh
npm test   # runs tests/test_npm_packaging.js
```

Checks `package.json` fields, platform detection, binary-path
resolution, binary slots for all five targets, the `FMTPP_BINARY`
override and the compile-from-source fallback.

## Building from source

```sh
cmake -S . -B build
cmake --build build --config Release
```

Tested compilers: GCC 5+, Clang 6+, MSVC 2019+. The project targets
C++11; the three toml++-including files compile as C++17 via per-file
flags (`-std=c++17` / `/std:c++17`).

Three portability details, all automatic:

- **Linking.** Fully-static `-static` applies on Linux only (the macOS
  linker forbids it; it breaks `lld-link` on Windows). Windows/MinGW
  gets `-static-libstdc++ -static-libgcc`; `ws2_32` links on Windows.
- **SIMD.** toml++ uses SSE2 intrinsics on x86-64. A configure-time
  compile+link probe (`FMTPP_HAVE_WORKING_SSE2`) detects toolchains that
  cannot link them and defines `TOML_ENABLE_SIMD=0` (scalar fallback)
  for those builds only. Healthy toolchains keep SIMD with no flags.
- **UB sanitizer.** `cmake -S . -B build-sanitize -DFMTPP_SANITIZE_UB=ON`
  builds with `-fsanitize=undefined` for QA runs.

### Cross-compiling the npm binaries

```sh
bash scripts/build-all.sh            # needs zig cc; fills npm/<platform-arch>/
```

Single target:

```sh
cmake -S . -B build-cross -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -DTARGET=linux-arm64
cmake --build build-cross --config Release
```

Targets: `linux-x64`, `linux-arm64`, `macos-x64`, `macos-arm64`,
`windows-x64` (see `toolchain.cmake`).

### Docs site

```sh
cd docs
npm install
npm run docs:dev       # local preview
npm run docs:build     # static build
```

## Project layout

```text
include/fmtpp/   public headers (formatters, AST, infra)
src/             implementation (C++11, Java-style OOP)
tests/           CTest suites + tests/fixtures/ language samples
docs/            VitePress documentation site
lib/             vendored CLI11 + toml++ mirrors
npm/             prebuilt binary slots per platform-arch
scripts/         cross-compilation (zig cc) helpers
```

## How it works

```text
SourceBuffer → Parser → AST → FormatterVisitor → OutputBuffer → stdout
                             (one per language)     ↑
ThreadPool fans multi-file runs out; each job owns its buffer+parser.
```

- **`MemoryPool`** — one 10 MB bump-allocated chunk; every AST node
  comes from `ASTNodeFactory` via placement new. No per-node
  `new`/`delete`, freed in one shot.
- **`CstyleFormatterBase`** — the shared master for all C-like
  languages (indent, braces, semicolons, spacing, wrapping, comments).
  Subclasses override only what differs (`visitFunctionDecl`,
  `visitClassDecl`, …). Go and Rust extend `FormatterVisitor` directly.
- **Resources** — every acquisition (`fopen`, `malloc`, `mmap`) lives in
  a RAII wrapper (`FileHandle`, `MallocBuffer`, `MmapRegion`) or a
  `goto`-cleanup function with a single exit label.
- Config precedence: `.fmtpp.toml` found from the file upward >
  built-in defaults. `StyleDetector` (`learn`) and `TuiConfigurator`
  (`config`) both read and write that file.

## Troubleshooting

- **Fresh configure fails oddly / stale errors after editing
  `CMakeLists.txt`.** Delete the build dir and reconfigure:
  `cmake -S . -B build` re-runs automatically under Ninja, but a clean
  `Remove-Item -Recurse build; cmake -S . -B build` never hurts.
- **`ctest` says "No tests were found".** You ran it from the repo root
  without `--test-dir build`, or from inside `build/` with
  `cmake --build build` (that path only exists at the repo root). From
  the root: `ctest --test-dir build`. From inside `build/`: plain
  `ctest`.
- **`test_threadpool` segfaults once, passes on re-run.** Known flake:
  workers share the global `MemoryPool` while `Parser::parse` calls
  `g_pool->reset()` — a data race. Re-run; the proper fix is per-thread
  pools (pending `ThreadPool`/`Parser` rework).
- **Link errors about `_mm_*` / SSE intrinsics.** Your toolchain cannot
  link them (seen with mismatched Clang/MSVC installs). The build
  detects this at configure time
  (`fmtpp: toolchain cannot link SSE2 intrinsics…`) and falls back to
  the scalar TOML path automatically — no action needed.
- **Formatted output looks like a placeholder** (`const placeholder =
  0;`). Expected for now: `Parser` is a fallback stub until tree-sitter
  grammars are linked (see below). Formatter behavior itself is covered
  by the unit suites.
- **`.fmtpp.toml` seems ignored.** It is resolved from the formatted
  file's directory upward, not from your shell's cwd. Put it next to
  (or above) the file you format; check `learn` output parses with no
  typos in key names.

## Current status and limitations

Implemented and tested: all eight formatters, config load/merge,
`learn`, TUI `config`, `bench` (+`--compare`), thread-pool multi-file
runs, npm packaging with five prebuilt targets.

Pending: linking tree-sitter grammars into `Parser` (it currently
returns a minimal stub AST, so CLI file output does not yet reflect
real input). The full phase plan lives in the engineering blueprint
(`fmtpp.txt`, gitignored) and `docs/guide/architecture.md`.

## License

MIT — see `package.json` (`fmtpp` on npm).
