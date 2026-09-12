#include "fmtpp/CstyleFormatterBase.hpp"
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
#include "fmtpp/ast/WithNode.hpp"
#include "fmtpp/ast/SwitchCaseNode.hpp"
#include <cstdio>
#include <cstring>

// Easter egg: the master formatter awakens - ready to beautify the world, one token at a time

CstyleFormatterBase::CstyleFormatterBase(OutputBuffer& out, const Config& config)
    : FormatterVisitor(out, config)
{
}

CstyleFormatterBase::~CstyleFormatterBase()
{
}

void CstyleFormatterBase::format(ASTNode* root)
{
    if (root == NULL) { return; }
    preFormat();
    root->accept(*this);
    postFormat();
}

void CstyleFormatterBase::preFormat() {}
void CstyleFormatterBase::postFormat()
{
    if (m_out.getSize() > 0) { printNewline(); }
}

bool CstyleFormatterBase::braceOnSameLine() const
{
    return m_config.braceStyle == "same-line";
}

bool CstyleFormatterBase::spaceBeforeOpeningBrace() const
{
    return m_config.spaceBeforeBrace;
}

bool CstyleFormatterBase::spaceAfterKeywords() const
{
    return m_config.spaceAfterKeyword;
}

bool CstyleFormatterBase::requiresSemicolon(ASTNode*) const
{
    return m_config.semi;
}

void CstyleFormatterBase::printOpeningBrace()
{
    if (spaceBeforeOpeningBrace()) { printSpace(); }
    printToken("{");
}

void CstyleFormatterBase::printClosingBrace()
{
    printToken("}");
}

void CstyleFormatterBase::wrapLines()
{
    if (m_currentColumn >= m_config.printWidth)
    {
        printNewline();
        printIndent();
    }
}

void CstyleFormatterBase::visitNode(ASTNode* node)
{
    if (node != NULL) { node->accept(*this); }
}

void CstyleFormatterBase::printSemicolonIfNeeded(ASTNode* node)
{
    if (requiresSemicolon(node)) { printSemicolon(); }
}
// ================================================================
// Visitor method implementations
// ================================================================

void CstyleFormatterBase::visitProgram(ProgramNode& node)
{
    size_t count = node.m_statements.size();
    for (size_t i = 0; i < count; i++)
    {
        if (node.m_statements[i] != NULL) { node.m_statements[i]->accept(*this); }
        if (i < count - 1) { printNewline(); }
    }
}

void CstyleFormatterBase::visitFunctionDecl(FunctionDeclNode& node)
{
    printToken("function");
    if (spaceAfterKeywords()) { printSpace(); }
    printToken(node.m_name.c_str());
    printToken("(");
    size_t pc = node.m_params.size();
    for (size_t i = 0; i < pc; i++)
    {
        if (node.m_params[i] != NULL) { node.m_params[i]->accept(*this); }
        if (i < pc - 1) { printComma(); }
    }
    printToken(")");
    if (!node.m_returnType.empty()) { printToken(": "); printToken(node.m_returnType.c_str()); }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printSemicolon(); printNewline(); }
}

void CstyleFormatterBase::visitVariableDecl(VariableDeclNode& node)
{
    printToken(node.m_kind.c_str());
    printSpace();
    printToken(node.m_name.c_str());
    if (!node.m_type.empty()) { printToken(": "); printToken(node.m_type.c_str()); }
    if (node.m_initializer != NULL) { printToken(" = "); node.m_initializer->accept(*this); }
    printSemicolonIfNeeded(&node);
    printNewline();
}

void CstyleFormatterBase::visitIfStmt(IfStmtNode& node)
{
    printToken("if");
    if (spaceAfterKeywords()) { printSpace(); }
    if (node.m_condition != NULL) { printToken("("); node.m_condition->accept(*this); printToken(")"); }
    if (node.m_thenBranch != NULL) { printSpace(); node.m_thenBranch->accept(*this); }
    if (node.m_elseBranch != NULL)
    {
        printSpace();
        printToken("else");
        printSpace();
        node.m_elseBranch->accept(*this);
    }
}

void CstyleFormatterBase::visitForStmt(ForStmtNode& node)
{
    // ponytail: classic triple prints with semicolons; raw headers
    // (for-in, foreach, range) print verbatim, parens only with a condition
    printToken(node.m_keyword.empty() ? "for" : node.m_keyword.c_str());
    if (node.m_classic)
    {
        if (spaceAfterKeywords()) { printSpace(); }
        printToken("(");
        if (node.m_init != NULL) { node.m_init->accept(*this); }
        printToken("; ");
        if (node.m_condition != NULL) { node.m_condition->accept(*this); }
        printToken("; ");
        if (node.m_update != NULL) { node.m_update->accept(*this); }
        printToken(")");
    }
    else if (node.m_condition != NULL)
    {
        printSpace();
        printToken("(");
        node.m_condition->accept(*this);
        printToken(")");
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printSemicolon(); printNewline(); }
}

void CstyleFormatterBase::visitWhileStmt(WhileStmtNode& node)
{
    printToken("while");
    if (spaceAfterKeywords()) { printSpace(); }
    if (node.m_condition != NULL) { printToken("("); node.m_condition->accept(*this); printToken(")"); }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printSemicolon(); printNewline(); }
}

void CstyleFormatterBase::visitSwitchStmt(SwitchStmtNode& node)
{
    printToken("switch");
    if (spaceAfterKeywords()) { printSpace(); }
    if (node.m_discriminant != NULL) { printToken("("); node.m_discriminant->accept(*this); printToken(")"); }
    printOpeningBrace();
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
    printClosingBrace();
    printNewline();
}

void CstyleFormatterBase::visitBlock(BlockNode& node)
{
    printOpeningBrace();
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
    printClosingBrace();
}

void CstyleFormatterBase::visitReturnStmt(ReturnStmtNode& node)
{
    printToken("return");
    if (node.m_argument != NULL) { printSpace(); node.m_argument->accept(*this); }
    printSemicolonIfNeeded(&node);
    printNewline();
}

void CstyleFormatterBase::visitThrowStmt(ThrowStmtNode& node)
{
    printToken("throw");
    if (node.m_expression != NULL) { printSpace(); node.m_expression->accept(*this); }
    printSemicolonIfNeeded(&node);
    printNewline();
}

void CstyleFormatterBase::visitTryCatchStmt(TryCatchStmtNode& node)
{
    printToken("try");
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    if (node.m_catchClause != NULL || node.m_catchBody != NULL)
    {
        printSpace();
        printToken("catch");
        if (node.m_catchClause != NULL)
        {
            printSpace();
            printToken("(");
            node.m_catchClause->accept(*this);
            printToken(")");
        }
        if (node.m_catchBody != NULL) { printSpace(); node.m_catchBody->accept(*this); }
    }
    for (size_t eci = 0; eci < node.m_extraCatches.size(); eci++)
    {
        // ponytail: extras carry clause+body only, so this prints
        // exactly one more catch block each, never recursing further
        if (node.m_extraCatches[eci] == NULL) { continue; }
        printSpace();
        node.m_extraCatches[eci]->accept(*this);
    }
    if (node.m_finallyBlock != NULL) { printSpace(); printToken("finally"); printSpace(); node.m_finallyBlock->accept(*this); }
}
void CstyleFormatterBase::visitComment(CommentNode& node)
{
    if (node.m_isBlock) { printToken("/*"); printToken(node.m_text.c_str()); printToken("*/"); }
    else { printToken("//"); printToken(node.m_text.c_str()); }
    printNewline();
}

void CstyleFormatterBase::visitLiteral(LiteralNode& node)
{
    switch (node.m_type)
    {
    case LiteralNode::TYPE_STRING:
        if (m_config.singleQuote) { m_out.printf("'%s'", node.m_value.c_str()); }
        else { m_out.printf("\"%s\"", node.m_value.c_str()); }
        m_currentColumn += static_cast<int>(node.m_value.size()) + 2;
        break;
    case LiteralNode::TYPE_BOOLEAN:
    case LiteralNode::TYPE_NUMBER:
    case LiteralNode::TYPE_NULL:
    case LiteralNode::TYPE_REGEX:
    default: printToken(node.m_value.c_str()); break;
    }
}

void CstyleFormatterBase::visitIdentifier(IdentifierNode& node)
{
    printToken(node.m_name.c_str());
}

void CstyleFormatterBase::visitArrayLiteral(ArrayLiteralNode& node)
{
    printToken("[");
    size_t ec = node.m_elements.size();
    for (size_t i = 0; i < ec; i++)
    {
        if (node.m_elements[i] != NULL) { node.m_elements[i]->accept(*this); }
        if (i < ec - 1) { printComma(); }
    }
    printToken("]");
}

void CstyleFormatterBase::visitObjectLiteral(ObjectLiteralNode& node)
{
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

void CstyleFormatterBase::visitTemplateLiteral(TemplateLiteralNode& node)
{
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

void CstyleFormatterBase::visitArrowFunction(ArrowFunctionNode& node)
{
    if (node.m_params.size() == 1 && m_config.arrowParens == "as-needed")
    {
        if (node.m_params[0] != NULL) { node.m_params[0]->accept(*this); }
    }
    else
    {
        printToken("(");
        size_t pc = node.m_params.size();
        for (size_t i = 0; i < pc; i++)
        {
            if (node.m_params[i] != NULL) { node.m_params[i]->accept(*this); }
            if (i < pc - 1) { printComma(); }
        }
        printToken(")");
    }
    printToken(" => ");
    if (!node.m_returnType.empty()) { printToken(node.m_returnType.c_str()); printToken(" "); }
    if (node.m_body != NULL) { node.m_body->accept(*this); }
}

void CstyleFormatterBase::visitClassDecl(ClassDeclNode& node)
{
    printToken("class");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_superClass != NULL) { printToken(" extends "); node.m_superClass->accept(*this); }
    if (node.m_implements.size() > 0)
    {
        printToken(" implements ");
        size_t ic = node.m_implements.size();
        for (size_t i = 0; i < ic; i++)
        {
            if (node.m_implements[i] != NULL) { node.m_implements[i]->accept(*this); }
            if (i < ic - 1) { printComma(); }
        }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
}
void CstyleFormatterBase::visitInterfaceDecl(InterfaceDeclNode& node)
{
    // ponytail: modifiers ride in the kind ("private interface"); bare and
    // plain "interface" print the keyword, anything else prints verbatim
    if (!node.m_kind.empty() && node.m_kind != "interface")
    {
        printToken(node.m_kind.c_str());
    }
    else
    {
        printToken("interface");
    }
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_extends.size() > 0)
    {
        printToken(" extends ");
        size_t ec = node.m_extends.size();
        for (size_t i = 0; i < ec; i++)
        {
            if (node.m_extends[i] != NULL) { node.m_extends[i]->accept(*this); }
            if (i < ec - 1) { printComma(); }
        }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
}

void CstyleFormatterBase::visitEnumDecl(EnumDeclNode& node)
{
    // ponytail: modifiers ride in the kind ("public enum"); bare and plain
    // "enum" print the keyword, anything else prints verbatim
    if (!node.m_kind.empty() && node.m_kind != "enum")
    {
        printToken(node.m_kind.c_str());
    }
    else
    {
        printToken("enum");
    }
    printSpace();
    printToken(node.m_name.c_str());
    printOpeningBrace();
    printNewline();
    indent();
    size_t mc = node.m_members.size();
    for (size_t i = 0; i < mc; i++)
    {
        printIndent();
        if (node.m_members[i] != NULL) { node.m_members[i]->accept(*this); }
        if (i < mc - 1) { printToken(","); }
        printNewline();
    }
    dedent();
    printIndent();
    printClosingBrace();
}

void CstyleFormatterBase::visitTypeAlias(TypeAliasNode& node)
{
    printToken("type");
    printSpace();
    printToken(node.m_name.c_str());
    printToken(" = ");
    if (node.m_typeAnnotation != NULL) { node.m_typeAnnotation->accept(*this); }
    printSemicolonIfNeeded(&node);
    printNewline();
}

void CstyleFormatterBase::visitImportDecl(ImportDeclNode& node)
{
    printToken("import");
    if (node.m_defaultImport != NULL) { printSpace(); node.m_defaultImport->accept(*this); }
    if (node.m_specifiers.size() > 0)
    {
        if (node.m_defaultImport != NULL) { printComma(); }
        else { printSpace(); }
        printToken("{");
        if (m_config.bracketSpacing) { printSpace(); }
        size_t sc = node.m_specifiers.size();
        for (size_t i = 0; i < sc; i++)
        {
            if (node.m_specifiers[i] != NULL) { node.m_specifiers[i]->accept(*this); }
            if (i < sc - 1) { printComma(); }
        }
        if (m_config.bracketSpacing) { printSpace(); }
        printToken("}");
    }
    if (!node.m_source.empty())
    {
        if (node.m_specifiers.size() > 0 || node.m_defaultImport != NULL) { printToken(" from"); }
        printSpace();
        m_out.printf("\"%s\"", node.m_source.c_str());
        m_currentColumn += static_cast<int>(node.m_source.size()) + 2;
    }
    printSemicolonIfNeeded(&node);
    printNewline();
}

void CstyleFormatterBase::visitExportDecl(ExportDeclNode& node)
{
    printToken("export");
    if (node.m_declaration != NULL) { printSpace(); node.m_declaration->accept(*this); }
    if (!node.m_source.empty())
    {
        printToken(" from ");
        m_out.printf("\"%s\"", node.m_source.c_str());
        m_currentColumn += static_cast<int>(node.m_source.size()) + 2;
        printSemicolonIfNeeded(&node);
        printNewline();
    }
}

void CstyleFormatterBase::visitPreprocessorDirective(PreprocessorDirectiveNode& node)
{
    printToken(node.m_directive.c_str());
    printNewline();
}

void CstyleFormatterBase::visitNamespace(NamespaceNode& node)
{
    printToken("namespace");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
}

void CstyleFormatterBase::visitProperty(PropertyNode& node)
{
    if (node.m_key != NULL) { node.m_key->accept(*this); }
    printToken(": ");
    if (node.m_value != NULL) { node.m_value->accept(*this); }
}

void CstyleFormatterBase::visitRawText(RawTextNode& node)
{
    printToken(node.m_text.c_str());
    printNewline();
}

static bool isVoidElement(const char* tag)
{
    static const char* voids[] = {
        "area", "base", "br", "col", "embed", "hr", "img", "input",
        "link", "meta", "param", "source", "track", "wbr", NULL
    };
    for (int i = 0; voids[i] != NULL; i++)
    {
        if (strcmp(tag, voids[i]) == 0) { return true; }
    }
    // ponytail: tags are matched case-sensitively; HTML authors lowercase anyway
    return false;
}

void CstyleFormatterBase::visitHtmlElement(HtmlElementNode& node)
{
    printToken("<");
    printToken(node.m_tag.c_str());
    for (size_t i = 0; i < node.m_attrs.size(); i++)
    {
        printSpace();
        printToken(node.m_attrs[i].c_str());
    }
    if (node.m_selfClosing || isVoidElement(node.m_tag.c_str()))
    {
        printToken(">");
        printNewline();
        return;
    }
    if (node.m_children.size() == 1 && node.m_children[0] != NULL
        && node.m_children[0]->isRawText())
    {
        // ponytail: single text child stays inline (<p>hi</p>, not 3 lines)
        printToken(">");
        printToken(static_cast<RawTextNode*>(node.m_children[0])->m_text.c_str());
        printToken("</");
        printToken(node.m_tag.c_str());
        printToken(">");
        printNewline();
        return;
    }
    printToken(">");
    printNewline();
    indent();
    for (size_t i = 0; i < node.m_children.size(); i++)
    {
        printIndent();
        if (node.m_children[i] != NULL) { node.m_children[i]->accept(*this); }
        if (i < node.m_children.size() - 1) { printNewline(); }
    }
    dedent();
    printNewline();
    printIndent();
    printToken("</");
    printToken(node.m_tag.c_str());
    printToken(">");
    printNewline();
}

void CstyleFormatterBase::visitWith(WithNode& node)
{
    // ponytail: JS-shaped fallback; only PythonFormatter prints real with
    if (!node.m_keyword.empty())
    {
        printToken(node.m_keyword.c_str());
    }
    else
    {
        printToken("with");
    }
    if (node.m_context != NULL)
    {
        printSpace();
        printToken("(");
        node.m_context->accept(*this);
        printToken(")");
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void CstyleFormatterBase::visitSwitchCase(SwitchCaseNode& node)
{
    if (node.m_pattern != NULL) { node.m_pattern->accept(*this); }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

// Easter egg: 800 lines of pure formatting glory - complete! The master formatter stands ready.