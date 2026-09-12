# Node API

The npm package ships a small Node launcher at `index.js`. It resolves
the right binary for the current machine and spawns it with your
arguments. You can also require it and call its helpers directly.

This page covers the four exports, the five platform keys, the
resolution order in words, error cases, and a complete example script
that formats a file.

## Exports

Require the package and pick the helper you need:

```js
const fmtpp = require('fmtpp');
```

| Export | Signature | What it does |
| --- | --- | --- |
| `getPlatformKey()` | `getPlatformKey() -> string` | Returns the platform key for this machine |
| `getPrebuiltBinaryPath()` | `getPrebuiltBinaryPath() -> string` | Returns the path to the prebuilt binary for this machine |
| `resolveBinaryPath()` | `resolveBinaryPath() -> string` | Returns the binary to run, honoring overrides and fallback |
| `compileFromSource()` | `compileFromSource() -> string` | Compiles from source and returns the built binary path |

Use `getPlatformKey()` to branch on platform,
`getPrebuiltBinaryPath()` for the bundled path without fallback,
`resolveBinaryPath()` for the normal "which binary would run" answer,
and `compileFromSource()` only to force a source build.

The launcher's `main()` calls `resolveBinaryPath()`, then `spawnSync`s
the binary with `process.argv` and exits with that child's status.

::: tip
Prefer `resolveBinaryPath()` over the other three in application
code. It encodes the supported order so you do not have to.
:::

## Platform keys

`getPlatformKey()` maps Node's OS and CPU values to one of five keys:

| Platform key | Node OS | Node architecture | Binary path |
| --- | --- | --- | --- |
| `win-x64` | `win32` | `x64` | `npm/win-x64/fmtpp.exe` |
| `macos-x64` | `darwin` | `x64` | `npm/macos-x64/fmtpp` |
| `macos-arm64` | `darwin` | `arm64` | `npm/macos-arm64/fmtpp` |
| `linux-x64` | `linux` | `x64` | `npm/linux-x64/fmtpp` |
| `linux-arm64` | `linux` | `arm64` | `npm/linux-arm64/fmtpp` |

The binary lives under `npm/<key>/` inside the installed package
(`.exe` on Windows).

Example inspection script:

```js
const fmtpp = require('fmtpp');
const key = fmtpp.getPlatformKey();
console.log(key);
```

On an Intel Mac that prints `macos-x64`; on ARM Linux, `linux-arm64`:

```text
macos-x64
```

When the key is outside this table, there is no matching prebuilt
binary. That is exactly when `resolveBinaryPath()` falls through to
compiling from source.

## Resolution order in words

`resolveBinaryPath()` answers "which binary runs" in three steps,
always in this order. First match wins.

Step one: check the `FMTPP_BINARY` environment variable. When it is
set, use that path directly. Skip all platform detection and skip any
compilation. This is the pinning path:

```sh
FMTPP_BINARY=/path/to/fmtpp npx fmtpp format main.js
```

Step two: look for the prebuilt binary for the current platform key.
This is the common path on all five supported targets.

Step three: compile from source. When no prebuilt binary matches,
run `compileFromSource()` automatically and use the binary it
produces. This path needs `zig cc` plus the normal CMake toolchain.

In one sentence: explicit pin beats prebuilt binary beats automatic
source compile.

::: warning
Treat `FMTPP_BINARY` as an override, not a default. A stale pin
hides platform upgrades and source rebuilds until you unset it.
:::

## compileFromSource

`compileFromSource()` runs a two-command CMake build and returns the
resulting binary path. It uses a toolchain file, unlike the manual
build in the installation guide.

The two commands it runs are:

```sh
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake
cmake --build build --config Release
```

Needs CMake >= 3.16, Ninja (or Make), a C++11 compiler (GCC 5+,
Clang 6+, MSVC 2019+), and `zig cc` for the toolchain file.

Call it directly only when you want to force a rebuild:

```js
const fmtpp = require('fmtpp');
const built = fmtpp.compileFromSource();
console.log(built);
```

The printed path points at the freshly built binary, normally under
`build/`. Confirm it:

```sh
./build/fmtpp --version
```

Which should print:

```text
fmtpp 0.1.0
```

## Error cases

| Situation | What happens | Fix |
| --- | --- | --- |
| `FMTPP_BINARY` points nowhere | Spawn fails; the pinned path does not exist | Point it at a real binary or unset it |
| No prebuilt match and no `zig cc` | Automatic compile fails | Install `zig cc`, then retry |
| CMake configure fails | `compileFromSource()` reports the CMake error | Install CMake >= 3.16 and a C++11 compiler, then retry |
| Unsupported OS or CPU | No platform key matches the table above | Build from source and pin with `FMTPP_BINARY` |

Diagnose in order:

```js
const fmtpp = require('fmtpp');
console.log(fmtpp.getPlatformKey());
console.log(fmtpp.getPrebuiltBinaryPath());
```

Compare the key against the five-row table. When it matches but the
binary does not run, suspect a stale install or a bad pin. When the
key falls outside the table, go straight to the source-build path.

A quick shell-side check covers the same ground:

```sh
npx fmtpp --version
FMTPP_BINARY=/path/to/fmtpp npx fmtpp format main.js
```

The first line tests default resolution. The second tests the pinned
binary. When the first fails and the second succeeds, the problem is
resolution, not the formatter itself.

## Example: format a file from Node

This script resolves the binary, spawns `format` on one file, and
forwards the child's exit status. It mirrors what the launcher's
`main()` does with `spawnSync` and `process.argv`.

```js
const { spawnSync } = require('child_process');
const fmtpp = require('fmtpp');

const binary = fmtpp.resolveBinaryPath();
const result = spawnSync(binary, ['format', 'main.js'], { encoding: 'utf8' });

console.log(result.stdout);
console.error(result.stderr);
process.exit(result.status);
```

Notes on that script:

- `resolveBinaryPath()` honors `FMTPP_BINARY` first.
- `['format', 'main.js']` formats one file to stdout; the file stays
  untouched.
- `result.status` is the formatter's exit code: 0 on success, 1 when
  the file failed.

Pin from the shell around any Node script when CI must use one exact
binary:

```sh
FMTPP_BINARY=/path/to/fmtpp npx fmtpp format main.js
```

## Quick reference

```js
const fmtpp = require('fmtpp');
fmtpp.getPlatformKey();
fmtpp.getPrebuiltBinaryPath();
fmtpp.resolveBinaryPath();
fmtpp.compileFromSource();
```

| Task | Helper |
| --- | --- |
| Which platform am I on | `getPlatformKey()` |
| Where is the bundled binary | `getPrebuiltBinaryPath()` |
| Which binary should run | `resolveBinaryPath()` |
| Force a source build | `compileFromSource()` |

All four helpers come from `index.js`. `FMTPP_BINARY` overrides
everything; source compilation needs `zig cc`.
