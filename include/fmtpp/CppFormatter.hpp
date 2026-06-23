#pragma once
#include "fmtpp/CFormatter.hpp"

/**
 * C++ code formatter.
 * Extends CFormatter with C++-specific constructs:
 * classes with public/private/protected, namespaces,
 * templates, operator overloading, using/typedef.
 *
 * Easter egg: C++ - because sometimes you need more pluses
 */
class CppFormatter : public CFormatter
{
public:
    CppFormatter(OutputBuffer& out, const Config& config);
    virtual ~CppFormatter();

    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitNamespace(NamespaceNode& node);
    virtual void visitFunctionDecl(FunctionDeclNode& node);
};