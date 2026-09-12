# Migrating from other formatters

fmtpp aims for reference-like output, not byte-for-byte compatibility.
Use this page to translate your existing setup and set expectations.

## From Prettier (JS/TS/HTML/CSS/JSON/Vue)

Most Prettier options map one-to-one:

| Prettier | fmtpp key | Notes |
|---|---|---|
| `printWidth` | `printWidth` | Same default `80` |
| `useTabs` | `useTabs` | Same default `false` |
| `tabWidth` | `tabWidth` | Same default `2` |
| `semi` | `semi` | Same default `true` |
| `singleQuote` | `singleQuote` | Same default `false` |
| `trailingComma` | `trailingComma` | Same default `"es5"` |
| `bracketSpacing` | `bracketSpacing` | Same default `true` |
| `arrowParens` | `arrowParens` | `"always"` default; `"as-needed"` supported |
| `endOfLine` | `endOfLine` | Same default `"lf"` |
| `proseWrap` | `proseWrap` | Same default `"preserve"` |
| `htmlWhitespaceSensitivity` | `htmlWhitespaceSensitivity` | Same default `"css"` |
| `vueIndentScriptAndStyle` | `vueIndentScriptAndStyle` | Same default `false` |
| `embeddedLanguageFormatting` | `embeddedLanguageFormatting` | Same default `"auto"` |
| `singleAttributePerLine` | `singleAttributePerLine` | Same default `false` |

fmtpp-only keys with no Prettier equivalent: `braceStyle`,
`spaceBeforeBrace`, `spaceAfterKeyword`, `spaceAroundOperators`.

If you have a `.prettierrc`, the fastest migration is:

```sh
fmtpp learn src/index.js --output .fmtpp-learned.toml
```

then copy each value across using the table above (TOML syntax differs
from JSON/YAML, so translate key by key rather than copying the file).
Diffing `learn` output against your translated config surfaces every
option where fmtpp would format differently from Prettier today.

::: tip
`learn` detects what your code actually looks like; diffing its output
against your translated config surfaces every option where fmtpp would
format differently from Prettier today.
:::

Differences to expect: control-flow bodies may use Allman braces where
Prettier keeps them same-line, blank lines appear around block
boundaries, and object literals keep their spacing style from
`bracketSpacing` only.

## From clang-format (C/C++/Java/C#/Objective-C)

There is no `.clang-format` importer — translate the handful of keys
you actually set:

| clang-format | fmtpp equivalent |
|---|---|
| `BasedOnStyle: Google/Chromium` + `IndentWidth: N` | `tabWidth = N`, `useTabs = false` |
| `UseTab: Always` | `useTabs = true` |
| `ColumnLimit: N` | `printWidth = N` |
| `BreakBeforeBraces: Attach` | `braceStyle = "same-line"` |
| `BreakBeforeBraces: Allman` | `braceStyle = "next-line"` |
| `SpaceBeforeParens: ControlStatements` | `spaceAfterKeyword = true` |

C++ output uses Allman function braces and keeps `class`/`struct`/
`union` kinds with trailing `;`; C uses same-line function braces.
`#include`/`#define`/`#pragma`/`#version` lines pass through verbatim.

## From gofmt (Go)

No config translates — gofmt has no options and fmtpp's Go output
follows the same conventions (`func`, `:=`, `type … struct`,
`package` blank line, semicolons only in `for` headers). Run both on
your tree and diff; anything beyond whitespace is a fmtpp bug worth
reporting (see [Troubleshooting](./troubleshooting.md)).

## From rustfmt (Rust)

Same story: no options to translate. `fn … -> Ret`, `let [mut]`,
`match`, `impl`/`trait` blocks follow rustfmt-like shapes. `vec![…]`
and `|…|` closures are preserved.

## From black (Python)

| black | fmtpp equivalent |
|---|---|
| default 88 cols | `printWidth = 88` |
| `--skip-magic-trailing-comma` | n/a (fmtpp has no equivalent) |
| string normalization off | `singleQuote` as needed |

fmtpp normalizes suite indent to two spaces and keeps `:`/`else`/
`except`/`finally`/`case` structure. Decorators, `async`, and
`match`/`case` are preserved.

## From rubocop / StandardRB (Ruby)

No option mapping — keep `end` suites, `def`/`class`/`module`,
`if`/`elsif`, `case`/`when`, `begin`/`rescue`/`ensure`, `do` blocks,
heredocs and `%`-literals. fmtpp preserves heredoc bodies and
`=begin` blocks byte-for-byte.

## General advice

1. Format one representative file per language first and review the
   diff before bulk-converting.
2. Commit the clean tree, run `fmtpp format` over it, and review as a
   single commit — never mix formatting with behavior changes.
3. Check in the `.fmtpp.toml` files so every machine formats the same.
4. Wire the [pre-commit hook](./usage.md#pre-commit-hook) so the tree
   stays formatted.
