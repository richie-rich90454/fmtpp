#pragma once
#include "fmtpp/CFormatter.hpp"

/**
 * D code formatter.
 * Extends CFormatter (C-style functions). Classes use : bases,
 * imports are dotted, modules end with a semicolon.
 *
 * Easter egg: D - the systems language that does it all, templates included
 */
class DFormatter : public CFormatter
{
public:
    DFormatter(OutputBuffer& out, const Config& config);
    virtual ~DFormatter();

    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitInterfaceDecl(InterfaceDeclNode& node);
    virtual void visitImportDecl(ImportDeclNode& node);
    virtual void visitNamespace(NamespaceNode& node);
};
