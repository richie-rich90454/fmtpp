#pragma once
#include "fmtpp/CFormatter.hpp"

/**
 * C++ code formatter.
 * Extends CFormatter with C++-specific constructs: class/struct/union
 * with bases, namespaces and return-type functions with modifiers.
 * (Templates, using/typedef, operator overloads and lambdas are not
 * modeled yet and pass through unstructured.)
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