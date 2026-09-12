# Testing

## C++ suites (CTest)

35 suites, run from a configured build:

```sh
cmake -S . -B build && cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

Targeted runs (from inside `build/`):

```sh
ctest -R javaformatter --output-on-failure   # one suite only
ctest -R "formatter"                         # all 25 formatter suites
./test_goformatter.exe                       # a suite binary directly
```

Note: `ctest` without `--test-dir build` from the repo root, or
`cmake --build build` from inside `build/`, fails with "not a
directory" / "no tests found" — the paths above are the ones that work.

Every supported language has a dedicated formatter suite plus a sample
file under `tests/fixtures/`:

| Suite | Fixture |
|---|---|
| `test_jsformatter` | `sample.js` |
| `test_tsformatter` | `sample.ts` |
| `test_javaformatter` | `sample.java` |
| `test_cformatter` | `sample.c` |
| `test_cppformatter` | `sample.cpp` |
| `test_csharpformatter` | `sample.cs` |
| `test_goformatter` | `sample.go` |
| `test_rustformatter` | `sample.rs` |
| `test_pythonformatter` | `sample.py` |
| `test_rubyformatter` | `sample.rb` |

Infra suites: `test_ast`, `test_benchmark`, `test_configbuilder`,
`test_cstyle_formatter_base_core`, `test_formatter_visitor`,
`test_memorypool_astnode`, `test_outputbuffer`, `test_parser`,
`test_raii_wrappers`, `test_threadpool`.

Suites follow one pattern: build ASTs via `ASTNodeFactory` on a local
`MemoryPool` (`g_pool`), format into an `OutputBuffer`, assert on
`strstr` of the output, print `N passed, M failed`, return nonzero on
failure. To add a language test, copy `test_cformatter.cpp` and register
the name in `FMTPP_TESTS` in `CMakeLists.txt`.

## npm packaging tests

```sh
npm test   # runs tests/test_npm_packaging.js
```

Covers `package.json` fields, platform-key detection, binary-path
resolution, binary slots for all five targets, `FMTPP_BINARY` override
and the compile-from-source fallback export.

## Known flake

`test_threadpool` shares the global `MemoryPool` across worker threads
(`Parser::parse` calls `g_pool->reset()`), so it can rarely segfault
under load. It passes consistently on re-run; the proper fix is
per-thread pools (pending `ThreadPool`/`Parser` rework).
