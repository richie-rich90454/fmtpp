# Installation

fmtpp v0.1.0 installs from npm as a prebuilt static binary. When no
prebuilt binary matches your machine, the launcher compiles from source
automatically. You can also build from source by hand with CMake.

## Install with npm (recommended)

```sh
npm install fmtpp
npx fmtpp --version
```

The first command downloads the package including the prebuilt
binaries. The second command runs the launcher, which resolves the
correct binary for your platform and prints the version.

A successful install prints:

```text
fmtpp 0.1.0
```

If you see that string, the binary resolved and executed correctly.
Otherwise jump to the troubleshooting table before retrying.

## Prebuilt targets

Five static binaries ship with the npm package. The launcher picks one
based on the operating system and CPU architecture:

| Target | Operating system | Architecture |
| --- | --- | --- |
| `linux-x64` | Linux | x64 |
| `linux-arm64` | Linux | arm64 |
| `macos-x64` | macOS | x64 |
| `macos-arm64` | macOS | arm64 |
| `win-x64` | Windows | x64 |

These are static binaries, so no compiler or shared runtime is needed
when one matches. The launcher matches your platform the same way
Node.js reports it, then runs that binary directly with your arguments.

## Pin a binary with FMTPP_BINARY

Use the `FMTPP_BINARY` environment variable when you want to skip
platform detection and run one specific binary:

```sh
FMTPP_BINARY=/path/to/fmtpp npx fmtpp format main.js
```

In that example the launcher runs `/path/to/fmtpp` directly and passes
it `format main.js`. Use an absolute path.

Pinning is useful when you built from source and want the launcher to
use that build, when you keep a known-good binary outside the package,
or when you are testing a freshly compiled binary.

::: tip
Keep the default resolution for day-to-day use. Pin `FMTPP_BINARY`
only when you can name the exact binary you want and why.
:::

## Automatic source fallback

When no prebuilt binary matches your platform and architecture, the
launcher does not stop. It compiles from source automatically with
`zig cc`.

That fallback needs `zig cc` on the machine. On a supported platform
you never hit this path. You hit it on an unusual CPU, a new OS
release, or a container reporting an unexpected platform.

What to expect:

- The first run takes longer because it compiles; later runs reuse it.
- The compiled binary behaves the same as a prebuilt one.

If the fallback cannot compile, it reports the missing piece. The most
common cause is a missing `zig cc` (see below).

## Build from source

Build from source for a binary at a known path, an unsupported
platform, or work on fmtpp itself.

Requirements:

- CMake >= 3.16, Ninja (or Make), and a C++11 compiler (see table below).

CLI11 and toml++ are vendored under `lib/` and need no separate
install.

Compiler support:

| Compiler | Minimum version | Notes |
| --- | --- | --- |
| GCC | 5+ | C++11 mode |
| Clang | 6+ | C++11 mode |
| MSVC | 2019+ | C++11 mode |

Configure and build from the repository root:

```sh
cmake -S . -B build
cmake --build build --config Release
```

The finished binary lands at:

```text
./build/fmtpp
```

Run it directly to confirm the build:

```sh
./build/fmtpp --version
```

You should see:

```text
fmtpp 0.1.0
```

That is the same string the npm launcher prints for v0.1.0.

::: tip
Use a clean `build` directory per configuration. Delete it and
re-run the two `cmake` commands above if a previous configure step
was interrupted halfway.
:::

## Verify the install

Run one command for the path you installed through.

npm install path:

```sh
npx fmtpp --version
```

Manual source build path:

```sh
./build/fmtpp --version
```

Pinned binary path:

```sh
FMTPP_BINARY=/path/to/fmtpp npx fmtpp format main.js
```

The first two commands must print:

```text
fmtpp 0.1.0
```

The third formats `main.js` to stdout with the pinned binary; the
original file stays untouched.

## Troubleshooting

| Problem | Likely cause | Fix |
| --- | --- | --- |
| Unsupported platform or architecture error | No prebuilt binary matches this machine | Install `zig cc` so the automatic source fallback can compile, or build from source with CMake |
| Compilation fails mentioning `zig cc` | `zig cc` is missing | Install `zig cc`, confirm it runs, then retry the same command |
| Stale binary: old behavior after upgrade | Launcher resolves a pinned or cached binary from before the upgrade | Check `FMTPP_BINARY`, point it at the new binary or unset it, then re-run `npx fmtpp --version` |

### Unsupported platform or architecture

This happens on machines outside the five prebuilt targets. Compile
instead of retrying the prebuilt path:

```sh
cmake -S . -B build
cmake --build build --config Release
./build/fmtpp --version
```

Once that prints `fmtpp 0.1.0`, run `./build/fmtpp` directly or point
the launcher at it:

```sh
FMTPP_BINARY=/path/to/fmtpp npx fmtpp format main.js
```

### Missing zig

The automatic fallback needs `zig cc`. Install it, then re-run the
exact command that failed. There is no extra fmtpp-side configuration.

### Stale binary

A stale binary usually means `FMTPP_BINARY` still points at an old
build. Update it to the new binary or unset it, then verify again:

```sh
npx fmtpp --version
```

::: warning
Unset `FMTPP_BINARY` when you are done testing a pinned binary.
A forgotten pin silently overrides every later `npx fmtpp` call.
:::

## Next steps

```sh
npx fmtpp format main.js
```

Single-file output goes to stdout; multi-file runs print a status
report instead.
