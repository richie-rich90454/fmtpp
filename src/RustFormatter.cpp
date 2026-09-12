#include "fmtpp/RustFormatter.hpp"
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

// Easter egg: the Rust formatter awakens - ready to fight the borrow checker

RustFormatter::RustFormatter(OutputBuffer& out, const Config& config)
    : FormatterVisitor(out, config)
{
}

RustFormatter::~RustFormatter()
{
}

void RustFormatter::format(ASTNode* root)
{
    if (root == NULL) { return; }
    root->accept(*this);
    if (m_out.getSize() > 0) { printNewline(); }
}

// ================================================================
// visitProgram - top-level items
// ================================================================
void RustFormatter::visitProgram(ProgramNode& node)
{
    size_t count = node.m_statements.size();
    for (size_t i = 0; i < count; i++)
    {
        if (node.m_statements[i] != NULL) { node.m_statements[i]->accept(*this); }
        if (i < count - 1) { printNewline(); }
    }
}

// ================================================================
// visitFunctionDecl - fn name(params) -> ReturnType { body }
// ================================================================
void RustFormatter::visitFunctionDecl(FunctionDeclNode& node)
{
    if (!node.m_modifiers.empty())
    {
        printToken(node.m_modifiers.c_str());
        printSpace();
    }
    printToken("fn");
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
    if (!node.m_returnType.empty())
    {
        printToken(" -> ");
        printToken(node.m_returnType.c_str());
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

// ================================================================
// visitVariableDecl - let [mut] name: Type = value;
// ================================================================
void RustFormatter::visitVariableDecl(VariableDeclNode& node)
{
    // ponytail: shapeless leftovers (struct fields as raw text) print verbatim
    if (node.m_kind.empty() && node.m_type.empty() && node.m_initializer == NULL)
    {
        printToken(node.m_name.c_str());
        printNewline();
        return;
    }
    // ponytail: multi-word left sides we never modeled (pub static X) print
    // verbatim instead of gaining an invented let
    if (node.m_kind.empty() && node.m_name.find_first_of(" \t\n") != std::string::npos)
    {
        printToken(node.m_name.c_str());
        if (!node.m_type.empty())
        {
            printToken(": ");
            printToken(node.m_type.c_str());
        }
        if (node.m_initializer != NULL)
        {
            printToken(" = ");
            node.m_initializer->accept(*this);
        }
        printToken(";");
        printNewline();
        return;
    }
    if (node.m_kind == "const")
    {
        printToken("const");
    }
    else if (node.m_kind == "static")
    {
        printToken("static");
    }
    else
    {
        printToken("let");
    }
    if (node.m_kind == "mut" || node.m_kind == "let mut" || node.m_kind == "var mut")
    {
        printSpace();
        printToken("mut");
    }
    printSpace();
    printToken(node.m_name.c_str());
    if (!node.m_type.empty())
    {
        printToken(": ");
        printToken(node.m_type.c_str());
    }
    if (node.m_initializer != NULL)
    {
        printToken(" = ");
        node.m_initializer->accept(*this);
    }
    printToken(";");
    printNewline();
}

// ================================================================
// visitIfStmt - if condition { body } else { body }
// ================================================================
void RustFormatter::visitIfStmt(IfStmtNode& node)
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

// ================================================================
// visitForStmt - for pattern in expr { body }
// ================================================================
void RustFormatter::visitForStmt(ForStmtNode& node)
{
    // ponytail: a C-style triple here means invalid Rust input;
    // print it part-preserving rather than dropping the update
    if (node.m_classic && node.m_update != NULL)
    {
        printToken("for (");
        if (node.m_init != NULL) { node.m_init->accept(*this); }
        printToken("; ");
        if (node.m_condition != NULL) { node.m_condition->accept(*this); }
        printToken("; ");
        node.m_update->accept(*this);
        printToken(")");
        if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
        else { printToken(";"); printNewline(); }
        return;
    }
    printToken(node.m_keyword.empty() ? "for" : node.m_keyword.c_str());
    printSpace();
    // ponytail: for-in headers stay whole (pattern lives inside the
    // condition); inventing " in " around a whole header duplicates it
    // on every pass, so only split shapes print the separator
    if (node.m_init != NULL)
    {
        node.m_init->accept(*this);
        if (node.m_condition != NULL)
        {
            printToken(" in ");
            node.m_condition->accept(*this);
        }
    }
    else if (node.m_condition != NULL)
    {
        node.m_condition->accept(*this);
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

// ================================================================
// visitWhileStmt - while condition { body }
// ================================================================
void RustFormatter::visitWhileStmt(WhileStmtNode& node)
{
    printToken("while");
    printSpace();
    if (node.m_condition != NULL) { node.m_condition->accept(*this); }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

// ================================================================
// visitSwitchStmt - match discriminant { cases }
// ================================================================
void RustFormatter::visitSwitchStmt(SwitchStmtNode& node)
{
    printToken("match");
    printSpace();
    if (node.m_discriminant != NULL) { node.m_discriminant->accept(*this); }
    printSpace();
    printToken("{");
    printNewline();
    indent();
    size_t cc = node.m_cases.size();
    for (size_t i = 0; i < cc; i++)
    {
        // ponytail: RawText arms self-terminate, so print the token bare
        // and own the comma+newline; comments print bare with no comma
        printIndent();
        if (node.m_cases[i] != NULL && node.m_cases[i]->isRawText())
        {
            RawTextNode* raw = static_cast<RawTextNode*>(node.m_cases[i]);
            printToken(raw->m_text.c_str());
            printToken(",");
            printNewline();
        }
        else if (node.m_cases[i] != NULL)
        {
            node.m_cases[i]->accept(*this);
        }
    }
    dedent();
    printIndent();
    printToken("}");
    printNewline();
}

// ================================================================
// visitBlock - Rust-style block (same-line brace)
// ================================================================
void RustFormatter::visitBlock(BlockNode& node)
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

// ================================================================
// visitReturnStmt - return [value];
// Rust uses expression-based returns, but return keyword is still valid
// ================================================================
void RustFormatter::visitReturnStmt(ReturnStmtNode& node)
{
    printToken("return");
    if (node.m_argument != NULL) { printSpace(); node.m_argument->accept(*this); }
    printToken(";");
    printNewline();
}

// ================================================================
// visitThrowStmt - panic!(expr);
// ================================================================
void RustFormatter::visitThrowStmt(ThrowStmtNode& node)
{
    printToken("panic!");
    printToken("(");
    if (node.m_expression != NULL) { node.m_expression->accept(*this); }
    printToken(")");
    printToken(";");
    printNewline();
}

// ================================================================
// visitTryCatchStmt - Rust Result/Error handling with match
// ================================================================
void RustFormatter::visitTryCatchStmt(TryCatchStmtNode& node)
{
    if (node.m_body != NULL)
    {
        node.m_body->accept(*this);
    }
    if (node.m_catchClause != NULL)
    {
        printNewline();
        printIndent();
        printToken("// catch clause (Rust uses Result<T, E> + match)");
        printNewline();
        printIndent();
        node.m_catchClause->accept(*this);
    }
    if (node.m_catchBody != NULL)
    {
        printNewline();
        printIndent();
        node.m_catchBody->accept(*this);
    }
    for (size_t eci = 0; eci < node.m_extraCatches.size(); eci++)
    {
        if (node.m_extraCatches[eci] == NULL) { continue; }
        node.m_extraCatches[eci]->accept(*this);
    }
    if (node.m_finallyBlock != NULL)
    {
        printNewline();
        printIndent();
        printToken("// finally block (Rust uses Drop trait for cleanup)");
        printNewline();
        printIndent();
        node.m_finallyBlock->accept(*this);
    }
}

// ================================================================
// visitComment - // or /* */
// ================================================================
void RustFormatter::visitComment(CommentNode& node)
{
    if (node.m_isBlock)
    {
        printToken("/*");
        printToken(node.m_text.c_str());
        printToken("*/");
    }
    else
    {
        printToken("//");
        printToken(node.m_text.c_str());
    }
    printNewline();
}

// ================================================================
// visitLiteral - Rust-style literals
// ================================================================
void RustFormatter::visitLiteral(LiteralNode& node)
{
    switch (node.m_type)
    {
    case LiteralNode::TYPE_STRING:
        if (m_config.singleQuote)
        {
            m_out.printf("'%s'", node.m_value.c_str());
        }
        else
        {
            m_out.printf("\"%s\"", node.m_value.c_str());
        }
        m_currentColumn += static_cast<int>(node.m_value.size()) + 2;
        break;
    case LiteralNode::TYPE_BOOLEAN:
        printToken(node.m_value.c_str());
        break;
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

// ================================================================
// visitIdentifier
// ================================================================
void RustFormatter::visitIdentifier(IdentifierNode& node)
{
    printToken(node.m_name.c_str());
}

// ================================================================
// visitArrayLiteral - vec![elem1, elem2]
// ================================================================
void RustFormatter::visitArrayLiteral(ArrayLiteralNode& node)
{
    printToken("vec![");
    size_t ec = node.m_elements.size();
    for (size_t i = 0; i < ec; i++)
    {
        if (node.m_elements[i] != NULL) { node.m_elements[i]->accept(*this); }
        if (i < ec - 1) { printComma(); }
    }
    printToken("]");
}

// ================================================================
// visitObjectLiteral - StructName { key: value }
// ================================================================
void RustFormatter::visitObjectLiteral(ObjectLiteralNode& node)
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

// ================================================================
// visitTemplateLiteral - format!("...{}...", expr)
// ================================================================
void RustFormatter::visitTemplateLiteral(TemplateLiteralNode& node)
{
    printToken("format!");
    printToken("(");
    printToken("\"");
    size_t qc = node.m_quasis.size();
    for (size_t i = 0; i < qc; i++)
    {
        if (node.m_quasis[i] != NULL) { node.m_quasis[i]->accept(*this); }
        if (i < node.m_expressions.size()) { printToken("{}"); }
    }
    printToken("\"");
    for (size_t i = 0; i < node.m_expressions.size(); i++)
    {
        printToken(", ");
        if (node.m_expressions[i] != NULL) { node.m_expressions[i]->accept(*this); }
    }
    printToken(")");
}

// ================================================================
// visitArrowFunction - |params| -> ReturnType { body } or |params| expr
// ================================================================
void RustFormatter::visitArrowFunction(ArrowFunctionNode& node)
{
    printToken("|");
    size_t pc = node.m_params.size();
    for (size_t i = 0; i < pc; i++)
    {
        if (node.m_params[i] != NULL) { node.m_params[i]->accept(*this); }
        if (i < pc - 1) { printComma(); }
    }
    printToken("|");
    if (!node.m_returnType.empty())
    {
        printToken(" -> ");
        printToken(node.m_returnType.c_str());
        printSpace();
    }
    if (node.m_body != NULL) { node.m_body->accept(*this); }
}

// ================================================================
// visitClassDecl - impl Name { ... } or impl TraitName for TypeName { ... }
// ================================================================
void RustFormatter::visitClassDecl(ClassDeclNode& node)
{
    // ponytail: struct stays a struct even with visibility prefixes
    // (pub struct); anything else is an impl block
    std::string kind = node.m_kind;
    std::string prefix;
    if (kind == "struct")
    {
        // plain struct, nothing to strip
    }
    else if (kind.size() > 7
        && kind.compare(kind.size() - 7, 7, " struct") == 0)
    {
        prefix = kind.substr(0, kind.size() - 7);
        kind = "struct";
    }
    if (kind == "struct")
    {
        if (!prefix.empty())
        {
            printToken(prefix.c_str());
            printSpace();
        }
        printToken("struct");
        printSpace();
        printToken(node.m_name.c_str());
        if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
        else { printToken(";"); printNewline(); }
        return;
    }
    if (node.m_superClass != NULL)
    {
        printToken("impl");
        printSpace();
        node.m_superClass->accept(*this);
        printToken(" for ");
        printToken(node.m_name.c_str());
    }
    else
    {
        printToken("impl");
        printSpace();
        printToken(node.m_name.c_str());
    }
    if (node.m_implements.size() > 0)
    {
        printToken(": ");
        size_t ic = node.m_implements.size();
        for (size_t i = 0; i < ic; i++)
        {
            if (node.m_implements[i] != NULL) { node.m_implements[i]->accept(*this); }
            if (i < ic - 1) { printToken(" + "); }
        }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

// ================================================================
// visitInterfaceDecl - trait Name { ... }
// ================================================================
void RustFormatter::visitInterfaceDecl(InterfaceDeclNode& node)
{
    printToken("trait");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_extends.size() > 0)
    {
        printToken(": ");
        size_t ec = node.m_extends.size();
        for (size_t i = 0; i < ec; i++)
        {
            if (node.m_extends[i] != NULL) { node.m_extends[i]->accept(*this); }
            if (i < ec - 1) { printToken(" + "); }
        }
    }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

// ================================================================
// visitEnumDecl - enum Name { variants }
// ================================================================
void RustFormatter::visitEnumDecl(EnumDeclNode& node)
{
    printToken("enum");
    printSpace();
    printToken(node.m_name.c_str());
    printSpace();
    printToken("{");
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
    printToken("}");
}

// ================================================================
// visitTypeAlias - type Name = Type;
// ================================================================
void RustFormatter::visitTypeAlias(TypeAliasNode& node)
{
    printToken("type");
    printSpace();
    printToken(node.m_name.c_str());
    printToken(" = ");
    if (node.m_typeAnnotation != NULL) { node.m_typeAnnotation->accept(*this); }
    printToken(";");
    printNewline();
}

// ================================================================
// visitImportDecl - use path::to::module;
// ================================================================
void RustFormatter::visitImportDecl(ImportDeclNode& node)
{
    printToken("use");
    printSpace();
    if (!node.m_source.empty())
    {
        printToken(node.m_source.c_str());
        printToken("::");
    }
    if (node.m_defaultImport != NULL)
    {
        node.m_defaultImport->accept(*this);
    }
    if (node.m_specifiers.size() > 0)
    {
        if (node.m_specifiers.size() == 1 && node.m_defaultImport == NULL)
        {
            if (node.m_specifiers[0] != NULL) { node.m_specifiers[0]->accept(*this); }
        }
        else
        {
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
    }
    printToken(";");
    printNewline();
}

// ================================================================
// visitExportDecl - pub ...
// ================================================================
void RustFormatter::visitExportDecl(ExportDeclNode& node)
{
    printToken("pub");
    if (node.m_declaration != NULL)
    {
        printSpace();
        node.m_declaration->accept(*this);
    }
    if (!node.m_source.empty())
    {
        printToken(" mod ");
        printToken(node.m_source.c_str());
        printToken(";");
        printNewline();
    }
}

// ================================================================
// visitPreprocessorDirective - #[attribute]
// ================================================================
void RustFormatter::visitPreprocessorDirective(PreprocessorDirectiveNode& node)
{
    printToken("#[");
    printToken(node.m_directive.c_str());
    printToken("]");
    printNewline();
}

// ================================================================
// visitNamespace - mod name { body }
// ================================================================
void RustFormatter::visitNamespace(NamespaceNode& node)
{
    printToken("mod");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printToken(";"); printNewline(); }
}

// ================================================================
// visitProperty - key: value
// ================================================================
void RustFormatter::visitProperty(PropertyNode& node)
{
    if (node.m_key != NULL) { node.m_key->accept(*this); }
    printToken(": ");
    if (node.m_value != NULL) { node.m_value->accept(*this); }
}

void RustFormatter::visitRawText(RawTextNode& node)
{
    printToken(node.m_text.c_str());
    printNewline();
}

void RustFormatter::visitHtmlElement(HtmlElementNode& node)
{
    size_t cc = node.m_children.size();
    for (size_t i = 0; i < cc; i++)
    {
        if (node.m_children[i] != NULL) { node.m_children[i]->accept(*this); }
        if (i < cc - 1) { printNewline(); }
    }
}

void RustFormatter::visitWith(WithNode& node)
{
    if (node.m_context != NULL) { node.m_context->accept(*this); }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

void RustFormatter::visitSwitchCase(SwitchCaseNode& node)
{
    if (node.m_pattern != NULL) { node.m_pattern->accept(*this); }
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    else { printNewline(); }
}

// Easter egg: RustFormatter complete - the borrow checker is satisfied... for now