#pragma once
#include "fmtpp/GoFormatter.hpp"

/**
 * V language formatter.
 * Extends GoFormatter (shared :=, fn, struct shapes). Overrides enum
 * (real enums, not iota), namespace (module), import (unquoted) and
 * preprocessor (verbatim #flag lines), plus mut bindings.
 *
 * Easter egg: V - simple, fast, and formatted in all ways
 */
class VFormatter : public GoFormatter
{
public:
    VFormatter(OutputBuffer& out, const Config& config);
    virtual ~VFormatter();

    virtual void visitVariableDeclNode(VariableDeclNode& node);
    virtual void visitFunctionDeclNode(FunctionDeclNode& node);
    virtual void visitClassDeclNode(ClassDeclNode& node);
    virtual void visitEnumDeclNode(EnumDeclNode& node);
    virtual void visitNamespaceNode(NamespaceNode& node);
    virtual void visitImportDeclNode(ImportDeclNode& node);
    virtual void visitPreprocessorDirectiveNode(PreprocessorDirectiveNode& node);
    virtual void visitLiteralNode(LiteralNode& node);

protected:
    virtual const char* switchKeyword() const;
};
