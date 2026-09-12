# Usage

fmtpp formats files from the command line. Remember two shapes:
single-file to stdout and multi-file status report.

Command shape for reference:

```text
fmtpp [OPTIONS] [files...] [SUBCOMMANDS]
```

Positional `files` formats a single file directly. The `format`
subcommand formats one or more files. Global flags are `-v/--version`,
`-h/--help`, `--easter-egg`, `--cowbell`, `--kittens`, and `--unicorn`.

Subcommands are `format`, `bench`, `config`, and `learn`. This page
covers `format` and the exit codes around it. See the dedicated pages
for benchmarking, configuration, and the Node API.

## Format one file to stdout

Pass one file either directly or through the `format` subcommand:

```sh
npx fmtpp format main.js
```

The formatted code goes to stdout. The original file stays untouched.
Review the output first, then decide whether to save it back.

The positional shorthand does the same thing for a single file:

```sh
npx fmtpp main.js
```

Use whichever shape reads better in your script. Both leave the input
file alone and print only formatted code, never a status report.

::: tip
Run the single-file form first on unfamiliar code. It is the only
form where you see the formatted result before anything is saved.
:::

## Rewrite a file in place

fmtpp has no in-place rewrite flag. Redirect stdout to a temporary
file, then move it over the original. That two-step keeps a bad run
from truncating the input.

sh recipe:

```sh
fmtpp format main.js > main.js.tmp && mv main.js.tmp main.js
npx fmtpp format main.js > main.js.tmp && mv main.js.tmp main.js
```

The first line uses a local `./build/fmtpp` on `PATH`. The second line
uses the npm launcher. Pick the one that matches your install.

PowerShell recipe (same idea, Windows syntax):

```sh
fmtpp format main.js > main.js.tmp
Move-Item -Force main.js.tmp main.js
```

Inspect `main.js.tmp` before the move on important files, and keep the
temporary file until the move succeeds.

::: warning
Never redirect straight back into the same file. Always go through a
temporary file and move it over only after the format run succeeds.
:::

## Format many files at once

Pass several files to the `format` subcommand:

```sh
fmtpp format a.js b.go c.rs
npx fmtpp format a.js b.go c.rs
```

Multi-file runs execute in parallel. Output is a per-file status
report, not formatted code. A typical report looks like:

```text
Formatting 3 files with 16 threads...
  Formatted: a.js
Done: 3 succeeded, 0 failed
```

Read it line by line: the first line says how many files were
attempted, each middle line reports one file, and the last line totals
successes and failures.

Because the report replaces the formatted code on stdout, do not use
the multi-file form when you want to capture code. Use the single-file
form from the previous section for that.

## Exit codes

| Situation | Exit code | What you see |
| --- | --- | --- |
| Success, single file | 0 | Formatted code on stdout |
| Success, multi-file | 0 | Per-file status report, zero failures |
| Any file failed, multi-file | 1 | Status report showing the failure count |
| Unreadable or unparsable file | 1 | `Error: ...` line for that file |

Exit code 0 means everything requested succeeded. Exit code 1 means at
least one file failed. Check the status report or the `Error: ...`
line to find which file caused the nonzero exit.

Single-file runs are simple: success prints code with exit 0, failure
prints an error with exit 1. Multi-file runs aggregate: one bad file
flips the whole run to 1. Gate scripts on the exit code rather than on
parsing stdout.

## Editor integration

Wire fmtpp up as a generic external tool. No plugin is required. The
pattern is the same in every editor:

1. Point the editor at the fmtpp binary or at `npx fmtpp`.
2. Set the tool command to `fmtpp format` with the current file.
3. Capture stdout back into the editor buffer or a preview pane.
4. Keep the on-disk file untouched until you explicitly save.

Concrete recipe with the npm launcher on the current file:

```sh
npx fmtpp format main.js
```

Concrete recipe with a pinned binary:

```sh
FMTPP_BINARY=/path/to/fmtpp npx fmtpp format main.js
```

Concrete recipe with a source build:

```sh
./build/fmtpp format main.js
```

All three print formatted code to stdout and leave the file alone.
Let the editor handle saving. Do not add shell redirection inside the
editor command itself.

## Pre-commit hook

Run `fmtpp format` on staged files and block the commit when formatting
fails. Keep the hook small so it stays fast.

Example hook calling the `format` subcommand:

```sh
#!/bin/sh
npx fmtpp format main.js
```

Extend that line to each file your commit touches. A nonzero exit
means that file needs attention before the commit lands. Multi-file
mode already prints a status report and returns 1 when any file
failed, so the hook just forwards that exit code to git.

## Continuous integration

Run the same commands in CI that you run locally. Verify the binary
first, then format the files under test.

Minimal CI snippet with npm:

```sh
npm install fmtpp
npx fmtpp --version
npx fmtpp format a.js b.go c.rs
```

Minimal CI snippet with a source build:

```sh
cmake -S . -B build
cmake --build build --config Release
./build/fmtpp --version
./build/fmtpp format a.js b.go c.rs
```

Both snippets rely on exit codes. A failed file returns 1 and fails
the step, which is what you want in CI.

Pin the binary in CI when reproducibility matters:

```sh
FMTPP_BINARY=/path/to/fmtpp npx fmtpp format main.js
```

That line guarantees CI formats with the exact binary you tested
locally.

## Quick reference

| Task | Command |
| --- | --- |
| Show version | `npx fmtpp --version` |
| Format one file to stdout | `fmtpp format main.js` |
| Shorthand single file | `fmtpp main.js` |
| Format several files | `fmtpp format a.js b.go c.rs` |
| Pinned binary | `FMTPP_BINARY=/path/to/fmtpp npx fmtpp format main.js` |
| Benchmark a file | `fmtpp bench app.js --runs 100` |
| Learn style to config | `fmtpp learn app.js --output .fmtpp.toml` |
| Interactive config | `fmtpp config` |

When in doubt, compare against the authoritative usage:

```sh
npx fmtpp --help
```
