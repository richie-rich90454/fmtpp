# Getting started

## What fmtpp is

One static binary that formats 26 languages — C-style braces,
Python/Ruby indentation, HTML/Vue/JSON — detected from the file
extension. Unknown extensions fall back to JavaScript. Anything the
parser does not understand passes through verbatim instead of
corrupting your code.

```mermaid
flowchart LR
    A["Write code"] --> B["Run fmtpp format"]
    B --> C["Review stdout diff"]
    C --> D["Save back to file"]
```

## Install from npm (recommended)

Prebuilt static binaries ship for `linux-x64`, `linux-arm64`,
`macos-x64`, `macos-arm64` and `win-x64`:

```sh
npm install fmtpp
npx fmtpp --version
```

```
fmtpp 0.1.0
```

If no prebuilt binary matches your platform, the `fmtpp` launcher
compiles from source with `zig cc` automatically. You can also pin a
binary explicitly:

```sh
FMTPP_BINARY=/path/to/fmtpp npx fmtpp format main.js
```

See [Installation](./installation.md) for all options, including
offline and per-platform notes.

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

Given `main.js`:

```js
function add(a,b){return a+b;}
```

```sh
fmtpp format main.js
```

```js
function add(a,b) {
  return a+b;

}
```

Single-file output goes to stdout and never touches the original file;
redirect it to rewrite in place:

```sh
fmtpp format main.js > main.js.tmp && mv main.js.tmp main.js
```

The shorthand `fmtpp main.js` formats one file the same way.
Multi-file runs print a per-file status report instead of code:

```sh
fmtpp format a.js b.go
```

```text
Formatting 2 files with 16 threads...
  Formatted: a.js
  Formatted: b.go
Done: 2 succeeded, 0 failed
The thread pool has done its parallel dance
```

Which `.fmtpp.toml` applies? The one found from the formatted file's
own directory upward — see [Configuration](./configuration.md).

## Core promises

Three behaviors you can rely on everywhere:

**1. Sources are never touched.** Single-file `format` writes to
stdout only. Nothing is rewritten unless you redirect.

**2. Unknown extensions fall back to JavaScript.** A file with no
recognized extension is parsed as JS:

```sh
fmtpp format snippet   # contains function f(){return 1;}
```

```js
function f() {
  return 1;

}
```

**3. Output is stable.** Formatting twice is byte-identical:

```sh
fmtpp format main.js > once.js
fmtpp format once.js > twice.js
diff once.js twice.js && echo STABLE
```

```text
STABLE
```

::: tip
New to formatters? Start with [Usage](./usage.md) for in-place
recipes, editor setup, and CI, then browse the
[Style reference](./style-reference.md) to see exactly what fmtpp does
to each of the 26 languages.
:::

## Next steps

- [Installation](./installation.md) — every install path in detail
- [Usage](./usage.md) — daily workflows, hooks, CI
- [CLI reference](./cli.md) — every command and flag
- [Configuration](./configuration.md) — `.fmtpp.toml`, `learn`, `config`
- [Languages](./languages.md) — per-language style notes
- [Style reference](./style-reference.md) — verified before/after pairs
