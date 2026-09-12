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
│   ExportDeclNode, PreprocessorDirectiveNode, NamespaceNode, ...
│
ASTVisitor
├── FormatterVisitor (abstract)
│   ├── CstyleFormatterBase            <-- master class
│   │   ├── JSFormatter → TSFormatter
│   │   ├── JavaFormatter
│   │   ├── CFormatter → CppFormatter
│   │   └── CSharpFormatter
│   ├── GoFormatter
│   └── RustFormatter
├── StyleDetector   (fmtpp learn → .fmtpp.toml)
└── ASTPrinter      (debug tree dumps)
```

`CstyleFormatterBase` owns the template-method pattern: `visit*` walks
the node, `print*` pieces are virtual so subclasses override only what
differs (`visitFunctionDecl`, `visitClassDecl`, …). Go/Rust extend
`FormatterVisitor` directly (no semicolons, different blocks).

## Performance strategy

- **`MemoryPool`** — one 10 MB bump-allocated chunk; all AST nodes come
  from `ASTNodeFactory` via placement new. No per-node `new`/`delete`.
- **Zero-copy intent** — `SourceBuffer` mmaps/reads the file once and
  outlives the AST (full tree-sitter integration pending, see below).
- **`OutputBuffer`** — pre-reserved (`filesize × 1.2`) `realloc` buffer,
  `append`/`printf` via `vsnprintf`. No `std::string` concatenation.
- **`ThreadPool`** — `hardware_concurrency()` workers, one file per task,
  results collected in order via futures.

## Resource rules (binding)

- Every acquisition (`fopen`, `malloc`, `mmap`) lives in a RAII wrapper
  (`FileHandle`, `MallocBuffer`, `MmapRegion`) or a `goto`-cleanup
  function with a single exit label.
- No `shared_ptr`/`unique_ptr`, no lambdas/`std::function`, no
  `stringstream` on hot paths, no unexplained `auto`.
- JavaDoc-style comment on every class and public method.

## Current status

Formatters, config/learn/TUI, benchmark, thread pool and npm packaging
are implemented and tested. `Parser` is a fallback stub returning a
minimal AST — linking tree-sitter grammars (declared in `fmtpp.txt`
Phase 2) is the pending item before CLI file formatting matches the
unit-tested formatter output.
