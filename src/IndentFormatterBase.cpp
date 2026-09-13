#include "fmtpp/IndentFormatterBase.hpp"
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

// Easter egg: the indent master awakens - significant whitespace, significant beauty

IndentFormatterBase::IndentFormatterBase(OutputBuffer& out, const Config& config)
    : FormatterVisitor(out, config)
{
}

IndentFormatterBase::~IndentFormatterBase()
{
}

void IndentFormatterBase::format(ASTNode* root)
{
    if (root == NULL) { return; }
    root->accept(*this);
    if (m_out.getSize() > 0) { printNewline(); }
}

void IndentFormatterBase::visitProgram(ProgramNode& node)
{
    size_t count = node.m_statements.size();
    int wantDefs = m_config.blankLinesAroundTopLevelDefs;
    for (size_t i = 0; i < count; i++)
    {
        ASTNode* stmt = node.m_statements[i];
        ASTNode* prev = (i > 0) ? node.m_statements[i - 1] : NULL;
        bool isDef = stmt != NULL && (stmt->isFunctionDecl() || stmt->isClassDecl());
        bool prevDef = prev != NULL && (prev->isFunctionDecl() || prev->isClassDecl());
        if (wantDefs > 0 && i > 0 && (isDef || prevDef)) { printBlankLine(wantDefs); }
        if (stmt != NULL) { stmt->accept(*this); }
        if (i < count - 1) { printNewline(); }
    }
}

void IndentFormatterBase::printSuite(ASTNode* body)
{
    printNewline();
    indent();
    if (body != NULL && body->isBlock())
    {
        BlockNode* block = static_cast<BlockNode*>(body);
        size_t sc = block->m_statements.size();
        for (size_t i = 0; i < sc; i++)
        {
            printIndent();
            if (block->m_statements[i] != NULL) { block->m_statements[i]->accept(*this); }
            if (i < sc - 1) { printNewline(); }
        }
        if (sc == 0)
        {
            printEmptySuite();
        }
    }
    else if (body != NULL)
    {
        printIndent();
        body->accept(*this);
    }
    else
    {
        printEmptySuite();
    }
    printNewline();
    dedent();
}

void IndentFormatterBase::printEmptySuite()
{
}

void IndentFormatterBase::printValue(ASTNode* node)
{
    if (node != NULL) { node->accept(*this); }
}

void IndentFormatterBase::printStringLiteral(LiteralNode& node)
{
    // ponytail: values that already carry quotes (prefixes, triples,
    // raw slices) print verbatim; bare words take the configured quotes
    if (node.m_value.find('"') != std::string::npos
        || node.m_value.find('\'') != std::string::npos)
    {
        printToken(node.m_value.c_str());
        return;
    }
    if (m_config.singleQuote)
    {
        m_out.printf("'%s'", node.m_value.c_str());
    }
    else
    {
        m_out.printf("\"%s\"", node.m_value.c_str());
    }
    m_currentColumn += static_cast<int>(node.m_value.size()) + 2;
}

// --- Shared leaf printers (identical in Python and Ruby) ---

void IndentFormatterBase::visitFunctionDecl(FunctionDeclNode& node)
{
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printSuite(node.m_body); }
    else { printNewline(); }
}

void IndentFormatterBase::visitVariableDecl(VariableDeclNode& node)
{
    if (!node.m_kind.empty())
    {
        printToken(node.m_kind.c_str());
        printSpace();
    }
    printToken(node.m_name.c_str());
    if (!node.m_type.empty())
    {
        printToken(": ");
        printToken(node.m_type.c_str());
    }
    if (node.m_initializer != NULL)
    {
        printAssign();
        node.m_initializer->accept(*this);
    }
    printNewline();
}

void IndentFormatterBase::visitIfStmt(IfStmtNode& node)
{
    printToken("if");
    printSpace();
    printValue(node.m_condition);
    if (node.m_thenBranch != NULL) { printSuite(node.m_thenBranch); }
    else { printNewline(); }
    if (node.m_elseBranch != NULL)
    {
        printIndent();
        printToken("else");
        printSuite(node.m_elseBranch);
    }
}

void IndentFormatterBase::visitForStmt(ForStmtNode& node)
{
    printToken(node.m_keyword.empty() ? "for" : node.m_keyword.c_str());
    if (node.m_condition != NULL)
    {
        printSpace();
        node.m_condition->accept(*this);
    }
    if (node.m_body != NULL) { printSuite(node.m_body); }
    else { printNewline(); }
}

void IndentFormatterBase::visitWhileStmt(WhileStmtNode& node)
{
    printToken(node.m_keyword.empty() ? "while" : node.m_keyword.c_str());
    if (node.m_condition != NULL)
    {
        printSpace();
        node.m_condition->accept(*this);
    }
    if (node.m_body != NULL) { printSuite(node.m_body); }
    else { printNewline(); }
}

void IndentFormatterBase::visitSwitchStmt(SwitchStmtNode& node)
{
    printToken("switch");
    if (node.m_discriminant != NULL)
    {
        printSpace();
        node.m_discriminant->accept(*this);
    }
    printNewline();
    indent();
    size_t cc = node.m_cases.size();
    for (size_t i = 0; i < cc; i++)
    {
        printIndent();
        if (node.m_cases[i] != NULL) { node.m_cases[i]->accept(*this); }
        if (i < cc - 1) { printNewline(); }
    }
    dedent();
    printNewline();
}

void IndentFormatterBase::visitBlock(BlockNode& node)
{
    // ponytail: standalone blocks only occur as Ruby do-blocks or as a
    // safety fallback; suites print without any delimiters here
    printSuite(&node);
}

void IndentFormatterBase::visitReturnStmt(ReturnStmtNode& node)
{
    printToken("return");
    if (node.m_argument != NULL) { printSpace(); node.m_argument->accept(*this); }
    printNewline();
}

void IndentFormatterBase::visitThrowStmt(ThrowStmtNode& node)
{
    printToken("raise");
    if (node.m_expression != NULL) { printSpace(); node.m_expression->accept(*this); }
    printNewline();
}

void IndentFormatterBase::visitTryCatchStmt(TryCatchStmtNode& node)
{
    // ponytail: extras (further catches) carry no try body, so no header;
    // the statement's first line always rides the caller slot, later parts
    // start fresh lines with their own indent
    bool bare = (node.m_body == NULL);
    if (node.m_body != NULL)
    {
        printToken("try");
        printSuite(node.m_body);
    }
    if (node.m_catchClause != NULL || node.m_catchBody != NULL)
    {
        if (!bare) { printIndent(); }
        bare = false;
        printToken("except");
        if (node.m_catchClause != NULL)
        {
            printSpace();
            node.m_catchClause->accept(*this);
        }
        if (node.m_catchBody != NULL) { printSuite(node.m_catchBody); }
        else { printNewline(); }
    }
    for (size_t eci = 0; eci < node.m_extraCatches.size(); eci++)
    {
        if (node.m_extraCatches[eci] == NULL) { continue; }
        // ponytail: slot indent for the extra's bare first line
        printIndent();
        node.m_extraCatches[eci]->accept(*this);
    }
    if (node.m_elseBlock != NULL)
    {
        if (!bare) { printIndent(); }
        bare = false;
        printToken("else");
        printSuite(node.m_elseBlock);
    }
    if (node.m_finallyBlock != NULL)
    {
        if (!bare) { printIndent(); }
        bare = false;
        printToken("finally");
        printSuite(node.m_finallyBlock);
    }
}

void IndentFormatterBase::visitComment(CommentNode& node)
{
    // ponytail: the stored text excludes the #; restore the space, except
    // for shebangs (#!/...) where the bytes are significant
    printToken("#");
    if (!node.m_text.empty() && node.m_text[0] != '!'
        && node.m_text[0] != ' ' && node.m_text[0] != '\t')
    {
        printSpace();
    }
    printToken(node.m_text.c_str());
    printNewline();
}

void IndentFormatterBase::visitLiteral(LiteralNode& node)
{
    switch (node.m_type)
    {
    case LiteralNode::TYPE_STRING:
        printStringLiteral(node);
        break;
    case LiteralNode::TYPE_BOOLEAN:
    case LiteralNode::TYPE_NUMBER:
        printToken(node.m_value.c_str());
        break;
    case LiteralNode::TYPE_NULL:
        printToken("None");
        break;
    case LiteralNode::TYPE_REGEX:
    default:
        printToken(node.m_value.c_str());
        break;
    }
}

void IndentFormatterBase::visitIdentifier(IdentifierNode& node)
{
    printToken(node.m_name.c_str());
}

void IndentFormatterBase::visitArrayLiteral(ArrayLiteralNode& node)
{
    printToken("[");
    if (m_config.spaceInBrackets && node.m_elements.size() > 0) { printSpace(); }
    size_t ec = node.m_elements.size();
    for (size_t i = 0; i < ec; i++)
    {
        if (node.m_elements[i] != NULL) { node.m_elements[i]->accept(*this); }
        if (i < ec - 1) { printComma(); }
    }
    if (m_config.spaceInBrackets && node.m_elements.size() > 0) { printSpace(); }
    printToken("]");
}

void IndentFormatterBase::visitObjectLiteral(ObjectLiteralNode& node)
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

void IndentFormatterBase::visitTemplateLiteral(TemplateLiteralNode& node)
{
    size_t qc = node.m_quasis.size();
    for (size_t i = 0; i < qc; i++)
    {
        if (node.m_quasis[i] != NULL) { node.m_quasis[i]->accept(*this); }
        if (i < node.m_expressions.size() && node.m_expressions[i] != NULL)
        {
            node.m_expressions[i]->accept(*this);
        }
    }
}

void IndentFormatterBase::visitArrowFunction(ArrowFunctionNode& node)
{
    size_t pc = node.m_params.size();
    for (size_t i = 0; i < pc; i++)
    {
        if (node.m_params[i] != NULL) { node.m_params[i]->accept(*this); }
        if (i < pc - 1) { printComma(); }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
}

void IndentFormatterBase::visitClassDecl(ClassDeclNode& node)
{
    printToken(node.m_kind.empty() ? "class" : node.m_kind.c_str());
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printSuite(node.m_body); }
    else { printNewline(); }
}

void IndentFormatterBase::visitInterfaceDecl(InterfaceDeclNode& node)
{
    printToken("interface");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printSuite(node.m_body); }
    else { printNewline(); }
}

void IndentFormatterBase::visitEnumDecl(EnumDeclNode& node)
{
    printToken("enum");
    printSpace();
    printToken(node.m_name.c_str());
    printNewline();
    indent();
    size_t mc = node.m_members.size();
    for (size_t i = 0; i < mc; i++)
    {
        printIndent();
        if (node.m_members[i] != NULL) { node.m_members[i]->accept(*this); }
        if (i < mc - 1) { printNewline(); }
    }
    dedent();
    printNewline();
}

void IndentFormatterBase::visitTypeAlias(TypeAliasNode& node)
{
    printToken("type");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_typeAnnotation != NULL)
    {
        printAssign();
        node.m_typeAnnotation->accept(*this);
    }
    printNewline();
}

void IndentFormatterBase::visitImportDecl(ImportDeclNode& node)
{
    printToken("import");
    if (!node.m_source.empty())
    {
        printSpace();
        printToken(node.m_source.c_str());
    }
    printNewline();
}

void IndentFormatterBase::visitExportDecl(ExportDeclNode& node)
{
    printToken("export");
    if (node.m_declaration != NULL)
    {
        printSpace();
        node.m_declaration->accept(*this);
    }
    else { printNewline(); }
}

void IndentFormatterBase::visitPreprocessorDirective(PreprocessorDirectiveNode& node)
{
    printToken(node.m_directive.c_str());
    printNewline();
}

void IndentFormatterBase::visitNamespace(NamespaceNode& node)
{
    printToken("namespace");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printSuite(node.m_body); }
    else { printNewline(); }
}

void IndentFormatterBase::visitProperty(PropertyNode& node)
{
    if (node.m_key != NULL) { node.m_key->accept(*this); }
    printToken(": ");
    if (node.m_value != NULL) { node.m_value->accept(*this); }
}

void IndentFormatterBase::visitRawText(RawTextNode& node)
{
    printToken(node.m_text.c_str());
    printNewline();
}

void IndentFormatterBase::visitHtmlElement(HtmlElementNode& node)
{
    size_t cc = node.m_children.size();
    for (size_t i = 0; i < cc; i++)
    {
        if (node.m_children[i] != NULL) { node.m_children[i]->accept(*this); }
        if (i < cc - 1) { printNewline(); }
    }
}

void IndentFormatterBase::visitWith(WithNode& node)
{
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
        node.m_context->accept(*this);
    }
    if (node.m_body != NULL) { printSuite(node.m_body); }
    else { printNewline(); }
}

void IndentFormatterBase::visitSwitchCase(SwitchCaseNode& node)
{
    // ponytail: RawText patterns self-terminate, which would blank-line
    // before the suite; print the token bare, the suite brings newlines
    if (node.m_pattern != NULL && node.m_pattern->isRawText())
    {
        RawTextNode* raw = static_cast<RawTextNode*>(node.m_pattern);
        printToken(raw->m_text.c_str());
    }
    else if (node.m_pattern != NULL) { node.m_pattern->accept(*this); }
    if (node.m_body != NULL) { printSuite(node.m_body); }
    else { printNewline(); }
}

// Easter egg: the indent master rests - significativo, indentato, bellissimo
