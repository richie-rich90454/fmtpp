# Configuration

`fmtpp` reads `.fmtpp.toml` (TOML, parsed with toml++). Defaults mirror
Prettier.

## Config resolution

`format` looks for `.fmtpp.toml` starting in the **formatted file's own
directory**, then each parent directory (same rule as Prettier). The
first config found wins; a missing or unparseable file silently falls
back to defaults. Unknown keys are ignored. If your config seems
ignored, check it sits next to (or above) the file you format — not
just in your shell's working directory.

## Generating config

```sh
fmtpp learn example.js --output .fmtpp.toml  # detect style from a file
fmtpp config                                 # edit interactively (TUI)
```

`learn` runs `StyleDetector` over the file (indent, brace placement,
semicolons, trailing commas, print width, spacing, final newline) and
writes the result as TOML.

## Options

| Key | Type | Default | Description |
|---|---|---|---|
| `printWidth` | int | `80` | Max line width before wrapping |
| `useTabs` | bool | `false` | Tabs instead of spaces |
| `tabWidth` | int | `2` | Spaces per indent level |
| `semi` | bool | `true` | Semicolons at statement ends |
| `singleQuote` | bool | `false` | Single instead of double quotes |
| `trailingComma` | string | `"es5"` | `"none"`, `"es5"` or `"all"` |
| `bracketSpacing` | bool | `true` | Spaces inside `{ braces }` |
| `arrowParens` | string | `"always"` | Parens around arrow-fn params |
| `endOfLine` | string | `"lf"` | `"lf"`, `"crlf"`, `"cr"`, `"auto"` |
| `braceStyle` | string | `"same-line"` | `"same-line"` (K&R) or `"next-line"` (Allman) |
| `spaceBeforeBrace` | bool | `true` | Space before `{` |
| `spaceAfterKeyword` | bool | `true` | Space after `if`, `for`, `while` |
| `spaceAroundOperators` | bool | `true` | Spaces around binary operators |

## Example

```toml
printWidth = 100
useTabs = false
tabWidth = 2
semi = true
singleQuote = true
trailingComma = "all"
bracketSpacing = true
braceStyle = "same-line"
```
