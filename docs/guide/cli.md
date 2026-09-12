# CLI reference

```sh
fmtpp [OPTIONS] [FILE]
fmtpp <command> [options]
```

## Commands

| Command | Description |
|---|---|
| `fmtpp format <file>` | Format one file, code to stdout (file untouched) |
| `fmtpp format <file...>` | Format N files in parallel, prints a status report |
| `fmtpp <file>` | Shorthand: format a single file |
| `fmtpp bench <file> [--runs N] [--compare <tool>]` | Benchmark formatting (`oxfmt`, `prettier` as comparison) |
| `fmtpp learn <file> [--output .fmtpp.toml]` | Detect style from a file, write config |
| `fmtpp config` | Interactive TUI configurator (needs a terminal) |

## Global flags

| Flag | Description |
|---|---|
| `--version, -v` | Print `fmtpp 0.1.0` |
| `--help, -h` | Print help |
| `--easter-egg` | Print a random easter egg |
| `--cowbell` | More cowbell! |
| `--kittens` | ASCII kitten approval |
| `--unicorn` | Formatting unicorn blessing |

## Benchmark

```sh
fmtpp bench app.js --runs 100
fmtpp bench app.js --runs 20 --compare oxfmt
```

Runs the formatter N times (with warmup) and prints average time.
With `--compare`, the baseline tool is timed on the same file and a
speedup report is printed.

## Exit codes

`0` on success. `format` on multiple files returns `1` if any file
failed; unreadable or unparsable files report `Error: ...` and return `1`.
