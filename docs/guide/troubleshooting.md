# Troubleshooting

This page covers the most common surprises in fmtpp v0.1.0 (26 languages,
38 CTest suites). Each section is one symptom, its cause, and the fix.

## Config file seems ignored

fmtpp reads `.fmtpp.toml` (TOML, parsed with toml++). Resolution:

1. Start in the formatted file's own directory.
2. Walk up parent directories.
3. First `.fmtpp.toml` found wins.
4. Missing or unparseable file means silent defaults.
5. Unknown keys are ignored.

The config is anchored to the file being formatted, not to your shell's
working directory. A `.fmtpp.toml` in your home directory does nothing
for a file in `/tmp/proj/` unless `/tmp/proj/` or one of its parents
contains it.

For example, `src/.fmtpp.toml` beats the repo-root `.fmtpp.toml` for
everything under `src/`.

```toml
printWidth = 100
singleQuote = true
braceStyle = "same-line"
```

::: tip
Copy the suspect config next to the file you format and retry. If the
behavior changes, the problem was resolution order, not the keys.
:::

If the file is unparseable TOML, fmtpp falls back to defaults without
an error. Validate the TOML syntax first, then check for typos: unknown
keys are silently ignored.

## Wrong language output

Language is detected from the file extension by
`FormatterFactory::detectLanguage`. There is no content sniffing.

Fallback rule: NULL path, no extension, or unknown extension formats
as `js`.

Full mapping:

| Extension | Language |
|---|---|
| `js`, `mjs`, `cjs`, `jsx` | `js` |
| `ts`, `tsx`, `mts`, `cts` | `ts` |
| `java` | `java` |
| `c`, `h` | `c` |
| `cpp`, `cc`, `cxx`, `hpp`, `hh`, `hxx`, `ino`, `cu`, `cuh` | `cpp` |
| `cs` | `cs` |
| `go` | `go` |
| `rs` | `rs` |
| `kt`, `kts` | `kotlin` |
| `swift` | `swift` |
| `dart` | `dart` |
| `php`, `phtml` | `php` |
| `scala`, `sc` | `scala` |
| `m`, `mm` | `objc` |
| `zig`, `zon` | `zig` |
| `d`, `di` | `d` |
| `groovy`, `gvy` | `groovy` |
| `sol` | `solidity` |
| `v`, `vsh`, `vv` | `v` |
| `glsl`, `vert`, `frag`, `geom`, `comp`, `tesc`, `tese`, `vs`, `fs` | `glsl` |
| `html`, `htm`, `xhtml`, `svg`, `xml` | `html` |
| `css`, `scss`, `less` | `css` |
| `json` | `json` (formatted by `JSFormatter`) |
| `vue` | `vue` |
| `py`, `pyw`, `pyi` | `python` |
| `rb` | `ruby` |

Surprising cases that are working as intended:

| File | Detected as | Why it surprises |
|---|---|---|
| `foo.h` | `c`, not `cpp` | C++ headers still format as C unless renamed to `.hpp` / `.hh` / `.hxx` |
| `foo.m` | `objc`, not MATLAB | `.m` / `.mm` belong to Objective-C here |
| `foo.v` | `v`, not Verilog | `.v` / `.vsh` / `.vv` belong to the V language |
| `foo.sc` | `scala`, not SuperCollider | `.sc` maps to Scala |
| `foo.fs` | `glsl`, not F# | `.fs` is treated as a fragment-shader extension |
| `foo.xml`, `foo.svg` | `html` | markup family shares the HTML formatter |
| `foo.scss`, `foo.less` | `css` | preprocessor files share the CSS formatter |
| `foo.json` | `json` via `JSFormatter` | same engine as JavaScript, compact output |
| `Makefile`, `README`, `data.xyz` | `js` | no extension or unknown extension falls back to `js` |

If output looks like the wrong language, rename or check the extension
first. There is no override flag; the extension is the language.

## Output differs from Prettier or gofmt

fmtpp follows reference behavior per language family. It is not
byte-identical to every upstream tool. Known intentional differences:

- C functions use same-line braces:

```cpp
int add(int a, int b) {
  return a + b;
}
```

- C++ functions use Allman style:

```cpp
int f()
{
  return 0;
}
```

- C structs print the opening brace on its own line and the closing
  `}` without a trailing semicolon:

```text
struct Node
{
  int value;
}
```

- TypeScript methods print a newline before the brace, matching the
  TypeScript formatter's reference output.
- JSON prints compact single-line output via `JSFormatter`.
- Python keeps colon suites; Ruby keeps `end` suites. They do not
  convert to brace style.
- Unknown constructs become verbatim `RawTextNode` output. The
  formatter never invents syntax to make code look prettier.

If a brace lands on the next line (or the same line) and the other
tool does the opposite, check the family rule above before filing a
bug. Include the language and a minimal snippet when reporting.

## Invalid code is kept verbatim

Formatters never corrupt input. Invalid input is kept verbatim:

```text
input with syntax errors -> same text back, formatted around it
```

Unknown or unparseable constructs become `RawTextNode` and pass
through unchanged. You always get your code back, never invented code.

This means a broken file formats "successfully" but looks unchanged.
Fix the syntax error first, then reformat to see the real output.

::: warning
Do not rely on the formatter to find syntax errors. Verbatim
passthrough is a safety feature, not a diagnostic.
:::

## Formatting is idempotent

Formatting twice gives the same result as formatting once. The
end-to-end suite enforces this with a double-format check on every
fixture.

If you see output change on the second run, that is a bug. Report it
with the original snippet, the first output, and the second output.

```sh
ctest --test-dir build --output-on-failure
```

## Blank lines and brace placement move

They shouldn't, beyond one blank per gap. fmtpp preserves input blank
lines (up to `maxBlankLines`) and preserves Allman `catch`/`else`/
`finally` placement (`} catch` stays split when the source splits it).
Set `preserveBlankLines = false` for fully compact output, or
`catchOnNewLine`/`elseOnNewLine`/`finallyOnNewLine` to `"always"` or
`"never"` to force one style. C++ functions default to Allman and C#
to Allman; override with `braceStyleFunctions = "same-line". See
[Configuration](./configuration.md) for the full option list.

## Empty and comments-only files

Empty files stay empty. Comments-only files keep their comments and
nothing else is added. The end-to-end suite covers both cases with
dedicated `empty` and `comments_only` fixtures per language.

If an empty file gains content, or a comments-only file loses
comments, report it with the file extension and exact input.

## Exit code 1 when formatting multiple files

When formatting several files at once, one failure fails the run.
The exit code is 1 and the output names which file failed.

Rerun with fewer files to isolate the failure, then report the failing
file's extension plus a minimal snippet that reproduces it.

## `learn` writes more keys than the docs table

`fmtpp learn` generates a complete style file. The short options table
in the configuration guide shows the most commonly changed keys, but
the generated file lists every real key.

```sh
fmtpp learn example.js --output .fmtpp.toml
fmtpp config
```

`learn` detects style from a file (indent, brace placement,
semicolons, trailing commas, print width, spacing, final newline) and
writes the result as TOML. `config` opens the interactive TUI editor.

Every key `learn` writes is real. Full list with defaults:

| Key | Default |
|---|---|
| `printWidth` | `80` |
| `useTabs` | `false` |
| `tabWidth` | `2` |
| `semi` | `true` |
| `singleQuote` | `false` |
| `trailingComma` | `"es5"` |
| `bracketSpacing` | `true` |
| `arrowParens` | `"always"` |
| `endOfLine` | `"lf"` |
| `braceStyle` | `"same-line"` or `"next-line"` (Allman) |
| `spaceBeforeBrace` | `true` |
| `spaceAfterKeyword` | `true` |
| `spaceAroundOperators` | `true` |
| `proseWrap` | `"preserve"` |
| `htmlWhitespaceSensitivity` | `"css"` |
| `vueIndentScriptAndStyle` | `false` |
| `embeddedLanguageFormatting` | `"auto"` |
| `singleAttributePerLine` | `false` |

If a key is missing from the short table, it is still supported. Do
not delete generated keys to "match the docs".

## `test_threadpool` rarely segfaults under load

Honest known flake: `test_threadpool` shares one global `MemoryPool`
across worker threads, so it can rarely segfault under load. It passes
on re-run. The proper fix is per-thread pools and is pending.

```sh
ctest --test-dir build --output-on-failure
```

If only `test_threadpool` fails and everything else passes, rerun it
in isolation before investigating further.

## Encoding and parse errors

`fmtpp` expects UTF-8 **without** a byte-order mark. A BOM poisons
the first token, so a BOM'd file can misformat or fail outright:

```sh
fmtpp format bom.json
```

```text
Error: Cannot parse file: bom.json
```

Strict JSON rejects malformed input instead of guessing:

```sh
fmtpp format bad.json   # contains {"a":}
```

```text
Error: Cannot parse file: bad.json
```

If you hit `Cannot parse file`, check in order: file encoding (strip
the BOM), balanced brackets, and that the extension matches the
content. For every other language, unparsable stretches pass through
verbatim as `RawTextNode` — only the strict JSON mini-parser hard
fails. `Cannot read file` instead means the path is wrong or
unreadable (permissions, missing file).

## Where to report

File bugs in the repo issue tracker. Include all four items:

1. fmtpp version (for example, `v0.1.0`).
2. File extension and detected language.
3. Minimal snippet that reproduces the problem.
4. Your `.fmtpp.toml`, or a note that you used defaults.

```text
version: v0.1.0
extension: .ts
snippet: (shortest code that shows the bug)
config: defaults (or paste .fmtpp.toml)
```

The snippet matters most. A 5-line reproduction gets fixed faster than
a full project dump.
