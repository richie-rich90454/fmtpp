#pragma once
#include "fmtpp/CstyleFormatterBase.hpp"

/**
 * Zig code formatter.
 * fn with trailing return type, const/var with : type, @import,
 * const Name = struct|enum|union containers. Semicolons required.
 *
 * Easter egg: Zig - comptime formatting, runtime beauty
 */
class ZigFormatter : public CstyleFormatterBase
{
public:
    ZigFormatter(OutputBuffer& out, const Config& config);
    virtual ~ZigFormatter();

    virtual void visitFunctionDecl(FunctionDeclNode& node);
    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitImportDecl(ImportDeclNode& node);
    virtual void visitForStmt(ForStmtNode& node);
};
