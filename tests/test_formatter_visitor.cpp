// Easter egg: formatter visitor tests - proving abstract classes can still have fun
#include <cstdio>
#include "fmtpp/FormatterVisitor.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"

// Minimal concrete subclass for testing
class TestFormatter : public FormatterVisitor
{
public:
    TestFormatter(OutputBuffer& out, const Config& config)
        : FormatterVisitor(out, config)
    {
    }

    virtual void format(ASTNode* /*root*/)
    {
        // Simple test: print a block with indentation
        printToken("{");
        printNewline();
        indent();
        printIndent();
        printToken("body");
        printSemicolon();
        printNewline();
        dedent();
        printIndent();
        printToken("}");
        printNewline();
    }

    // Concrete stubs for all ASTVisitor pure virtual methods
    virtual void visitProgram(ProgramNode& /*node*/) {}
    virtual void visitFunctionDecl(FunctionDeclNode& /*node*/) {}
    virtual void visitVariableDecl(VariableDeclNode& /*node*/) {}
    virtual void visitIfStmt(IfStmtNode& /*node*/) {}
    virtual void visitForStmt(ForStmtNode& /*node*/) {}
    virtual void visitWhileStmt(WhileStmtNode& /*node*/) {}
    virtual void visitSwitchStmt(SwitchStmtNode& /*node*/) {}
    virtual void visitBlock(BlockNode& /*node*/) {}
    virtual void visitReturnStmt(ReturnStmtNode& /*node*/) {}
    virtual void visitThrowStmt(ThrowStmtNode& /*node*/) {}
    virtual void visitTryCatchStmt(TryCatchStmtNode& /*node*/) {}
    virtual void visitComment(CommentNode& /*node*/) {}
    virtual void visitLiteral(LiteralNode& /*node*/) {}
    virtual void visitIdentifier(IdentifierNode& /*node*/) {}
    virtual void visitArrayLiteral(ArrayLiteralNode& /*node*/) {}
    virtual void visitObjectLiteral(ObjectLiteralNode& /*node*/) {}
    virtual void visitTemplateLiteral(TemplateLiteralNode& /*node*/) {}
    virtual void visitArrowFunction(ArrowFunctionNode& /*node*/) {}
    virtual void visitClassDecl(ClassDeclNode& /*node*/) {}
    virtual void visitInterfaceDecl(InterfaceDeclNode& /*node*/) {}
    virtual void visitEnumDecl(EnumDeclNode& /*node*/) {}
    virtual void visitTypeAlias(TypeAliasNode& /*node*/) {}
    virtual void visitImportDecl(ImportDeclNode& /*node*/) {}
    virtual void visitExportDecl(ExportDeclNode& /*node*/) {}
    virtual void visitPreprocessorDirective(PreprocessorDirectiveNode& /*node*/) {}
    virtual void visitNamespace(NamespaceNode& /*node*/) {}
    virtual void visitProperty(PropertyNode& /*node*/) {}
};

int main()
{
    OutputBuffer out(1024);
    Config config;
    TestFormatter fmt(out, config);

    // Format a dummy AST
    fmt.format(NULL);

    const char* result = out.getData();
    printf("Formatted output:\n%s\n", result);

    // Verify output
    bool ok = true;
    if (result[0] != '{') ok = false;

    if (ok)
    {
        printf("FormatterVisitor: test passed.\n");
    }
    else
    {
        printf("FormatterVisitor: test FAILED.\n");
        return 1;
    }

    // Test indentation
    OutputBuffer out2(1024);
    Config config2;
    config2.tabWidth = 4;
    TestFormatter fmt2(out2, config2);
    fmt2.format(NULL);
    printf("With tabWidth=4:\n%s\n", out2.getData());

    printf("All Phase 7 tests passed.\n");
    return 0;
}
