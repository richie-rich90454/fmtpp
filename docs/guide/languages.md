# Languages

Language is detected from the file extension. Unknown extensions fall
back to JavaScript.

| Language | Extensions | Class | Reference |
|---|---|---|---|
| JavaScript | `.js` `.mjs` `.cjs` | `JSFormatter` | Prettier |
| TypeScript | `.ts` `.tsx` `.mts` | `TSFormatter` | Prettier |
| Java | `.java` | `JavaFormatter` | google-java-format |
| C | `.c` `.h` | `CFormatter` | clang-format |
| C++ | `.cpp` `.cc` `.cxx` `.hpp` `.hh` `.hxx` | `CppFormatter` | clang-format |
| C# | `.cs` | `CSharpFormatter` | `dotnet format` |
| Go | `.go` | `GoFormatter` | gofmt |
| Rust | `.rs` | `RustFormatter` | rustfmt |

## How it maps

All C-style languages share `CstyleFormatterBase` (indent, braces,
semicolons, spacing, wrapping, comments) and override per-language
pieces:

- **JS** — `function` keyword, `let`/`const`/`var`, imports/exports,
  classes, arrow functions, template literals.
- **TS** (extends JS) — return-type annotations, `interface`, `enum`,
  `type` aliases, generics.
- **Java** — `public` methods, `class … extends … implements …`,
  `import …;`, `switch` with `break`, `try`/`catch`/`finally`.
- **C** — base C-style formatting plus `#include`/`#define` passthrough.
- **C++** (extends C) — `ReturnType name(params)`, `: public Base`
  inheritance, `namespace`.
- **C#** — `public class … : Base, IFace`, `public Ret Name(…)`,
  `namespace`, `public interface … : …`.
- **Go** (standalone visitor) — `func`, `x := v`, no semicolons except
  `for` headers, `type … struct`, `type … interface`, `const (… = iota)`,
  `panic()`, `[]type{…}`, `//go:build` tags, `nil`.
- **Rust** (standalone visitor) — `fn … -> Ret`, `let [mut] …[: Ty]`,
  `use a::b;`, `match`, `panic!()`, `vec![…]`, `format!(…)`, `|…|`
  closures, `impl`/`trait`/`enum`/`type`/`mod`, `#[attrs]`, `None`.

Sample inputs for every language live in `tests/fixtures/sample.<ext>`.
