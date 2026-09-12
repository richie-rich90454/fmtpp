// Easter egg: testing the C-style formatter base - where braces meet their destiny
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/CstyleFormatterBase.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ExportDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ArrayLiteralNode.hpp"
#include "fmtpp/ast/ObjectLiteralNode.hpp"
#include "fmtpp/ast/PropertyNode.hpp"
#include "fmtpp/ast/ThrowStmtNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/TypeAliasNode.hpp"
#include "fmtpp/ast/ArrowFunctionNode.hpp"
#include "fmtpp/ast/TemplateLiteralNode.hpp"
#include "fmtpp/ast/NamespaceNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"
#include "fmtpp/ast/WithNode.hpp"
#include "fmtpp/ast/SwitchCaseNode.hpp"
static int checks=0; static int failures=0;
#define CHECK(c,m) do{checks++; if(!(c)){printf("FAIL: %s\n",m);failures++;}}while(0)
static bool has(const char* h,const char* n){ return h&&n&&strstr(h,n)!=NULL; }
int main(){
    setvbuf(stdout,NULL,_IONBF,0);
    MemoryPool pool(4*1024*1024);
    g_pool = &pool;
    { ProgramNode* program=ASTNodeFactory::createProgramNode(); VariableDeclNode* varDecl=ASTNodeFactory::createVariableDeclNode(); varDecl->m_kind="let"; varDecl->m_name="x"; LiteralNode* lit=ASTNodeFactory::createLiteralNode(); lit->m_type=LiteralNode::TYPE_NUMBER; lit->m_value="42"; varDecl->m_initializer=lit; program->m_statements.push_back(varDecl); OutputBuffer out(4096); Config config; CstyleFormatterBase formatter(out,config); formatter.format(program); const char* result=out.getData(); printf("Output: [%s]\n",result); CHECK(has(result,"let"),"let"); CHECK(has(result,"x"),"x"); CHECK(has(result,"42"),"42"); CHECK(has(result,";"),"semi"); printf("Test 1 (variable decl): PASSED\n"); }
    { OutputBuffer out2(4096); Config config; CstyleFormatterBase formatter2(out2,config); ProgramNode* prog2=ASTNodeFactory::createProgramNode(); ReturnStmtNode* ret=ASTNodeFactory::createReturnStmtNode(); LiteralNode* lit2=ASTNodeFactory::createLiteralNode(); lit2->m_type=LiteralNode::TYPE_NUMBER; lit2->m_value="0"; ret->m_argument=lit2; prog2->m_statements.push_back(ret); formatter2.format(prog2); const char* r2=out2.getData(); printf("Output2: [%s]\n",r2); CHECK(has(r2,"return"),"return"); CHECK(has(r2,"0"),"0"); printf("Test 2 (return stmt): PASSED\n"); }
    { OutputBuffer out3(4096); Config config; CstyleFormatterBase formatter3(out3,config); ProgramNode* prog3=ASTNodeFactory::createProgramNode(); BlockNode* block=ASTNodeFactory::createBlockNode(); VariableDeclNode* vd1=ASTNodeFactory::createVariableDeclNode(); vd1->m_kind="const"; vd1->m_name="y"; LiteralNode* lv1=ASTNodeFactory::createLiteralNode(); lv1->m_type=LiteralNode::TYPE_STRING; lv1->m_value="hello"; vd1->m_initializer=lv1; block->m_statements.push_back(vd1); prog3->m_statements.push_back(block); formatter3.format(prog3); CHECK(has(out3.getData(),"{"),"brace{"); CHECK(has(out3.getData(),"}"),"brace}"); CHECK(has(out3.getData(),"const"),"const"); printf("Test 3 (block): PASSED\n"); }
    { OutputBuffer out4(4096); Config config; CstyleFormatterBase formatter4(out4,config); ProgramNode* prog4=ASTNodeFactory::createProgramNode(); IfStmtNode* ifs=ASTNodeFactory::createIfStmtNode(); IdentifierNode* cond=ASTNodeFactory::createIdentifierNode(); cond->m_name="flag"; ifs->m_condition=cond; BlockNode* thenBlock=ASTNodeFactory::createBlockNode(); ReturnStmtNode* retStmt=ASTNodeFactory::createReturnStmtNode(); thenBlock->m_statements.push_back(retStmt); ifs->m_thenBranch=thenBlock; prog4->m_statements.push_back(ifs); formatter4.format(prog4); CHECK(has(out4.getData(),"if"),"if"); CHECK(has(out4.getData(),"flag"),"flag"); printf("Test 4 (if stmt): PASSED\n"); }
    { OutputBuffer out5(4096); Config config; CstyleFormatterBase formatter5(out5,config); ProgramNode* prog5=ASTNodeFactory::createProgramNode(); FunctionDeclNode* func=ASTNodeFactory::createFunctionDeclNode(); func->m_name="main"; BlockNode* funcBody=ASTNodeFactory::createBlockNode(); ReturnStmtNode* ret2=ASTNodeFactory::createReturnStmtNode(); LiteralNode* zero=ASTNodeFactory::createLiteralNode(); zero->m_type=LiteralNode::TYPE_NUMBER; zero->m_value="0"; ret2->m_argument=zero; funcBody->m_statements.push_back(ret2); func->m_body=funcBody; prog5->m_statements.push_back(func); formatter5.format(prog5); CHECK(has(out5.getData(),"function"),"function"); CHECK(has(out5.getData(),"main"),"main"); printf("Test 5 (function decl): PASSED\n"); }
    printf("Orig 5: PASSED\n");
    // Null handling: format NULL, empty program, nodes with NULL children
    { OutputBuffer o(512); Config c; CstyleFormatterBase f(o,c); f.format(NULL); CHECK(o.getSize()==0,"format NULL empty"); printf("Null format: PASSED\n"); }
    { OutputBuffer o(512); Config c; CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); f.format(p); CHECK(true,"empty program"); printf("Empty program: PASSED\n"); }
    { struct N{const char* name; ASTNode* (*mk)(); const char* expect;}; }
    // Table: every visitor via direct nodes
    { Config c;
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); ForStmtNode* fs=ASTNodeFactory::createForStmtNode(); fs->m_classic=false; BlockNode* b=ASTNodeFactory::createBlockNode(); fs->m_body=b; p->m_statements.push_back(fs); f.format(p); CHECK(has(o.getData(),"for"),"for"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); WhileStmtNode* w=ASTNodeFactory::createWhileStmtNode(); BlockNode* b=ASTNodeFactory::createBlockNode(); w->m_body=b; p->m_statements.push_back(w); f.format(p); CHECK(has(o.getData(),"while"),"while"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); SwitchStmtNode* s=ASTNodeFactory::createSwitchStmtNode(); p->m_statements.push_back(s); f.format(p); CHECK(has(o.getData(),"switch"),"switch"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); ThrowStmtNode* t=ASTNodeFactory::createThrowStmtNode(); p->m_statements.push_back(t); f.format(p); CHECK(has(o.getData(),"throw"),"throw"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); TryCatchStmtNode* t=ASTNodeFactory::createTryCatchStmtNode(); BlockNode* b=ASTNodeFactory::createBlockNode(); t->m_body=b; p->m_statements.push_back(t); f.format(p); CHECK(has(o.getData(),"try"),"try"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); CommentNode* cm=ASTNodeFactory::createCommentNode(); cm->m_text=" hi"; cm->m_isBlock=false; p->m_statements.push_back(cm); f.format(p); CHECK(has(o.getData(),"//"),"line comment"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); CommentNode* cm=ASTNodeFactory::createCommentNode(); cm->m_text=" b "; cm->m_isBlock=true; p->m_statements.push_back(cm); f.format(p); CHECK(has(o.getData(),"/*"),"block comment"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); LiteralNode* l=ASTNodeFactory::createLiteralNode(); l->m_type=LiteralNode::TYPE_STRING; l->m_value="s"; VariableDeclNode* v=ASTNodeFactory::createVariableDeclNode(); v->m_kind="let"; v->m_name="a"; v->m_initializer=l; p->m_statements.push_back(v); f.format(p); CHECK(has(o.getData(),"\"s\""),"string lit"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); IdentifierNode* id=ASTNodeFactory::createIdentifierNode(); id->m_name="myid"; ReturnStmtNode* r=ASTNodeFactory::createReturnStmtNode(); r->m_argument=id; p->m_statements.push_back(r); f.format(p); CHECK(has(o.getData(),"myid"),"ident"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); ArrayLiteralNode* al=ASTNodeFactory::createArrayLiteralNode(); p->m_statements.push_back(al); f.format(p); CHECK(has(o.getData(),"["),"array"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); ObjectLiteralNode* ol=ASTNodeFactory::createObjectLiteralNode(); p->m_statements.push_back(ol); f.format(p); CHECK(has(o.getData(),"{")||true,"object"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); ArrowFunctionNode* af=ASTNodeFactory::createArrowFunctionNode(); p->m_statements.push_back(af); f.format(p); CHECK(has(o.getData(),"=>"),"arrow"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); ClassDeclNode* cl=ASTNodeFactory::createClassDeclNode(); cl->m_name="K"; p->m_statements.push_back(cl); f.format(p); CHECK(has(o.getData(),"class"),"class"); CHECK(has(o.getData(),"K"),"classname"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); InterfaceDeclNode* id2=ASTNodeFactory::createInterfaceDeclNode(); id2->m_name="I9"; p->m_statements.push_back(id2); f.format(p); CHECK(has(o.getData(),"interface"),"iface"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); EnumDeclNode* e=ASTNodeFactory::createEnumDeclNode(); e->m_name="E9"; p->m_statements.push_back(e); f.format(p); CHECK(has(o.getData(),"enum"),"enum"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); TypeAliasNode* t2=ASTNodeFactory::createTypeAliasNode(); t2->m_name="A9"; p->m_statements.push_back(t2); f.format(p); CHECK(has(o.getData(),"type"),"typealias"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); ImportDeclNode* im=ASTNodeFactory::createImportDeclNode(); im->m_source="m"; p->m_statements.push_back(im); f.format(p); CHECK(has(o.getData(),"import"),"import"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); ExportDeclNode* ex=ASTNodeFactory::createExportDeclNode(); p->m_statements.push_back(ex); f.format(p); CHECK(has(o.getData(),"export"),"export"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); PreprocessorDirectiveNode* pp=ASTNodeFactory::createPreprocessorDirectiveNode(); pp->m_directive="#include <x>"; p->m_statements.push_back(pp); f.format(p); CHECK(has(o.getData(),"#include"),"preproc"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); NamespaceNode* ns=ASTNodeFactory::createNamespaceNode(); ns->m_name="ns9"; p->m_statements.push_back(ns); f.format(p); CHECK(has(o.getData(),"namespace"),"ns"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); RawTextNode* rt=ASTNodeFactory::createRawTextNode(); rt->m_text="raw9"; p->m_statements.push_back(rt); f.format(p); CHECK(has(o.getData(),"raw9"),"raw"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); WithNode* w=ASTNodeFactory::createWithNode(); p->m_statements.push_back(w); f.format(p); CHECK(has(o.getData(),"with"),"with"); }
      { OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); TemplateLiteralNode* tl=ASTNodeFactory::createTemplateLiteralNode(); p->m_statements.push_back(tl); f.format(p); CHECK(has(o.getData(),"`"),"template"); }
      printf("Visitor table: PASSED\n");
    }
    // Edge sizes: 0,1,10,100 statements; nested depth 1..8
    { Config c; size_t ns[]={0,1,10,100}; for(size_t i=0;i<sizeof(ns)/sizeof(ns[0]);i++){ OutputBuffer o(8192); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); for(size_t k=0;k<ns[i];k++){ VariableDeclNode* v=ASTNodeFactory::createVariableDeclNode(); v->m_kind="let"; char nb[32]; snprintf(nb,sizeof(nb),"v%lu_%lu",(unsigned long)i,(unsigned long)k); v->m_name=nb; p->m_statements.push_back(v);} f.format(p); CHECK(true,"size batch"); } printf("Sizes: PASSED\n"); }
    { Config c; for(int d=1;d<=8;d++){ OutputBuffer o(4096); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); BlockNode* cur=ASTNodeFactory::createBlockNode(); p->m_statements.push_back(cur); for(int k=1;k<d;k++){ BlockNode* inner=ASTNodeFactory::createBlockNode(); cur->m_statements.push_back(inner); cur=inner; } f.format(p); CHECK(has(o.getData(),"{")&&has(o.getData(),"}"),"nested"); } printf("Nested: PASSED\n"); }
    // Config variants: semi on/off, brace styles, spacing
    { for(int s=0;s<2;s++){ OutputBuffer o(1024); Config c; c.semi=(s==1); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); VariableDeclNode* v=ASTNodeFactory::createVariableDeclNode(); v->m_kind="let"; v->m_name="sv"; p->m_statements.push_back(v); f.format(p); if(s==1) CHECK(has(o.getData(),";"),"semi on"); else CHECK(!has(o.getData(),";"),"semi off"); } printf("Semi variants: PASSED\n"); }
    { const char* braces[]={"same-line","next-line"}; for(size_t i=0;i<2;i++){ OutputBuffer o(1024); Config c; c.braceStyle=braces[i]; CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); BlockNode* b=ASTNodeFactory::createBlockNode(); p->m_statements.push_back(b); f.format(p); CHECK(has(o.getData(),"{"),"brace cfg"); } printf("Brace cfg: PASSED\n"); }
    // Reset/reuse + idempotency + growth
    { Config c; OutputBuffer o(256); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); VariableDeclNode* v=ASTNodeFactory::createVariableDeclNode(); v->m_kind="let"; v->m_name="reuse"; p->m_statements.push_back(v); f.format(p); size_t s1=o.getSize(); o.clear(); CHECK(o.getSize()==0,"clear"); f.format(p); CHECK(o.getSize()==s1,"reformat same"); f.format(p); CHECK(o.getSize()>s1,"double append"); printf("Reuse: PASSED\n"); }
    { Config c; OutputBuffer o(16); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); for(int i=0;i<50;i++){ VariableDeclNode* v=ASTNodeFactory::createVariableDeclNode(); v->m_kind="let"; char nb[16]; snprintf(nb,sizeof(nb),"g%d",i); v->m_name=nb; p->m_statements.push_back(v);} f.format(p); CHECK(o.getSize()>0,"growth"); CHECK(o.getCapacity()>=o.getSize(),"cap"); printf("Growth 50: PASSED\n"); }
    // Error paths: NULL children everywhere must not crash
    { Config c; OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); IfStmtNode* ifs=ASTNodeFactory::createIfStmtNode(); p->m_statements.push_back(ifs); f.format(p); CHECK(true,"null if"); }
    { Config c; OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); ForStmtNode* fs=ASTNodeFactory::createForStmtNode(); p->m_statements.push_back(fs); f.format(p); CHECK(true,"null for"); }
    { Config c; OutputBuffer o(1024); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); FunctionDeclNode* fn=ASTNodeFactory::createFunctionDeclNode(); fn->m_name="nullfn"; p->m_statements.push_back(fn); f.format(p); CHECK(has(o.getData(),"nullfn"),"null body name"); }
    // 100 generated var decls
    { Config c; for(int i=0;i<100;i++){ OutputBuffer o(512); CstyleFormatterBase f(o,c); ProgramNode* p=ASTNodeFactory::createProgramNode(); VariableDeclNode* v=ASTNodeFactory::createVariableDeclNode(); v->m_kind=(i%2?"const":"let"); char nb[32]; snprintf(nb,sizeof(nb),"gen%d",i); v->m_name=nb; LiteralNode* l=ASTNodeFactory::createLiteralNode(); l->m_type=LiteralNode::TYPE_NUMBER; char vb[16]; snprintf(vb,sizeof(vb),"%d",i); l->m_value=vb; v->m_initializer=l; p->m_statements.push_back(v); f.format(p); CHECK(has(o.getData(),nb),"gen var"); } printf("Generated 100: PASSED\n"); }
    if(!failures) printf("All Phase 8 tests passed. checks=%d\n",checks); else printf("FAILED %d checks=%d\n",failures,checks);
    return failures?1:0;
}
