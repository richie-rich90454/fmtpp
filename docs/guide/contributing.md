# Contributing

This guide shows how to add a language end to end. fmtpp is at v0.1.0
with 26 languages and 38 CTest suites. Follow the six steps below in
order; each one has a concrete done-criteria.

Targeted test runs used throughout:

```sh
ctest -R javaformatter
```

Full runs before opening a pull request:

```sh
ctest --test-dir build --output-on-failure
```

## Checklist overview

1. Parser profile row with class, interface, enum, and function words.
2. Formatter subclass on the right base with the right `visit*`
   overrides.
3. `FormatterFactory::create` plus `detectLanguage` registration.
4. `tests/fixtures/sample.<ext>` plus `tests/e2e/<lang>/` files.
5. `test_<lang>formatter.cpp` suite plus `FMTPP_TESTS` entry.
6. Docs row in `languages.md` plus a style example.

Work through them in order. Skip none.

## 1. Add the parser profile row

Each language has a parser profile: the keywords that introduce
classes, interfaces, enums, and functions. Find the existing profile
table for the language family closest to yours and add one row.

What to define:

- Words that start a class-like type.
- Words that start an interface or protocol.
- Words that start an enum.
- Words that start a named function or method.
- Any keyword handling the generic parser would get wrong
  (for example, a word that looks like a type but starts a block).

Keep the row minimal. If the language shares a family profile (C-like,
brace languages, colon suites, `end` suites), reuse that row and only
add the words that differ.

Done when: the parser recognizes a minimal class, interface, enum,
and function in the new language without misclassifying keywords.

## 2. Subclass the right formatter base

Pick one base:

| Base | Use when |
|---|---|
| `CstyleFormatterBase` | brace language in the C family (C, C++, C#, Java, Go, Rust, Kotlin, Swift, Dart, PHP, Scala, Objective-C, Zig, D, Groovy, Solidity, V, GLSL) |
| `FormatterVisitor` | markup, style, data, or component languages needing custom visits (JS, TS, HTML, CSS, JSON via `JSFormatter`, Vue) |
| `IndentFormatterBase` | indentation languages without braces (Python colon suites, Ruby `end` suites) |

Which `visit*` to override:

- C family: start from the C or C++ formatter. Override function,
  struct, and type visits only if the brace or keyword rule differs.
- Markup and style: override element, attribute, and embedded-language
  visits. Respect `proseWrap`, `htmlWhitespaceSensitivity`,
  `vueIndentScriptAndStyle`, `embeddedLanguageFormatting`, and
  `singleAttributePerLine` where they apply.
- Indentation languages: override suite and block visits. Python keeps
  colon suites, Ruby keeps `end` suites.

Reference behaviors your formatter must keep:

```cpp
int add(int a, int b) {
  return a + b;
}
```

C uses same-line braces. C++ uses Allman:

```cpp
int f()
{
  return 0;
}
```

C structs print `struct Node` followed by a newline `{`, with the
closing `}` carrying no semicolon. TypeScript methods print a newline
before the brace. JSON prints compact single-line output through
`JSFormatter`.

Two safety rules apply to every formatter:

- Unknown constructs become verbatim `RawTextNode`. Never invent
  syntax.
- Invalid input is kept verbatim. Formatters never corrupt code.

::: tip
Copy the closest existing formatter and delete what you do not need.
A small subclass that reuses the family base beats a large rewrite.
:::

Done when: the sample file in step 4 formats with the family brace,
suite, and verbatim rules above.

## 3. Register the factory and detection

Wire the new language into `FormatterFactory` in two places:

1. `FormatterFactory::create` returns your formatter for the new
   language name.
2. `FormatterFactory::detectLanguage` maps each file extension to
   that language name.

Extension mapping to follow (add your rows, change none of the rest):

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

Rules:

- NULL path, missing extension, and unknown extension fall back to
  `js`. Do not change the fallback.
- Surprising but intentional mappings stay: `.h` goes to `c`,
  `.m` / `.mm` go to `objc`, `.v` / `.vsh` / `.vv` go to `v`.

Done when: a file with the new extension detects the new language,
and an unknown extension still detects `js`.

## 4. Add fixtures and end-to-end files

Two sets of files are required.

Fixture (one file):

- `tests/fixtures/sample.<ext>`, 50 or more lines.
- Cover declarations, control flow, nesting, strings containing
  braces, comments, and one invalid snippet.

End-to-end directory `tests/e2e/<lang>/` with exactly these seven
files:

| File | Purpose |
|---|---|
| `basic` | minimal happy path |
| `control` | loops and branches |
| `stress` | deep nesting and long lines |
| `strings_braces` | braces inside strings |
| `empty` | zero-byte input stays empty |
| `comments_only` | comments pass through unchanged |
| `invalid` | broken syntax passes through verbatim |

The `test_e2e` runner checks every file for four properties:

- Output is non-empty except for the `empty` case.
- No `placeholder` leak in output.
- Double-format is idempotent.
- Expected keywords for the language are present.

```sh
ctest --test-dir build --output-on-failure
```

::: warning
Do not skip the `invalid` and `strings_braces` fixtures. They guard
the never-corrupt and verbatim rules.
:::

Done when: all seven files pass the four checks above.

## 5. Add the formatter test suite

Copy `test_cformatter.cpp` to `test_<lang>formatter.cpp`. Keep its
pattern exactly:

- Build ASTs with `ASTNodeFactory` on a local `MemoryPool` named
  `g_pool`.
- Format into an `OutputBuffer`.
- Assert with `strstr` on the output text.
- Print `N passed, M failed` and return nonzero on failure.

```cpp
// JavaDoc-style comment on every class and public method.
// MemoryPool is 10MB bump per thread; ASTNodeFactory uses placement new.
```

Then register the suite name in `FMTPP_TESTS` in `CMakeLists.txt`:

```text
FMTPP_TESTS includes test_<lang>formatter
(copy the test_cformatter.cpp entry and rename it)
```

Keep the suite focused: brace or suite style, keyword handling,
verbatim passthrough, idempotency spot check. Broad coverage belongs
in the fixtures from step 4, not in dozens of micro-asserts.

Run the new suite by name (example shape shown with the Java suite):

```sh
ctest -R javaformatter
```

Replace the suite name with yours for the real run.

Done when: the new suite passes alone and in the full run.

## 6. Document the language

Update the language list and show the style:

1. Add one row to `languages.md` with the language name, extensions,
   and formatter base.
2. Add a short style example showing brace or suite placement for the
   new language, following the existing per-language examples.

Keep the example under 20 lines. Show the rule a newcomer would get
wrong (brace placement, colon versus `end`, compact versus expanded).

Done when: a reader can tell from `languages.md` which extensions map
to the new language and what its braces look like.

## Code rules

These rules are enforced in review. They come from real constraints:
predictable memory, no hidden allocations on hot paths, readable
ownership.

- C++11 only.
- Every `fopen`, `malloc`, and `mmap` lives in an RAII wrapper
  (`FileHandle`, `MallocBuffer`, `MmapRegion`) or uses explicit
  `goto`-cleanup. No bare resource acquisition.
- No `shared_ptr` or `unique_ptr`.
- No lambdas and no `std::function`.
- No `stringstream` on hot paths.
- No unexplained `auto`. If the type is not obvious from the right
  hand side, spell it out.
- JavaDoc-style comment on every class and every public method.
- Memory management: `MemoryPool` is a 10MB bump allocator per
  thread; `ASTNodeFactory` creates nodes with placement new inside
  the pool.

Example shape for aSuite (simplified, follows `test_cformatter.cpp`):

```cpp
// Test suite for the new language formatter.
MemoryPool g_pool;
ASTNodeFactory factory(&g_pool);
OutputBuffer out;
// Build nodes, format, then strstr the output.
```

Keep new code in the existing style. Match the neighboring formatter
before introducing a new pattern.

## Build and test commands

Configure and build:

```sh
cmake -S . -B build && cmake --build build --config Release
```

Run everything:

```sh
ctest --test-dir build --output-on-failure
```

Run one suite:

```sh
ctest -R javaformatter
```

UB sanitize build:

```sh
cmake -S . -B build -DFMTPP_SANITIZE_UB=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

Known flake, not your bug: `test_threadpool` shares one global
`MemoryPool` across worker threads, so it can rarely segfault under
load. It passes on re-run. The proper fix is per-thread pools and is
pending. If only that suite fails, rerun it before debugging your
change.

Config commands you may need while testing style output:

```sh
fmtpp learn example.js --output .fmtpp.toml
fmtpp config
```

Config resolution reminder: start in the formatted file's own
directory, walk parents, first `.fmtpp.toml` found wins.
Missing or unparseable means silent defaults. See the configuration
guide for the full key list and defaults.

## Pull request checklist

Copy this list into the PR description and check each box:

- Parser profile row added for class, interface, enum, function words.
- Formatter subclassed from the correct base with minimal overrides.
- `FormatterFactory::create` and `detectLanguage` registered.
- `tests/fixtures/sample.<ext>` added with 50 or more lines.
- `tests/e2e/<lang>/` added with all seven files.
- `test_<lang>formatter.cpp` added following the
  `test_cformatter.cpp` pattern.
- `FMTPP_TESTS` entry added in `CMakeLists.txt`.
- `languages.md` row and style example added.
- Code rules followed: C++11, RAII or `goto`-cleanup, no banned
  constructs, JavaDoc comments on new classes and public methods.
- Full suite passes:

```sh
ctest --test-dir build --output-on-failure
```

- New suite passes by name, and `test_e2e` idempotency holds.
- No invented flags, keys, or commands. Anything unverified was
  omitted.

Small focused PRs merge fastest. One language per PR, no drive-by
refactors.
