#include <cstdio>
#include <cstring>
#include <vector>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
extern thread_local MemoryPool* g_pool;
static int checks = 0;
static int failures = 0;
#define CHECK(c, msg) do { checks++; if (!(c)) { printf("FAIL: %s\n", msg); failures++; } } while(0)
int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    { MemoryPool pool; CHECK(pool.getCapacity() == 10*1024*1024, "default 10MB"); CHECK(pool.getUsed()==0,"fresh 0"); printf("MemoryPool default capacity: PASSED\n"); }
    { MemoryPool pool(1024); g_pool=&pool; void* p1=pool.allocate(100); void* p2=pool.allocate(100); CHECK(p1&&p2,"bump"); CHECK(p2>p1,"advance"); g_pool=NULL; printf("MemoryPool bump allocation: PASSED\n"); }
    { MemoryPool pool(64); void* p=pool.allocate(128); CHECK(p==NULL,"over-alloc NULL"); printf("MemoryPool exhaustion: PASSED\n"); }
    { MemoryPool pool; g_pool=&pool; ProgramNode* program=ASTNodeFactory::createProgramNode(); CHECK(program,"bulk program"); if(program){ for(int i=0;i<1000;i++){ VariableDeclNode* var=ASTNodeFactory::createVariableDeclNode(); LiteralNode* lit=ASTNodeFactory::createLiteralNode(); if(!var||!lit){CHECK(false,"bulk");break;} var->m_name="x"; var->m_initializer=lit; program->m_statements.push_back(var);} CHECK(program->m_statements.size()==1000,"bulk 1000"); } g_pool=NULL; printf("Bulk AST (1000): PASSED\n"); }
    { MemoryPool pool(1024); g_pool=&pool; void* p1=pool.allocate(100); CHECK(p1,"alloc before reset"); pool.reset(); CHECK(pool.getUsed()==0,"reset 0"); g_pool=NULL; printf("MemoryPool reset: PASSED\n"); }
    { size_t sizes[]={0,1,7,8,9,15,16,100,1024,2048}; for(size_t i=0;i<sizeof(sizes)/sizeof(sizes[0]);i++){ MemoryPool pool(4096); void* p=pool.allocate(sizes[i]); CHECK(p!=NULL,"edge alloc"); CHECK(pool.getUsed()%8==0,"aligned"); } printf("Edge sizes table: PASSED\n"); }
    { MemoryPool pool(64); void* a=pool.allocate(32); void* b=pool.allocate(32); CHECK(a&&b,"exact fill"); void* c=pool.allocate(1); CHECK(c==NULL,"one past NULL"); printf("Exact fill: PASSED\n"); }
    { MemoryPool pool(256); size_t u0=pool.getUsed(); void* p0=pool.allocate(0); CHECK(p0!=NULL,"alloc0"); CHECK(pool.getUsed()==u0,"alloc0 no advance"); void* p1=pool.allocate(1); void* p2=pool.allocate(1); CHECK(p1&&p2,"tiny"); CHECK((char*)p2-(char*)p1==8,"align stride"); printf("Null/align: PASSED\n"); }
    { MemoryPool pool(64*1024); g_pool=&pool; for(int cycle=0;cycle<10;cycle++){ for(int i=0;i<20;i++){ VariableDeclNode* v=ASTNodeFactory::createVariableDeclNode(); CHECK(v,"reuse"); if(!v)break; } pool.reset(); CHECK(pool.getUsed()==0,"cycle reset"); } pool.reset(); pool.reset(); CHECK(pool.getUsed()==0,"double reset"); void* p=pool.allocate(16); CHECK(p,"after reset"); g_pool=NULL; printf("Reset/reuse cycles: PASSED\n"); }
    { size_t reqs[]={1,16,64,65,128,1024}; for(size_t i=0;i<sizeof(reqs)/sizeof(reqs[0]);i++){ MemoryPool pool(64); pool.allocate(64); void* p=pool.allocate(reqs[i]); CHECK(p==NULL,"exhausted NULL"); } printf("Exhaustion table: PASSED\n"); }
    { size_t caps[]={1,8,64,512,4096,65536,1024*1024}; for(size_t i=0;i<sizeof(caps)/sizeof(caps[0]);i++){ MemoryPool pool(caps[i]); CHECK(pool.getCapacity()==caps[i],"cap"); CHECK(pool.getUsed()==0,"fresh"); } printf("Capacities table: PASSED\n"); }
    { MemoryPool a(1024),b(1024); void* pa=a.allocate(100); void* pb=b.allocate(100); CHECK(pa&&pb,"two pools"); CHECK(a.getUsed()==b.getUsed(),"independent"); a.reset(); CHECK(a.getUsed()==0&&b.getUsed()!=0,"isolated"); printf("Multi-pool isolation: PASSED\n"); }
    { MemoryPool pool(1024*1024); g_pool=&pool; int made=0; for(int i=0;i<5000;i++){ VariableDeclNode* v=ASTNodeFactory::createVariableDeclNode(); LiteralNode* l=ASTNodeFactory::createLiteralNode(); if(!v||!l)break; v->m_initializer=l; made++; } CHECK(made==5000,"bulk5000"); g_pool=NULL; printf("Bulk 5000: PASSED\n"); }
    { MemoryPool pool(1024); void* p1=pool.allocate(16); void* p2=pool.allocate(16); CHECK(p1!=p2,"distinct"); CHECK(p1&&p2,"non-null"); printf("Distinct pointers: PASSED\n"); }
    // growth via sequential allocs to near capacity
    { MemoryPool pool(4096); int n=0; while(pool.allocate(32)!=NULL) n++; CHECK(n>0,"growth count"); CHECK(pool.allocate(1)==NULL,"growth exhausted"); pool.reset(); CHECK(pool.getUsed()==0,"growth reset"); CHECK(pool.allocate(32)!=NULL,"growth reuse"); printf("Growth/reuse: PASSED n=%d\n", n); }
    // idempotency: double reset + double alloc pattern
    { MemoryPool pool(512); g_pool=&pool; ProgramNode* p1=ASTNodeFactory::createProgramNode(); CHECK(p1,"idemp prog"); pool.reset(); ProgramNode* p2=ASTNodeFactory::createProgramNode(); CHECK(p2,"idemp prog2"); CHECK(p2->m_statements.empty(),"fresh empty"); g_pool=NULL; printf("Idempotency: PASSED\n"); }
    if(!failures) printf("All MemoryPool and ASTNode tests passed. checks=%d\n",checks); else printf("FAILED %d checks=%d\n",failures,checks);
    return failures;
}
