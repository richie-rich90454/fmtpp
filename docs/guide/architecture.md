# Architecture

One pipeline, one parser engine, one visitor family per language.

```mermaid
flowchart LR
    A["SourceBuffer<br/>reads file once"] --> B["Parser<br/>builds AST"]
    B --> C["FormatterFactory<br/>picks language"]
    C --> D["FormatterVisitor<br/>prints nodes"]
    D --> E["OutputBuffer<br/>to stdout"]
```

The ASCII version of the same path:

```
SourceBuffer → Parser → AST → FormatterVisitor → OutputBuffer → stdout
                             (one per language)     ↑
ThreadPool fans multi-file runs out; each job owns its buffer+parser.
```

## How a file flows through

```mermaid
flowchart TD
    S["CLI receives<br/>file path"] --> D["Detect language<br/>from extension"]
    D --> R["SourceBuffer<br/>loads bytes"]
    R --> P["Parser scans<br/>language profile"]
    P --> Q{"Recognized<br/>construct"}
    Q -->|"Yes"| N["Structured<br/>AST node"]
    Q -->|"No"| W["RawTextNode<br/>verbatim"]
    N --> F["Formatter<br/>prints node"]
    W --> F
    F --> O["OutputBuffer<br/>accumulates text"]
    O --> Z["stdout or<br/>status report"]
```

## Class hierarchy

```mermaid
flowchart TD
    V["ASTVisitor"] --> FV["FormatterVisitor"]
    FV --> CB["CstyleFormatterBase"]
    FV --> GO["GoFormatter"]
    FV --> RU["RustFormatter"]
    FV --> IB["IndentFormatterBase"]
    GO --> VF["VFormatter"]
    IB --> PY["PythonFormatter"]
    IB --> RB["RubyFormatter"]
```

```mermaid
flowchart LR
    CB["CstyleFormatterBase"] --> JS["JSFormatter"]
    CB --> JV["JavaFormatter"]
    CB --> CF["CFormatter"]
    CB --> CS["CSharpFormatter"]
    CB --> SW["SwiftFormatter"]
    CB --> ZG["ZigFormatter"]
    CB --> HT["HtmlFormatter"]
    CB --> CSS["CssFormatter"]
    HT --> VU["VueFormatter"]
```

```mermaid
flowchart TD
    JS["JSFormatter"] --> TS["TSFormatter"]
    JS --> SO["SolidityFormatter"]
```

```mermaid
flowchart TD
    JV["JavaFormatter"] --> KO["KotlinFormatter"]
    JV --> SC["ScalaFormatter"]
    JV --> GR["GroovyFormatter"]
```

```mermaid
flowchart LR
    CF["CFormatter"] --> CP["CppFormatter"]
    CF --> DA["DartFormatter"]
    CF --> DF["DFormatter"]
    CF --> GL["GlslFormatter"]
    CF --> OC["ObjectiveCFormatter"]
    CF --> PH["PhpFormatter"]
```

The same tree as text:

```
ASTNode (abstract)
├── ProgramNode, FunctionDeclNode, VariableDeclNode, IfStmtNode,
│   ForStmtNode, WhileStmtNode, SwitchStmtNode, BlockNode,
│   ReturnStmtNode, ThrowStmtNode, TryCatchStmtNode, CommentNode,
│   LiteralNode, IdentifierNode, ArrayLiteralNode, ObjectLiteralNode,
│   TemplateLiteralNode, ArrowFunctionNode, ClassDeclNode,
│   InterfaceDeclNode, EnumDeclNode, TypeAliasNode, ImportDeclNode,
│   ExportDeclNode, PreprocessorDirectiveNode, NamespaceNode,
│   PropertyNode, RawTextNode, HtmlElementNode, WithNode,
│   SwitchCaseNode
│
ASTVisitor
├── FormatterVisitor (abstract)
│   ├── CstyleFormatterBase            <-- master class
│   │   ├── JSFormatter → TSFormatter, SolidityFormatter
│   │   ├── JavaFormatter → KotlinFormatter, ScalaFormatter, GroovyFormatter
│   │   ├── CFormatter → CppFormatter, DartFormatter, DFormatter,
│   │   │                GlslFormatter, ObjectiveCFormatter, PhpFormatter
│   │   ├── CSharpFormatter
│   │   ├── SwiftFormatter, ZigFormatter
│   │   └── HtmlFormatter → VueFormatter
│   │   └── CssFormatter
│   ├── GoFormatter → VFormatter
│   ├── RustFormatter
│   └── IndentFormatterBase          <-- megaclass (no braces)
│       ├── PythonFormatter (colon suites, pass, match/case)
│       └── RubyFormatter (end suites, do blocks, heredocs)
├── StyleDetector   (fmtpp learn → .fmtpp.toml)
└── ASTPrinter      (debug tree dumps)
```

`CstyleFormatterBase` owns the template-method pattern: `visit*` walks
the node, `print*` pieces are virtual so subclasses override only what
differs (`visitFunctionDecl`, `visitClassDecl`, …). Go/Rust extend
`FormatterVisitor` directly (no semicolons, different blocks).
`FormatterFactory` is the single home for extension→language mapping
and formatter construction; CLI, thread pool and benchmark all use it.

```mermaid
classDiagram
    class ASTVisitor {
        +visitProgram()
        +visitFunctionDecl()
        +visitClassDecl()
    }
    class FormatterVisitor {
        +format()
        +printToken()
        +printSpace()
        +printNewline()
    }
    class CstyleFormatterBase {
        +visitFunctionDecl()
        +visitClassDecl()
        +visitIfStmt()
    }
    class FormatterFactory {
        +create()
    }
    ASTVisitor <|-- FormatterVisitor
    FormatterVisitor <|-- CstyleFormatterBase
    FormatterFactory ..> FormatterVisitor : creates
```

## The parser

One shared engine with per-language profiles: comment/string/regex
aware scanning, depth tracking, per-language keywords for declarations,
imports, control flow and attributes. Leaf expressions stay verbatim
and anything unrecognized becomes `RawTextNode`, so output is never
invented. HTML/XML parse as tag trees, JSON with a strict mini-parser,
Vue SFCs as HTML with per-block sub-formatting. Python parses with an
indentation engine (logical lines, suite holders for if/try/loop
chains); Ruby with an end-delimited segment engine (holders for
if/case/begin/method chains, heredoc and `=begin` awareness).

```mermaid
flowchart TD
    subgraph PROFILES ["Per-language profile"]
        K["keywords for<br/>class and interface"]
        I["import words"]
        F["function syntax"]
        X["switch and<br/>namespace words"]
    end
    S["Scanner skips<br/>strings and comments"] --> T["Depth tracker<br/>follows braces"]
    T --> M["Keyword matcher<br/>tries declaration"]
    PROFILES --> M
    M -->|"Match"| B["Build<br/>structured node"]
    M -->|"No match"| R["RawTextNode keeps<br/>source verbatim"]
```

## Parser profile catalog

Each row is one `LangProfile` in `src/Parser.cpp`: whether statements
end in semicolons, whether `#` lines are directives, and which keywords
open classes, interfaces, enums, and functions.

| Lang | `;` | `#dir` | Classes | Interfaces | Enums | Functions |
|---|---|---|---|---|---|---|
| js | yes | no | `class` | `interface` | `enum` | `function` (bare) |
| ts | yes | no | `class` | `interface` | `enum` | `function` (colon) |
| java | yes | no | `class` | `interface` | `enum` | prefix style |
| c | yes | yes | — | — | `enum` | prefix style |
| cpp | yes | yes | `class struct union` | — | `enum` | prefix style |
| cs | yes | yes | `class struct record` | `interface` | `enum` | prefix style |
| go | no | no | — | `interface` | — | `func` (trailing) |
| rs | yes | no | `struct union impl` | `trait` | `enum` | `fn` (arrow) |
| kotlin | no | no | `class object` | `interface` | `enum` | `fun` (colon) |
| swift | no | no | `class struct actor` | — | `enum` | `func` (arrow) |
| dart | yes | no | `class` | `interface` | `enum` | prefix style |
| php | yes | no | `class interface trait enum` | — | — | `function` (colon) |
| scala | no | no | `class object trait` | — | `enum` | `def` (colon) |
| objc | yes | yes | `@interface @implementation` | `@protocol` | `enum` | raw style |
| zig | yes | no | — | — | — | `fn` (trailing) |
| d | yes | yes | `class struct union` | `interface` | `enum` | prefix style |
| groovy | no | no | `class` | `interface` | `enum` | `def` (colon) |
| solidity | yes | no | `contract interface library` | — | — | `function` (bare) |
| v | no | yes | `struct` | `interface` | `enum` | `fn` (trailing) |
| glsl | yes | yes | `struct` | — | — | prefix style |
| python | no | no | `class` | — | — | `def` (colon suites) |
| ruby | no | no | `class` | — | — | `def` (bare, `end`) |

`—` means the profile declares no keyword: the construct either does
not exist in the language or parses through the generic path. Function
syntax tags (`FS_BARE`, `FS_COLON`, `FS_CPREFIX`, `FS_TRAIL`,
`FS_ARROW`, `FS_RAW`) select how headers are split into name, params,
and return type.

## Single-file run

```mermaid
sequenceDiagram
    participant CLI
    participant Factory as FormatterFactory
    participant Parser
    participant Formatter
    participant Output as OutputBuffer
    CLI->>Parser: parse source with language
    Parser-->>CLI: program node
    CLI->>Factory: create formatter for language
    Factory-->>CLI: formatter visitor
    CLI->>Formatter: format program node
    Formatter->>Output: print tokens
    Output-->>CLI: formatted text
```

## Multi-file parallel run

```mermaid
sequenceDiagram
    participant CLI
    participant Pool as ThreadPool
    participant WorkerA as Worker A
    participant WorkerB as Worker B
    CLI->>Pool: submit one task per file
    Pool->>WorkerA: format file one
    Pool->>WorkerB: format file two
    WorkerA-->>Pool: formatted result one
    WorkerB-->>Pool: formatted result two
    Pool-->>CLI: ordered status report
```

## Python indentation engine

```mermaid
flowchart TD
    L["Read logical line"] --> I["Measure indent"]
    I --> D{"Indent vs<br/>stack top"}
    D -->|"Deeper"| P["Push suite<br/>and indent"]
    D -->|"Same"| S["Append to<br/>current suite"]
    D -->|"Shallower"| O["Pop suites to<br/>matching level"]
    P --> H["Attach holder for<br/>if try loop match"]
    S --> H
    O --> H
    H --> N{"More lines"}
    N -->|"Yes"| L
    N -->|"No"| E["Close open suites"]
```

## Ruby end-delimited engine

```mermaid
flowchart TD
    A["Scan segment"] --> B{"Opens block"}
    B -->|"def class module<br/>if case begin do"| C["Push holder"]
    B -->|"Else"| K["Keep as<br/>statement"]
    C --> H["Track heredoc and<br/>embedded docs"]
    K --> H
    H --> E{"end keyword"}
    E -->|"Yes"| P["Pop holder<br/>and close node"]
    E -->|"No"| M{"More input"}
    M -->|"Yes"| A
    M -->|"No"| Z["Emit method-level<br/>rescue if present"]
    P --> M
```

## Performance strategy

- **`MemoryPool`** — one 10 MB bump-allocated chunk per thread
  (`thread_local`); all AST nodes come from `ASTNodeFactory` via
  placement new. No per-node `new`/`delete`.
- **Zero-copy intent** — `SourceBuffer` mmaps/reads the file once and
  outlives the AST.
- **`OutputBuffer`** — pre-reserved (`filesize × 1.2`) `realloc` buffer,
  `append`/`printf` via `vsnprintf`. No `std::string` concatenation.
- **`ThreadPool`** — `hardware_concurrency()` workers, one file per task,
  results collected in order via futures. Language is detected per
  file, so mixed-extension runs format each file correctly.

```mermaid
flowchart LR
    subgraph POOL ["MemoryPool per thread"]
        B["Bump pointer"] --> N["Placement new<br/>AST nodes"]
        N --> R["Reset<br/>after file"]
    end
    subgraph IO ["Single-pass IO"]
        S["SourceBuffer<br/>mmaps file"] --> O["OutputBuffer<br/>presized"]
    end
    POOL --> F["Formatter walks<br/>tree once"]
    IO --> F
```

## Configuration flow

```mermaid
flowchart TD
    A["fmtpp learn<br/>reads example"] --> B["StyleDetector infers<br/>indent and braces"]
    B --> C["Write fmtpp<br/>toml file"]
    C --> D["format finds toml<br/>above target file"]
    D --> E["Merge file config<br/>over defaults"]
    E --> F["Formatter<br/>reads flags"]
```

## Testing pyramid

```mermaid
flowchart TD
    U["Unit suites<br/>per formatter"] --> G["Parser<br/>round-trip suite"]
    G --> E["Real-file<br/>fixture suite"]
    E --> C["CLI phase via<br/>built binary"]
    C --> V["38 of 38 green"]
```

## Anatomy of a formatter

Every formatter is a visitor plus a set of overrides. The base owns
the boring parts; the subclass only teaches the differences:

```mermaid
flowchart TD
    A["Node arrives<br/>at visit method"] --> B{"Subclass<br/>overrides it"}
    B -->|"Yes"| C["Subclass prints<br/>language shape"]
    B -->|"No"| D["Base prints<br/>generic shape"]
    C --> E["Shared print helpers<br/>indent, space, brace"]
    D --> E
```

Concretely, adding support for one new construct means:

1. **Parse it** — extend the shared engine or a language branch in
   `src/Parser.cpp` so the construct becomes a node (or a `RawTextNode`
   if it should pass through verbatim).
2. **Print it** — override the matching `visit*` in the language
   formatter (e.g. `visitFunctionDecl` for brace style,
   `visitClassDecl` for `struct` kinds, `visitEnumDecl` for modifiers
   riding in `m_kind`).
3. **Cover it** — add a round-trip case to `tests/test_parser.cpp`,
   unit blocks to the language suite, and real files under
   `tests/e2e/<lang>/`.

Recent examples of this exact loop: `const enum` in TS (kind printed
verbatim), `public enum` / `private interface` modifiers, and the
C-versus-C++ function brace split (`printSpace` vs
`printNewline` + `printIndent` before the body).

## Resource rules (binding)

- Every acquisition (`fopen`, `malloc`, `mmap`) lives in a RAII wrapper
  (`FileHandle`, `MallocBuffer`, `MmapRegion`) or a `goto`-cleanup
  function with a single exit label.
- No `shared_ptr`/`unique_ptr`, no lambdas/`std::function`, no
  `stringstream` on hot paths, no unexplained `auto`.
- JavaDoc-style comment on every class and public method.

## Current status

A real parser for all 26 languages, all formatters, config load/merge,
`learn` with source-text detection, TUI `config`, `bench`
(+`--compare`), thread-pool multi-file runs and npm packaging with five
prebuilt targets — implemented and tested (38 CTest suites plus npm
packaging tests).
