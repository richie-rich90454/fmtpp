#pragma once
#include "fmtpp/CFormatter.hpp"

/**
 * Dart code formatter.
 * Extends CFormatter (C-style functions), adding class with
 * extends/implements and quoted imports.
 *
 * Easter egg: Dart - hit the bullseye of formatting on the first throw
 */
class DartFormatter : public CFormatter
{
public:
    DartFormatter(OutputBuffer& out, const Config& config);
    virtual ~DartFormatter();

    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitImportDecl(ImportDeclNode& node);
};
