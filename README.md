# fmtpp — Format++

A professional-grade, multi-language code formatter written in C++.
One static binary, twenty-six languages, zero runtime dependencies.

Full guides live on the docs site: `docs/` (run `npm run docs:dev` inside `docs/`).

## Supported languages

The formatter is picked from the file extension. Unknown extensions
fall back to JavaScript.

| Language | Extensions | Reference style |
|---|---|---|
| JavaScript | `.js` `.mjs` `.cjs` `.jsx` | Prettier |
| TypeScript | `.ts` `.tsx` `.mts` `.cts` | Prettier |
| Java | `.java` | google-java-format |
| C | `.c` `.h` | clang-format |
| C++ | `.cpp` `.cc` `.cxx` `.hpp` `.hh` `.hxx` `.ino` `.cu` `.cuh` | clang-format |
| C# | `.cs` | `dotnet format` |
| Go | `.go` | gofmt |
| Rust | `.rs` | rustfmt |
| Kotlin | `.kt` `.kts` | ktfmt |
| Swift | `.swift` | swift-format |
| Dart | `.dart` | dart format |
| PHP | `.php` `.phtml` | php-cs-fixer |
| Scala | `.scala` `.sc` | scalafmt |
| Objective-C | `.m` `.mm` | clang-format |
| Zig | `.zig` `.zon` | zig fmt |
| D | `.d` `.di` | dfmt |
| Groovy | `.groovy` `.gvy` | Spotless |
| Solidity | `.sol` | prettier-plugin-solidity |
| V | `.v` `.vsh` `.vv` | v fmt |
| GLSL | `.glsl` `.vert` `.frag` `.geom` `.comp` `.tesc` `.tese` `.vs` `.fs` | clang-format |
| HTML/XML | `.html` `.htm` `.xhtml` `.svg` `.xml` | Prettier |
| CSS | `.css` `.scss` `.less` | Prettier |
| JSON | `.json` | Prettier |
| Vue SFC | `.vue` | Prettier/Volar |
| Python | `.py` `.pyw` `.pyi` | black |
| Ruby | `.rb` | rubocop |

## Install

```sh
npm install formatpp
npx fmtpp --version
```

Prebuilt static binaries ship for `linux-x64`, `linux-arm64`,
`macos-x64`, `macos-arm64` and `win-x64` under `npm/`. The launcher
(`index.js`) picks the right one, or compiles from source with `zig
cc` when none matches. Pin a binary explicitly with
`FMTPP_BINARY=/path/to/fmtpp`.

From source (CMake ≥ 3.28, Ninja/Make, C++11 compiler; CLI11 and
toml++ are vendored under `lib/`):

```sh
cmake -S . -B build
cmake --build build --config Release
./build/fmtpp --version
```

## Usage

```sh
fmtpp format main.js          # code to stdout, file untouched
fmtpp main.js                 # shorthand for a single file
fmtpp format a.js b.go c.rs  # parallel run, prints a status report
fmtpp bench app.js --runs 100
fmtpp learn example.js        # detect style, write .fmtpp.toml
fmtpp config                  # interactive TUI configurator
```

Single-file mode prints code; multi-file mode prints progress.
Details: `docs/guide/cli.md`.

## Configuration

`.fmtpp.toml` (TOML, Prettier-like defaults) is resolved from the
formatted file's directory upward. Unknown keys are ignored, broken
files fall back to defaults. Options and `learn`/`config`
workflows: `docs/guide/configuration.md`.

## Tests and layout

```sh
ctest --test-dir build --output-on-failure   # 35 CTest suites
npm test                                     # npm packaging tests
```

```text
include/fmtpp/   public headers (formatters, AST, infra)
src/             implementation (C++11, Java-style OOP)
tests/           suites + tests/fixtures/ language samples
docs/            VitePress site
lib/             vendored CLI11 + toml++ mirrors
npm/             prebuilt binary slots per platform-arch
scripts/         cross-compilation (zig cc) helpers
```

How it parses and prints: `docs/guide/architecture.md`.

## License

MIT — see `package.json` (`formatpp` on npm, code at
`github.com/fmtpp/fmtpp`).
