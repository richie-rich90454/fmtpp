#pragma once
#include "fmtpp/CstyleFormatterBase.hpp"

/**
 * C# code formatter.
 * Extends CstyleFormatterBase with C#-specific formatting:
 * namespace declarations, class/struct/interface with modifiers,
 * properties with get/set, using statements, async/await.
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
};