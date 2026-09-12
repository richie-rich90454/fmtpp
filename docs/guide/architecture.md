# Architecture

```
SourceBuffer → Parser → AST → FormatterVisitor → OutputBuffer → stdout
                             (one per language)     ↑
ThreadPool fans multi-file runs out; each job owns its buffer+parser.
```

## Class hierarchy

```
ASTNode (abstract)
├── ProgramNode, FunctionDeclNode, VariableDeclNode, IfStmtNode,
│   ForStmtNode, WhileStmtNode, SwitchStmtNode, BlockNode,
│   ReturnStmtNode, ThrowStmtNode, TryCatchStmtNode, CommentNode,
│   LiteralNode, IdentifierNode, ArrayLiteralNode, ObjectLiteralNode,
│   TemplateLiteralNode, ArrowFunctionNode, ClassDeclNode,
│   InterfaceDeclNode, EnumDeclNode, TypeAliasNode, ImportDeclNode,
│   ExportDeclNode, PreprocessorDirectiveNode, NamespaceNode,
│   PropertyNode, RawTextNode, HtmlElementNode
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
│   └── RustFormatter
├── StyleDetector   (fmtpp learn → .fmtpp.toml)
└── ASTPrinter      (debug tree dumps)
```

`CstyleFormatterBase` owns the template-method pattern: `visit*` walks
the node, `print*` pieces are virtual so subclasses override only what
differs (`visitFunctionDecl`, `visitClassDecl`, …). Go/Rust extend
`FormatterVisitor` directly (no semicolons, different blocks).
`FormatterFactory` is the single home for extension→language mapping
and formatter construction; CLI, thread pool and benchmark all use it.

## The parser

One shared engine with per-language profiles: comment/string/regex
aware scanning, depth tracking, per-language keywords for declarations,
imports, control flow and attributes. Leaf expressions stay verbatim
and anything unrecognized becomes `RawTextNode`, so output is never
invented. HTML/XML parse as tag trees, JSON with a strict mini-parser,
Vue SFCs as HTML with per-block sub-formatting.

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

## Resource rules (binding)

- Every acquisition (`fopen`, `malloc`, `mmap`) lives in a RAII wrapper
  (`FileHandle`, `MallocBuffer`, `MmapRegion`) or a `goto`-cleanup
  function with a single exit label.
- No `shared_ptr`/`unique_ptr`, no lambdas/`std::function`, no
  `stringstream` on hot paths, no unexplained `auto`.
- JavaDoc-style comment on every class and public method.

## Current status

A real parser for all 25 languages, all formatters, config load/merge,
`learn` with source-text detection, TUI `config`, `bench`
(+`--compare`), thread-pool multi-file runs and npm packaging with five
prebuilt targets — implemented and tested (33 CTest suites plus npm
packaging tests).
