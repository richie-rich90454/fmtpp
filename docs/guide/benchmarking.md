# Benchmarking

Use `fmtpp bench` to time formatting on real files. It runs the
formatter N times with a warmup phase, then reports timing. Add
`--compare` to time a baseline tool on the same file for a speedup
report.

This page covers the syntax, an annotated transcript from a real run,
how to read warmup versus iterations, comparing against `oxfmt` and
`prettier`, and tips for stable numbers.

## Syntax

The `bench` subcommand needs at least one file:

```sh
fmtpp bench app.js --runs 100
```

Options:

| Option | Meaning |
| --- | --- |
| `files` | One or more files to benchmark (required) |
| `--runs N` | Run the formatter N times and average |
| `--compare <tool>` | Also time `oxfmt` or `prettier` on the same file |

A comparison run looks like:

```sh
fmtpp bench app.js --runs 20 --compare oxfmt
```

Swap in `prettier` for the other supported baseline:

```sh
fmtpp bench app.js --runs 20 --compare prettier
```

Those are the only two baseline names. Anything else is not a
supported comparison target.

::: tip
Start with a small `--runs` value while iterating, then raise it for
the final numbers you plan to quote or act on.
:::

## Annotated real transcript

Below is the shape of a real small run. Timing lines vary by machine,
so focus on the structure: header, warmup, then one line per
iteration.

Run:

```sh
fmtpp bench app.js --runs 3
```

Output shape:

```text
fmtpp Benchmark
Files: 1
Iterations: 3
Language: js
Warmup: yes
The benchmarker is warming up...
Warmup run...
Iteration 1/3...
Iteration 2/3...
Iteration 3/3...
```

Line by line, the header names the tool (`fmtpp Benchmark`), then
reports the inputs: `Files: 1` matches the single `app.js` argument,
`Iterations: 3` matches `--runs 3`, `Language: js` is the detected
language driving which parser path gets timed, and `Warmup: yes`
confirms a warmup phase ran before timing.

```text
The benchmarker is warming up...
Warmup run...
```

The warmup phase absorbs one-time costs like cold caches. It is
intentionally excluded from the reported average, so never compare
warmup timing against iteration timing.

```text
Iteration 1/3...
Iteration 2/3...
Iteration 3/3...
```

The timed iterations. Each formats the same input under the same
conditions, and the reported average comes from these lines.

## Warmup versus iterations

Warmup and iterations answer different questions. Keep them separate
when reading a report.

Warmup measures cold start: cold caches, first-allocation costs, and
lazy initialization. It is real work but not representative of
repeated formatting. Iterations measure steady state: the cost once
the machine has settled. That is the number to quote.

Practical rules:

- Never average the warmup run into the iteration times.
- Never compare one run's warmup against another run's iterations.
- Do compare iteration averages across runs on the same machine.
- Do re-run when the first iteration looks wildly different from the rest.

The header tells you both inputs at a glance. `Iterations` sets how
many timed samples you get. `Warmup: yes` confirms the samples exclude
cold-start effects.

## Comparing against oxfmt and prettier

Add `--compare` with exactly one baseline name:

```sh
fmtpp bench app.js --runs 20 --compare oxfmt
```

Or:

```sh
fmtpp bench app.js --runs 20 --compare prettier
```

What happens under the hood:

1. fmtpp formats the file N times with warmup, as usual.
2. The baseline tool times on the same file under the same conditions.
3. A speedup report is printed alongside the fmtpp average.

Use the comparison to answer "which tool is faster on this input,"
not "which tool formats better." Run it on files you format every day,
not on a micro-benchmark that flatters one tool.

Keep the comparison fair:

- Same file for both tools.
- Same `--runs` value for both tools.
- Same machine, same load, back-to-back runs.
- Same input content; do not edit the file between runs.

A minimal fair pair looks like:

```bash
fmtpp bench app.js --runs 20 --compare oxfmt
fmtpp bench app.js --runs 20 --compare prettier
```

Run each line a couple of times. If the winner flips between repeats,
the difference is noise on that input, not a real speedup.

::: warning
Close background builds, tests, and video calls before recording
comparison numbers. A busy machine punishes whichever tool runs second.
:::

## Tips for stable numbers

Stable numbers come from controlling the machine, not from adding
more flags. There are only three knobs: the file set, `--runs`, and
`--compare`. Everything else is procedure.

Pick representative files at realistic sizes from your own
repository:

```sh
fmtpp bench app.js --runs 100
```

Small `--runs` values iterate fast; large ones smooth out jitter. Use
`--runs 3` while exploring, `--runs 20` when comparing tools, and
`--runs 100` for numbers you plan to publish.

Isolate the machine:

- One benchmark at a time; never two in parallel.
- Quiet SSD and CPU: pause sync, indexing, and container builds.
- Repeat the exact command at least twice and keep both outputs.
- Prefer wired power on laptops; low-battery throttling skews timing.

Read the report skeptically:

- If iteration 1 is much slower than the rest, trust the later ones.
- If all iterations swing widely, the machine was busy; re-run.
- If two tools differ by a few percent, call it a tie on that input.

Keep a tiny log next to the numbers: file name, `--runs` value, and
`--compare` target when used.

## Quick reference

| Task | Command |
| --- | --- |
| Time one file, 100 runs | `fmtpp bench app.js --runs 100` |
| Compare against oxfmt | `fmtpp bench app.js --runs 20 --compare oxfmt` |
| Compare against prettier | `fmtpp bench app.js --runs 20 --compare prettier` |
| Quick smoke timing | `fmtpp bench app.js --runs 3` |

All four lines use only verified `bench` syntax: required `files`,
`--runs N`, and `--compare <tool>` with `oxfmt` or `prettier`. Check
the header (`Files`, `Iterations`, `Language`, `Warmup`) before quoting
any number from the run.
