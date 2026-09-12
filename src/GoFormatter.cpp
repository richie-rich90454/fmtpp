#include "fmtpp/GoFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/ThrowStmtNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/ArrayLiteralNode.hpp"
#include "fmtpp/ast/ObjectLiteralNode.hpp"
#include "fmtpp/ast/TemplateLiteralNode.hpp"
#include "fmtpp/ast/ArrowFunctionNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/TypeAliasNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ExportDeclNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/PropertyNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"
#include "fmtpp/ast/HtmlElementNode.hpp"
#include <cstdio>

// Easter egg: Go formatter born - simplicity is our middle name

GoFormatter::GoFormatter(OutputBuffer& out, const Config& config)
    : FormatterVisitor(out, config)
{
}

GoFormatter::~GoFormatter()
{
}

void GoFormatter::format(ASTNode* root)
{
    if (root == NULL) { return; }
    root->accept(*this);
    if (m_out.getSize() > 0) { printNewline(); }
}

// --- Base class overrides (delegate to Node-suffixed methods) ---

void GoFormatter::visitProgram(ProgramNode& node)          { visitProgramNode(node); }
void GoFormatter::visitFunctionDecl(FunctionDeclNode& node) { visitFunctionDeclNode(node); }
void GoFormatter::visitVariableDecl(VariableDeclNode& node) { visitVariableDeclNode(node); }
void GoFormatter::visitIfStmt(IfStmtNode& node)             { visitIfStmtNode(node); }
void GoFormatter::visitForStmt(ForStmtNode& node)           { visitForStmtNode(node); }
void GoFormatter::visitWhileStmt(WhileStmtNode& node)       { visitWhileStmtNode(node); }
void GoFormatter::visitSwitchStmt(SwitchStmtNode& node)     { visitSwitchStmtNode(node); }
void GoFormatter::visitBlock(BlockNode& node)               { visitBlockNode(node); }
void GoFormatter::visitReturnStmt(ReturnStmtNode& node)     { visitReturnStmtNode(node); }
void GoFormatter::visitThrowStmt(ThrowStmtNode& node)       { visitThrowStmtNode(node); }
void GoFormatter::visitTryCatchStmt(TryCatchStmtNode& node) { visitTryCatchStmtNode(node); }
void GoFormatter::visitComment(CommentNode& node)           { visitCommentNode(node); }
void GoFormatter::visitLiteral(LiteralNode& node)           { visitLiteralNode(node); }
void GoFormatter::visitIdentifier(IdentifierNode& node)     { visitIdentifierNode(node); }
void GoFormatter::visitArrayLiteral(ArrayLiteralNode& node) { visitArrayLiteralNode(node); }
void GoFormatter::visitObjectLiteral(ObjectLiteralNode& node) { visitObjectLiteralNode(node); }
void GoFormatter::visitTemplateLiteral(TemplateLiteralNode& node) { visitTemplateLiteralNode(node); }
void GoFormatter::visitArrowFunction(ArrowFunctionNode& node) { visitArrowFunctionNode(node); }
void GoFormatter::visitClassDecl(ClassDeclNode& node)       { visitClassDeclNode(node); }
void GoFormatter::visitInterfaceDecl(InterfaceDeclNode& node) { visitInterfaceDeclNode(node); }
void GoFormatter::visitEnumDecl(EnumDeclNode& node)         { visitEnumDeclNode(node); }
void GoFormatter::visitTypeAlias(TypeAliasNode& node)       { visitTypeAliasNode(node); }
void GoFormatter::visitImportDecl(ImportDeclNode& node)     { visitImportDeclNode(node); }
void GoFormatter::visitExportDecl(ExportDeclNode& node)     { visitExportDeclNode(node); }
void GoFormatter::visitPreprocessorDirective(PreprocessorDirectiveNode& node) { visitPreprocessorDirectiveNode(node); }
void GoFormatter::visitNamespace(NamespaceNode& node)       { visitNamespaceNode(node); }
void GoFormatter::visitProperty(PropertyNode& node)         { visitPropertyNode(node); }
void GoFormatter::visitRawText(RawTextNode& node)             { visitRawTextNode(node); }
void GoFormatter::visitHtmlElement(HtmlElementNode& node)     { visitHtmlElementNode(node); }
// --- Helpers ---

void GoFormatter::visitNode(ASTNode* node)
{
    if (node != NULL) { node->accept(*this); }
}

void GoFormatter::printGoOpeningBrace()
{
    printSpace();
    printToken("{");
}

void GoFormatter::printGoClosingBrace()
{
    printToken("}");
}

void GoFormatter::printGoSemicolon(bool inForLoop)
{
    if (inForLoop)
    {
        m_out.appendChar(';');
        m_currentColumn++;
    }
    // Go omits semicolons everywhere else
}

// ================================================================
// Go-specific visitor implementations
// ================================================================

void GoFormatter::visitProgramNode(ProgramNode& node)
{
    size_t count = node.m_statements.size();
    for (size_t i = 0; i < count; i++)
    {
        if (node.m_statements[i] != NULL) { node.m_statements[i]->accept(*this); }
        if (i < count - 1) { printNewline(); }
    }
}

void GoFormatter::visitFunctionDeclNode(FunctionDeclNode& node)
{
    // Go uses "func" keyword
    printToken("func");
    printSpace();
    printToken(node.m_name.c_str());
    printToken("(");
    size_t pc = node.m_params.size();
    for (size_t i = 0; i < pc; i++)
    {
        if (node.m_params[i] != NULL) { node.m_params[i]->accept(*this); }
        if (i < pc - 1) { printComma(); }
    }
    printToken(")");
    // Go return type comes after params
    if (!node.m_returnType.empty()) { printSpace(); printToken(node.m_returnType.c_str()); }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void GoFormatter::visitVariableDeclNode(VariableDeclNode& node)
{
    // Go short variable declaration: x := 42
    // Or typed: var x int = 42, const Pi = 3.14
    // Unshaped leftovers (struct fields as raw text) print verbatim.
    if (node.m_kind.empty() && node.m_initializer == NULL)
    {
        printToken(node.m_name.c_str());
        printNewline();
        return;
    }
    if (node.m_kind == "let" || node.m_kind == "var")
    {
        if (node.m_initializer != NULL)
        {
            // Short variable declaration with :=
            printToken(node.m_name.c_str());
            printToken(" := ");
            node.m_initializer->accept(*this);
        }
        else
        {
            // ponytail: `var x T` has no value, so := would be invalid
            printToken("var");
            printSpace();
            printToken(node.m_name.c_str());
            if (!node.m_type.empty()) { printSpace(); printToken(node.m_type.c_str()); }
        }
    }
    else if (node.m_kind == "const")
    {
        printToken("const");
        printSpace();
        printToken(node.m_name.c_str());
        if (!node.m_type.empty()) { printSpace(); printToken(node.m_type.c_str()); }
        if (node.m_initializer != NULL) { printToken(" = "); node.m_initializer->accept(*this); }
    }
    else
    {
        printToken(node.m_name.c_str());
        printToken(" := ");
        if (node.m_initializer != NULL) { node.m_initializer->accept(*this); }
    }
    printNewline();
}

void GoFormatter::visitImportDeclNode(ImportDeclNode& node)
{
    // Go uses "import" with grouped package imports
    printToken("import");
    if (node.m_source.empty())
    {
        // No source - just print "import"
        printNewline();
        return;
    }
    printSpace();
    m_out.printf("\"%s\"", node.m_source.c_str());
    m_currentColumn += static_cast<int>(node.m_source.size()) + 2;
    printNewline();
}

void GoFormatter::visitBlockNode(BlockNode& node)
{
    printToken("{");
    printNewline();
    indent();
    size_t sc = node.m_statements.size();
    for (size_t i = 0; i < sc; i++)
    {
        printIndent();
        if (node.m_statements[i] != NULL) { node.m_statements[i]->accept(*this); }
        if (i < sc - 1) { printNewline(); }
    }
    dedent();
    printNewline();
    printIndent();
    printToken("}");
}

void GoFormatter::visitReturnStmtNode(ReturnStmtNode& node)
{
    printToken("return");
    if (node.m_argument != NULL) { printSpace(); node.m_argument->accept(*this); }
    // Go: no semicolon
    printNewline();
}

void GoFormatter::visitIfStmtNode(IfStmtNode& node)
{
    printToken("if");
    printSpace();
    if (node.m_condition != NULL) { node.m_condition->accept(*this); }
    if (node.m_thenBranch != NULL) { printSpace(); node.m_thenBranch->accept(*this); }
    if (node.m_elseBranch != NULL)
    {
        printSpace();
        printToken("else");
        printSpace();
        node.m_elseBranch->accept(*this);
    }
}

void GoFormatter::visitForStmtNode(ForStmtNode& node)
{
    printToken(node.m_keyword.empty() ? "for" : node.m_keyword.c_str());
    printSpace();
    // Go for loop: for init; condition; update { }
    // Semicolons are used in for loop headers in Go
    if (node.m_init != NULL || node.m_update != NULL)
    {
        if (node.m_init != NULL) { node.m_init->accept(*this); }
        printGoSemicolon(true);
        printSpace();
        if (node.m_condition != NULL) { node.m_condition->accept(*this); }
        printGoSemicolon(true);
        printSpace();
        if (node.m_update != NULL) { node.m_update->accept(*this); }
    }
    else if (node.m_condition != NULL)
    {
        // Go for-range or simple for: for condition { }
        node.m_condition->accept(*this);
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void GoFormatter::visitWhileStmtNode(WhileStmtNode& node)
{
    // Go doesn't have while; it uses "for condition { }"
    printToken("for");
    printSpace();
    if (node.m_condition != NULL) { node.m_condition->accept(*this); }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void GoFormatter::visitSwitchStmtNode(SwitchStmtNode& node)
{
    printToken("switch");
    printSpace();
    if (node.m_discriminant != NULL) { node.m_discriminant->accept(*this); }
    printGoOpeningBrace();
    printNewline();
    indent();
    size_t cc = node.m_cases.size();
    for (size_t i = 0; i < cc; i++)
    {
        printIndent();
        if (node.m_cases[i] != NULL) { node.m_cases[i]->accept(*this); }
        printNewline();
    }
    dedent();
    printIndent();
    printGoClosingBrace();
    printNewline();
}

void GoFormatter::visitTryCatchStmtNode(TryCatchStmtNode& node)
{
    // Go uses defer/recover instead of try/catch, but we map try to a function
    // ponytail: extras (further catches) carry no try body, so no header
    if (node.m_body != NULL)
    {
        printToken("// try");
        printNewline();
    }
    if (node.m_body != NULL)
    {
        printIndent();
        // Go idiom: use func() and defer recover()
        printToken("func()");
        printSpace();
        node.m_body->accept(*this);
        printToken("()");
        printNewline();
    }
    if (node.m_catchClause != NULL)
    {
        printIndent();
        printToken("// catch: ");
        node.m_catchClause->accept(*this);
        printNewline();
    }
    if (node.m_catchBody != NULL)
    {
        printIndent();
        node.m_catchBody->accept(*this);
        printNewline();
    }
    for (size_t eci = 0; eci < node.m_extraCatches.size(); eci++)
    {
        if (node.m_extraCatches[eci] == NULL) { continue; }
        node.m_extraCatches[eci]->accept(*this);
    }
    if (node.m_finallyBlock != NULL)
    {
        printIndent();
        printToken("defer");
        printSpace();
        node.m_finallyBlock->accept(*this);
        printNewline();
    }
}

void GoFormatter::visitCommentNode(CommentNode& node)
{
    if (node.m_isBlock) { printToken("/*"); printToken(node.m_text.c_str()); printToken("*/"); }
    else { printToken("//"); printToken(node.m_text.c_str()); }
    printNewline();
}

void GoFormatter::visitLiteralNode(LiteralNode& node)
{
    switch (node.m_type)
    {
    case LiteralNode::TYPE_STRING:
        m_out.printf("\"%s\"", node.m_value.c_str());
        m_currentColumn += static_cast<int>(node.m_value.size()) + 2;
        break;
    case LiteralNode::TYPE_BOOLEAN:
    case LiteralNode::TYPE_NUMBER:
    case LiteralNode::TYPE_NULL:
        // Go uses "nil" instead of "null"
        if (node.m_type == LiteralNode::TYPE_NULL) { printToken("nil"); }
        else { printToken(node.m_value.c_str()); }
        break;
    case LiteralNode::TYPE_REGEX:
    default: printToken(node.m_value.c_str()); break;
    }
}

void GoFormatter::visitIdentifierNode(IdentifierNode& node)
{
    printToken(node.m_name.c_str());
}

void GoFormatter::visitArrayLiteralNode(ArrayLiteralNode& node)
{
    printToken("[]");
    // Go type annotation: []int{1, 2, 3}
    printToken("{");
    size_t ec = node.m_elements.size();
    for (size_t i = 0; i < ec; i++)
    {
        if (node.m_elements[i] != NULL) { node.m_elements[i]->accept(*this); }
        if (i < ec - 1) { printComma(); }
    }
    printToken("}");
}

void GoFormatter::visitObjectLiteralNode(ObjectLiteralNode& node)
{
    // Go struct literal: StructName{Key: value}
    printToken("{");
    if (m_config.bracketSpacing && node.m_properties.size() > 0) { printSpace(); }
    size_t pc = node.m_properties.size();
    for (size_t i = 0; i < pc; i++)
    {
        if (node.m_properties[i] != NULL) { node.m_properties[i]->accept(*this); }
        if (i < pc - 1) { printComma(); }
    }
    if (m_config.bracketSpacing && node.m_properties.size() > 0) { printSpace(); }
    printToken("}");
}

void GoFormatter::visitTemplateLiteralNode(TemplateLiteralNode& node)
{
    // Go uses fmt.Sprintf or string concatenation; map to backtick raw string
    printToken("`");
    size_t qc = node.m_quasis.size();
    for (size_t i = 0; i < qc; i++)
    {
        if (node.m_quasis[i] != NULL) { node.m_quasis[i]->accept(*this); }
        if (i < node.m_expressions.size() && node.m_expressions[i] != NULL)
        { printToken("${"); node.m_expressions[i]->accept(*this); printToken("}"); }
    }
    printToken("`");
}

void GoFormatter::visitArrowFunctionNode(ArrowFunctionNode& node)
{
    // Go uses anonymous functions: func(params) returnType { body }
    printToken("func(");
    size_t pc = node.m_params.size();
    for (size_t i = 0; i < pc; i++)
    {
        if (node.m_params[i] != NULL) { node.m_params[i]->accept(*this); }
        if (i < pc - 1) { printComma(); }
    }
    printToken(")");
    if (!node.m_returnType.empty()) { printSpace(); printToken(node.m_returnType.c_str()); }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
}

void GoFormatter::visitClassDeclNode(ClassDeclNode& node)
{
    // Go uses struct instead of class
    printToken("type");
    printSpace();
    printToken(node.m_name.c_str());
    printSpace();
    printToken("struct");
    if (node.m_superClass != NULL)
    {
        // Go uses embedding, not inheritance
        printNewline();
        printIndent();
        node.m_superClass->accept(*this);
    }
    if (node.m_implements.size() > 0)
    {
        // Go interfaces are implicit, but we note them as comments
        printNewline();
        printIndent();
        printToken("// implements: ");
        size_t ic = node.m_implements.size();
        for (size_t i = 0; i < ic; i++)
        {
            if (node.m_implements[i] != NULL) { node.m_implements[i]->accept(*this); }
            if (i < ic - 1) { printComma(); }
        }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
}

void GoFormatter::visitInterfaceDeclNode(InterfaceDeclNode& node)
{
    printToken("type");
    printSpace();
    printToken(node.m_name.c_str());
    printSpace();
    printToken("interface");
    if (node.m_extends.size() > 0)
    {
        // Go interface embedding
        printNewline();
        printIndent();
        size_t ec = node.m_extends.size();
        for (size_t i = 0; i < ec; i++)
        {
            if (node.m_extends[i] != NULL) { node.m_extends[i]->accept(*this); }
            if (i < ec - 1) { printNewline(); printIndent(); }
        }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
}

void GoFormatter::visitEnumDeclNode(EnumDeclNode& node)
{
    // Go uses iota for enums
    printToken("const");
    printSpace();
    printToken("(");
    printNewline();
    indent();
    printIndent();
    printToken(node.m_name.c_str());
    printToken(" = iota");
    printNewline();
    size_t mc = node.m_members.size();
    for (size_t i = 0; i < mc; i++)
    {
        printIndent();
        if (node.m_members[i] != NULL) { node.m_members[i]->accept(*this); }
        printNewline();
    }
    dedent();
    printIndent();
    printToken(")");
}

void GoFormatter::visitTypeAliasNode(TypeAliasNode& node)
{
    printToken("type");
    printSpace();
    printToken(node.m_name.c_str());
    printSpace();
    if (node.m_typeAnnotation != NULL) { node.m_typeAnnotation->accept(*this); }
    printNewline();
}

void GoFormatter::visitExportDeclNode(ExportDeclNode& node)
{
    // Go uses capitalized names for export, not explicit export keyword
    if (node.m_declaration != NULL) { node.m_declaration->accept(*this); }
    if (!node.m_source.empty())
    {
        printToken("// re-exported from ");
        m_out.printf("\"%s\"", node.m_source.c_str());
        m_currentColumn += static_cast<int>(node.m_source.size()) + 2;
        printNewline();
    }
}

void GoFormatter::visitPreprocessorDirectiveNode(PreprocessorDirectiveNode& node)
{
    // Go doesn't have preprocessor; map to build tags
    printToken("//go:build");
    printSpace();
    printToken(node.m_directive.c_str());
    printNewline();
}

void GoFormatter::visitNamespaceNode(NamespaceNode& node)
{
    // Go package clause, e.g. `package main`
    printToken("package");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printNewline(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void GoFormatter::visitPropertyNode(PropertyNode& node)
{
    if (node.m_key != NULL) { node.m_key->accept(*this); }
    printToken(": ");
    if (node.m_value != NULL) { node.m_value->accept(*this); }
}

void GoFormatter::visitThrowStmtNode(ThrowStmtNode& node)
{
    // Go uses panic() instead of throw
    printToken("panic(");
    if (node.m_expression != NULL) { node.m_expression->accept(*this); }
    printToken(")");
    // Go: no semicolon
    printNewline();
}

void GoFormatter::visitRawTextNode(RawTextNode& node)
{
    printToken(node.m_text.c_str());
    printNewline();
}

void GoFormatter::visitHtmlElementNode(HtmlElementNode& node)
{
    size_t cc = node.m_children.size();
    for (size_t i = 0; i < cc; i++)
    {
        if (node.m_children[i] != NULL) { node.m_children[i]->accept(*this); }
        if (i < cc - 1) { printNewline(); }
    }
}

// Easter egg: Go formatter complete - where simplicity meets concurrency, and semicolons are ghosts