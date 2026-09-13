#pragma once
#include "fmtpp/CstyleFormatterBase.hpp"

/**
 * C# code formatter.
 * Extends CstyleFormatterBase with C#-specific formatting:
 * public classes/structs with bases, methods, namespaces and interfaces.
 * (Properties, events, LINQ and async are not modeled yet and pass
 * through unstructured.)
 *
 * Easter egg: C# - Microsoft's answer to Java, with extra sharpness
 */
class CSharpFormatter : public CstyleFormatterBase
{
public:
    CSharpFormatter(OutputBuffer& out, const Config& config);
    virtual ~CSharpFormatter();

    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitFunctionDecl(FunctionDeclNode& node);
    virtual void visitNamespace(NamespaceNode& node);
    virtual void visitInterfaceDecl(InterfaceDeclNode& node);

protected:
    virtual bool defaultFunctionBraceSameLine() const;
    virtual bool defaultTypeBraceSameLine() const;
};