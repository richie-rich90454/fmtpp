# Languages

Language is detected from the file extension. Unknown extensions fall
back to JavaScript.

| Language | Extensions | Class | Reference |
|---|---|---|---|
| JavaScript | `.js` `.mjs` `.cjs` `.jsx` | `JSFormatter` | Prettier |
| TypeScript | `.ts` `.tsx` `.mts` `.cts` | `TSFormatter` | Prettier |
| Java | `.java` | `JavaFormatter` | google-java-format |
| C | `.c` `.h` | `CFormatter` | clang-format |
| C++ | `.cpp` `.cc` `.cxx` `.hpp` `.hh` `.hxx` `.ino` `.cu` `.cuh` | `CppFormatter` | clang-format |
| C# | `.cs` | `CSharpFormatter` | `dotnet format` |
| Go | `.go` | `GoFormatter` | gofmt |
| Rust | `.rs` | `RustFormatter` | rustfmt |
| Kotlin | `.kt` `.kts` | `KotlinFormatter` | ktfmt |
| Swift | `.swift` | `SwiftFormatter` | swift-format |
| Dart | `.dart` | `DartFormatter` | dart format |
| PHP | `.php` `.phtml` | `PhpFormatter` | php-cs-fixer |
| Scala | `.scala` `.sc` | `ScalaFormatter` | scalafmt |
| Objective-C | `.m` `.mm` | `ObjectiveCFormatter` | clang-format |
| Zig | `.zig` `.zon` | `ZigFormatter` | zig fmt |
| D | `.d` `.di` | `DFormatter` | dfmt |
| Groovy | `.groovy` `.gvy` | `GroovyFormatter` | Spotless |
| Solidity | `.sol` | `SolidityFormatter` | prettier-plugin-solidity |
| V | `.v` `.vsh` `.vv` | `VFormatter` | v fmt |
| GLSL | `.glsl` `.vert` `.frag` `.geom` `.comp` `.tesc` `.tese` `.vs` `.fs` | `GlslFormatter` | clang-format |
| HTML/XML | `.html` `.htm` `.xhtml` `.svg` `.xml` | `HtmlFormatter` | Prettier |
| CSS | `.css` `.scss` `.less` | `CssFormatter` | Prettier |
| JSON | `.json` | `JSFormatter` | Prettier |
| Vue SFC | `.vue` | `VueFormatter` | Prettier/Volar |
| Python | `.py` `.pyw` `.pyi` | `PythonFormatter` | black |
| Ruby | `.rb` | `RubyFormatter` | rubocop |

## How it maps

All C-style languages share `CstyleFormatterBase` (indent, braces,
semicolons, spacing, wrapping, comments) and override per-language
pieces:

- **JS** — `function` keyword, `let`/`const`/`var`, imports/exports,
  classes, arrow functions, template literals.
- **TS** (extends JS) — return-type annotations, `interface`, `enum`,
  `type` aliases, generics.
- **Java** — `public` methods, `class … extends … implements …`,
  `import …;`, `package …;`, verbatim `switch` cases, full
  `try`/`catch`/`finally` with catch bodies.
- **C** — `Ret name(…)` functions, `#include`/`#define` passthrough,
  `;`-terminated enums.
- **C++** (extends C) — `class`/`struct`/`union` with kinds and trailing
  `;`, `: public Base` inheritance, `namespace`.
- **C#** — `public class … : Base, IFace`, `public Ret Name(…)`,
  `namespace`, `public interface … : …`.
- **Go** (standalone visitor) — `func`, `x := v`, `var x T`, no
  semicolons except `for` headers, `type … struct`, `type … interface`,
  real `package` clauses, `panic()`, `nil`.
- **Rust** (standalone visitor) — `[pub] fn … -> Ret`, `let [mut] …[: Ty]`,
  `use a::b;`, `match`, `panic!()`, `vec![…]`, `|…|` closures,
  `struct`/`impl`/`trait`/`enum`/`type`/`mod`, `None`.
- **Kotlin** (extends Java) — `fun …: Ret`, `class … : Super(), Trait`,
  `object`, `when`, `package`, no semicolons.
- **Swift** — `func … -> Ret` (`init`/`deinit` bare), `class`/`struct`,
  paren-less `for`-in and `switch`, no semicolons.
- **Dart** (extends C) — C-style functions, `class … extends …
  implements …`, quoted imports.
- **PHP** (extends C) — `[modifiers] function`, `$` variables,
  `use`/`namespace …;`, `<?php` passthrough.
- **Scala** (extends Java) — `def …: Ret`, `class`/`object`/`trait`,
  `match`, `package`, no semicolons.
- **Objective-C** (extends C) — `@interface`/`@implementation`/`@protocol`
  with `@end`, raw method signatures, plain C passes through.
- **Zig** — `fn … Ret`, `const`/`var` with types, `const x = @import(…);`,
  `const Name = struct|enum|union`, `for` captures.
- **D** (extends C) — `class … : Bases`, dotted imports, `module …;`.
- **Groovy** (extends Java) — `def`, constructor detection, `package`,
  no semicolons.
- **Solidity** (extends JS) — `contract … is …`, `function … modifiers
  returns`, quoted imports, `pragma` passthrough.
- **V** (extends Go) — `fn`, `mut x := v`, real `enum`, `module`,
  unquoted imports, verbatim `#flag` lines.
- **GLSL** (extends C) — `struct …;`, uniforms, `#version` passthrough.
- **HTML** — elements with attributes, inline text, void elements,
  comments, doctypes.
- **CSS** (extends C-style base) — `selector { prop: value; }` rules,
  `@import`, `@media` nesting.
- **JSON** — parsed strictly, printed via the JS object/array printers.
- **Vue** (extends HTML) — `<template>` as HTML, `<script>` via JS/TS by
  `lang`, `<style>` via CSS.
- **Python** (extends `IndentFormatterBase`) — `def`/`class` with `:`
  suites (inline or indented), `if`/`elif`/`else`, `for`/`while` with
  `else`, `try`/`except`/`else`/`finally`, `with`, `match`/`case`,
  `import`/`from`, decorators, `async`.
- **Ruby** (extends `IndentFormatterBase`) — `def` (incl. endless)/
  `class`/`module` with `end`, `if`/`elsif`/`else`/`unless`,
  `case`/`when`/`in`/`else`, `begin`/`rescue`/`else`/`ensure` plus
  method-level rescue, `while`/`until`/`for`, `do` blocks, `require`,
  heredocs, `=begin` blocks, `__END__` tails.

Sample inputs for every language live in `tests/fixtures/sample.<ext>`.
