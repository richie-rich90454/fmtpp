#pragma once
#include "fmtpp/CstyleFormatterBase.hpp"

/**
 * JavaScript code formatter.
 * Extends CstyleFormatterBase with JS-specific formatting:
 * ASI (automatic semicolon insertion), arrow functions,
 * template literals, and ES6+ syntax.
 *
 * Easter egg: JavaScript - the language that runs the internet, for better or worse
 */
class JSFormatter : public CstyleFormatterBase
{
public:
    JSFormatter(OutputBuffer& out, const Config& config);
    virtual ~JSFormatter();

    // --- Override base visitor methods to delegate to Node-suffixed ---
    virtual void visitFunctionDecl(FunctionDeclNode& node);
    virtual void visitVariableDecl(VariableDeclNode& node);
    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitImportDecl(ImportDeclNode& node);
    virtual void visitExportDecl(ExportDeclNode& node);
    virtual void visitArrowFunction(ArrowFunctionNode& node);
    virtual void visitTemplateLiteral(TemplateLiteralNode& node);
    virtual void visitInterfaceDecl(InterfaceDeclNode& node);
    virtual void visitEnumDecl(EnumDeclNode& node);
    virtual void visitTypeAlias(TypeAliasNode& node);

protected:
    /** JS-specific: handle ASI (automatic semicolon insertion) */
    virtual bool requiresSemicolon(ASTNode* node) const;

    // --- Node-suffixed implementations (called by overridden base methods) ---
    virtual void visitFunctionDeclNode(FunctionDeclNode& node);
    virtual void visitVariableDeclNode(VariableDeclNode& node);
    virtual void visitClassDeclNode(ClassDeclNode& node);
    virtual void visitImportDeclNode(ImportDeclNode& node);
    virtual void visitExportDeclNode(ExportDeclNode& node);
    virtual void visitArrowFunctionNode(ArrowFunctionNode& node);
    virtual void visitTemplateLiteralNode(TemplateLiteralNode& node);
    virtual void visitInterfaceDeclNode(InterfaceDeclNode& node);
    virtual void visitEnumDeclNode(EnumDeclNode& node);
    virtual void visitTypeAliasNode(TypeAliasNode& node);
};