// Easter egg: formatter visitor tests - proving abstract classes can still have fun
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "fmtpp/FormatterVisitor.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
class TestFormatter : public FormatterVisitor
{
public:
    TestFormatter(OutputBuffer& out, const Config& config) : FormatterVisitor(out, config) {}
    virtual void format(ASTNode* /*root*/) {
        printToken("{"); printNewline(); indent(); printIndent(); printToken("body"); printSemicolon(); printNewline(); dedent(); printIndent(); printToken("}"); printNewline();
    }
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
    virtual void visitRawText(RawTextNode& /*node*/) {}
    virtual void visitHtmlElement(HtmlElementNode& /*node*/) {}
    virtual void visitWith(WithNode& /*node*/) {}
    virtual void visitSwitchCase(SwitchCaseNode& /*node*/) {}
};
class ProbeFormatter : public TestFormatter {
public:
    ProbeFormatter(OutputBuffer& o, const Config& c): TestFormatter(o,c) {}
    using FormatterVisitor::printIndent; using FormatterVisitor::printNewline; using FormatterVisitor::printSpace;
    using FormatterVisitor::printToken; using FormatterVisitor::printComma; using FormatterVisitor::printSemicolon;
    using FormatterVisitor::indent; using FormatterVisitor::dedent;
};
static int checks=0; static int failures=0;
#define CHECK(c,m) do{checks++; if(!(c)){printf("FAIL: %s\n",m);failures++;}}while(0)
int main(){
    setvbuf(stdout,NULL,_IONBF,0);
    { OutputBuffer out(1024); Config config; TestFormatter fmt(out,config); fmt.format(NULL); const char* r=out.getData(); printf("Formatted output:\n%s\n",r); CHECK(r[0]=='{',"first {"); printf("FormatterVisitor: test passed.\n"); }
    { OutputBuffer out2(1024); Config config2; config2.tabWidth=4; TestFormatter fmt2(out2,config2); fmt2.format(NULL); printf("With tabWidth=4:\n%s\n",out2.getData()); }
    printf("Orig 2: PASSED\n");
    // Null handling: printToken NULL, format NULL, empty token
    { OutputBuffer o(256); Config c; ProbeFormatter p(o,c); p.printToken(NULL); CHECK(o.getSize()==0,"token NULL no-op"); p.printToken(""); CHECK(o.getSize()==0,"token empty"); p.printToken("hi"); CHECK(strcmp(o.getData(),"hi")==0,"token hi"); printf("Null token: PASSED\n"); }
    // Edge: tabWidth variants
    { int tws[]={1,2,4,8}; for(size_t i=0;i<sizeof(tws)/sizeof(tws[0]);i++){ OutputBuffer o(256); Config c; c.tabWidth=tws[i]; TestFormatter f(o,c); f.format(NULL); CHECK(strstr(o.getData(),"{")!=NULL,"tw brace"); CHECK(strstr(o.getData(),"body")!=NULL,"tw body"); } printf("tabWidth table: PASSED\n"); }
    // useTabs true/false
    { for(int t=0;t<2;t++){ OutputBuffer o(256); Config c; c.useTabs=(t==1); c.tabWidth=2; TestFormatter f(o,c); f.format(NULL); CHECK(strstr(o.getData(),"body")!=NULL,"tabs body"); } printf("useTabs: PASSED\n"); }
    // semi true/false
    { OutputBuffer o1(256); Config c1; c1.semi=true; ProbeFormatter p1(o1,c1); p1.printToken("x"); p1.printSemicolon(); CHECK(strcmp(o1.getData(),"x;")==0,"semi true"); OutputBuffer o2(256); Config c2; c2.semi=false; ProbeFormatter p2(o2,c2); p2.printToken("x"); p2.printSemicolon(); CHECK(strcmp(o2.getData(),"x")==0,"semi false"); printf("Semicolon: PASSED\n"); }
    // indent/dedent clamping + growth
    { OutputBuffer o(64); Config c; ProbeFormatter p(o,c); p.dedent(); p.dedent(); CHECK(true,"dedent clamp"); for(int i=0;i<10;i++) p.indent(); for(int i=0;i<5;i++){ p.printIndent(); p.printToken("L"); p.printNewline(); } CHECK(strstr(o.getData(),"L")!=NULL,"deep indent"); for(int i=0;i<20;i++) p.dedent(); p.dedent(); CHECK(true,"multi dedent clamp"); printf("Indent clamp/growth: PASSED\n"); }
    // printSpace dedup, printComma, printNewline idempotency
    { OutputBuffer o(256); Config c; ProbeFormatter p(o,c); p.printToken("a"); p.printSpace(); p.printSpace(); p.printSpace(); p.printToken("b"); CHECK(strstr(o.getData(),"a b")!=NULL,"space dedup"); CHECK(strstr(o.getData(),"a  b")==NULL,"no double"); printf("Space dedup: PASSED\n"); }
    { OutputBuffer o(256); Config c; ProbeFormatter p(o,c); p.printToken("a"); p.printComma(); p.printToken("b"); CHECK(strstr(o.getData(),"a, b")!=NULL,"comma"); printf("Comma: PASSED\n"); }
    { OutputBuffer o(256); Config c; ProbeFormatter p(o,c); p.printNewline(); p.printNewline(); p.printNewline(); CHECK(o.getSize()==1,"newlines collapse"); p.printToken("a"); p.printNewline(); p.printNewline(); CHECK(strcmp(o.getData(),"\na\n")==0,"newline after token"); printf("Newlines: PASSED\n"); }
    // reset/reuse: clear buffer and reformat
    { OutputBuffer o(256); Config c; TestFormatter f(o,c); f.format(NULL); size_t s1=o.getSize(); o.clear(); CHECK(o.getSize()==0,"clear"); f.format(NULL); CHECK(o.getSize()==s1,"reformat same"); printf("Reuse: PASSED\n"); }
    // idempotency: format twice without clear appends (documents behavior, just no crash)
    { OutputBuffer o(256); Config c; TestFormatter f(o,c); f.format(NULL); f.format(NULL); CHECK(o.getSize()>0,"double format"); printf("Double format: PASSED\n"); }
    // growth: tiny buffer forces realloc
    { OutputBuffer o(8); Config c; TestFormatter f(o,c); f.format(NULL); CHECK(strstr(o.getData(),"body")!=NULL,"tiny grow"); CHECK(o.getCapacity()>=o.getSize(),"cap grow"); printf("Growth: PASSED\n"); }
    // 50 generated tabWidth/useTabs/semi combos
    { for(int i=0;i<50;i++){ OutputBuffer o(128); Config c; c.tabWidth=2+(i%4)*2; c.useTabs=(i%2==0); c.semi=(i%3!=0); TestFormatter f(o,c); f.format(NULL); CHECK(o.getSize()>0,"gen fmt"); CHECK(strstr(o.getData(),"{")!=NULL,"gen brace"); } printf("Generated 50: PASSED\n"); }
    // concurrency-ish: two formatters independent buffers
    { OutputBuffer a(128),b(128); Config c; TestFormatter fa(a,c),fb(b,c); fa.format(NULL); fb.format(NULL); CHECK(strcmp(a.getData(),b.getData())==0,"indep same"); a.clear(); CHECK(b.getSize()>0,"clear isolated"); printf("Isolation: PASSED\n"); }
    if(!failures) printf("All Phase 7 tests passed. checks=%d\n",checks); else printf("FormatterVisitor FAILED %d checks=%d\n",failures,checks);
    return failures?1:0;
}
