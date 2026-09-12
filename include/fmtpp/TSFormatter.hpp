#pragma once
#include "fmtpp/JSFormatter.hpp"

/**
 * TypeScript code formatter.
 * Extends JSFormatter with TypeScript-specific constructs:
 * type annotations, interfaces, enums, type aliases, generics.
 *
 * Easter egg: TypeScript - because JavaScript needed to grow up
 */
class TSFormatter : public JSFormatter
{
public:
    TSFormatter(OutputBuffer& out, const Config& config);
    virtual ~TSFormatter();

    virtual void visitFunctionDeclNode(FunctionDeclNode& node);
    virtual void visitVariableDeclNode(VariableDeclNode& node);
    virtual void visitPropertyNode(PropertyNode& node);
    virtual void visitInterfaceDeclNode(InterfaceDeclNode& node);
    virtual void visitEnumDeclNode(EnumDeclNode& node);
    virtual void visitTypeAliasNode(TypeAliasNode& node);
};