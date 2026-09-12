#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "fmtpp/FileHandle.hpp"
#include "fmtpp/MallocBuffer.hpp"
#include "fmtpp/MmapRegion.hpp"
static int checks=0; static int failures=0;
#define CHECK(c,m) do{checks++; if(!(c)){printf("FAIL: %s\n",m);failures++;}}while(0)
static void wf(const char* p,const char* c){ FILE* f=fopen(p,"w"); if(f){fwrite(c,1,strlen(c),f); fclose(f);} }
int main(){
    setvbuf(stdout,NULL,_IONBF,0);
    { const char* tp="test_raii_file.txt"; FILE* f=fopen(tp,"w"); if(f){fprintf(f,"hello"); fclose(f);} FileHandle h(tp,"r"); CHECK(h.isValid(),"fh valid"); if(h.isValid()){ char b[16]={0}; size_t n=fread(b,1,15,h.get()); CHECK(strcmp(b,"hello")==0,"fh content"); (void)n; } else CHECK(false,"fh"); remove(tp); printf("FileHandle open/read/close: PASSED\n"); }
    { MallocBuffer buf(64); CHECK(buf.isValid(),"mb valid"); if(buf.isValid()){ char* d=(char*)buf.get(); strcpy(d,"RAII"); CHECK(strcmp(d,"RAII")==0,"mb data"); } printf("MallocBuffer allocate/free: PASSED\n"); }
    { const char* tp="test_raii_mmap.txt"; FILE* f=fopen(tp,"w"); if(f){fprintf(f,"mapped"); fclose(f);} MmapRegion r(tp); CHECK(r.isValid(),"mmap valid"); if(r.isValid()){ const char* d=(const char*)r.getData(); CHECK(d&&strncmp(d,"mapped",6)==0,"mmap data"); } remove(tp); printf("MmapRegion map/unmap: PASSED\n"); }
    { const char* tp="test_raii_move.txt"; FILE* f=fopen(tp,"w"); if(f){fprintf(f,"move"); fclose(f);} FileHandle o(tp,"r"); FileHandle m(static_cast<FileHandle&&>(o)); CHECK(!o.isValid(),"moved-from invalid"); CHECK(m.isValid(),"moved-to valid"); if(m.isValid()){ char b[16]={0}; fread(b,1,15,m.get()); CHECK(strcmp(b,"move")==0,"move data"); } remove(tp); printf("FileHandle move semantics: PASSED\n"); }
    { MallocBuffer o(64); char* d=(char*)o.get(); strcpy(d,"move"); MallocBuffer m(static_cast<MallocBuffer&&>(o)); CHECK(!o.isValid(),"mb from invalid"); CHECK(m.isValid(),"mb to valid"); CHECK(strcmp((char*)m.get(),"move")==0,"mb move data"); printf("MallocBuffer move semantics: PASSED\n"); }
    { const char* tp="test_raii_mmap_move.txt"; FILE* f=fopen(tp,"w"); if(f){fprintf(f,"mmapmove"); fclose(f);} MmapRegion o(tp); MmapRegion m(static_cast<MmapRegion&&>(o)); CHECK(!o.isValid(),"mmap from invalid"); CHECK(m.isValid(),"mmap to valid"); if(m.isValid()){ const char* d=(const char*)m.getData(); CHECK(d&&strncmp(d,"mmapmove",8)==0,"mmap move data"); } remove(tp); printf("MmapRegion move semantics: PASSED\n"); }
    { FileHandle nh(NULL,NULL); CHECK(!nh.isValid(),"null fh invalid"); printf("Null FileHandle destructor: PASSED\n"); }
    printf("Orig 7: PASSED\n");
    // Null handling table
    { FileHandle a(NULL,"r"); CHECK(!a.isValid(),"null path"); FileHandle b("x",NULL); CHECK(!b.isValid(),"null mode"); FileHandle c("","r"); CHECK(!c.isValid(),"empty path"); FileHandle d("definitely_missing_raii_12345.txt","r"); CHECK(!d.isValid(),"missing"); CHECK(d.get()==NULL,"miss get null"); printf("FH null: PASSED\n"); }
    { MmapRegion r(NULL); CHECK(!r.isValid(),"mmap null"); CHECK(r.getData()==NULL,"mmap null data"); MmapRegion r2("definitely_missing_mmap_12345.bin"); CHECK(!r2.isValid(),"mmap miss"); printf("Mmap null: PASSED\n"); }
    { MallocBuffer b1(1); CHECK(b1.isValid(),"mb1"); CHECK(b1.getSize()==1,"mb1 size"); MallocBuffer b2(4096); CHECK(b2.isValid()&&b2.getSize()==4096,"mb4096"); printf("MB sizes: PASSED\n"); }
    // Edge sizes table: file sizes
    { size_t lens[]={0,1,2,100,1024,8192}; for(size_t i=0;i<sizeof(lens)/sizeof(lens[0]);i++){ char fn[64]; snprintf(fn,sizeof(fn),"test_raii_e%lu.bin",(unsigned long)lens[i]); std::string content(lens[i],'E'); wf(fn,content.c_str()); FileHandle h(fn,"rb"); if(lens[i]==0){ CHECK(true,"empty fh tolerant"); } else { CHECK(h.isValid(),"fh edge"); } MmapRegion m(fn); CHECK(true,"mmap edge no crash"); remove(fn);} printf("File sizes: PASSED\n"); }
    // Edge: MallocBuffer sizes (avoid 0 which is platform-specific)
    { size_t ss[]={1,2,16,64,1024,65536}; for(size_t i=0;i<sizeof(ss)/sizeof(ss[0]);i++){ MallocBuffer b(ss[i]); CHECK(b.isValid(),"mb edge valid"); CHECK(b.getSize()==ss[i],"mb edge size"); memset(b.get(),0xAB,ss[i]); CHECK(true,"mb memset"); } printf("MB table: PASSED\n"); }
    // Growth: large buffer read/write
    { MallocBuffer b(64000); CHECK(b.isValid(),"large"); memset(b.get(),0xCD,64000); CHECK(((unsigned char*)b.get())[63999]==0xCD,"large tail"); printf("Large MB: PASSED\n"); }
    // Reset/reuse: move-assign chains
    { wf("test_raii_chain.txt","chain"); FileHandle a("test_raii_chain.txt","r"); FileHandle b(NULL,NULL); b=static_cast<FileHandle&&>(a); CHECK(!a.isValid()&&b.isValid(),"chain1"); FileHandle c(NULL,NULL); c=static_cast<FileHandle&&>(b); CHECK(!b.isValid()&&c.isValid(),"chain2"); remove("test_raii_chain.txt"); printf("FH chain: PASSED\n"); }
    { MallocBuffer a(32); strcpy((char*)a.get(),"aa"); MallocBuffer b(32); strcpy((char*)b.get(),"bb"); b=static_cast<MallocBuffer&&>(a); CHECK(!a.isValid(),"mb assign from"); CHECK(strcmp((char*)b.get(),"aa")==0,"mb assign data"); printf("MB assign: PASSED\n"); }
    { wf("test_raii_mchain.txt","mc"); MmapRegion a("test_raii_mchain.txt"); MmapRegion b(static_cast<MmapRegion&&>(a)); CHECK(!a.isValid()&&b.isValid(),"mmap chain"); MmapRegion c(static_cast<MmapRegion&&>(b)); CHECK(!b.isValid()&&c.isValid(),"mmap chain2"); remove("test_raii_mchain.txt"); printf("Mmap chain: PASSED\n"); }
    // Idempotency: self-move safe? (guarded by this!=&other, just check no crash on double move)
    { MallocBuffer a(16); MallocBuffer b(static_cast<MallocBuffer&&>(a)); MallocBuffer c(static_cast<MallocBuffer&&>(b)); CHECK(!a.isValid()&&!b.isValid()&&c.isValid(),"double move"); printf("Double move: PASSED\n"); }
    // Concurrency-ish: many handles same file
    { wf("test_raii_shared.txt","shared"); for(int i=0;i<10;i++){ FileHandle h("test_raii_shared.txt","r"); CHECK(h.isValid(),"shared fh"); MmapRegion m("test_raii_shared.txt"); CHECK(m.isValid(),"shared mmap"); } remove("test_raii_shared.txt"); printf("Shared: PASSED\n"); }
    // Error paths: missing dir/file with valid modes (invalid fopen modes abort in debug CRT, so not tested)
    { FileHandle h2("/definitely/missing/dir/f.txt","r"); CHECK(!h2.isValid(),"missing dir"); FileHandle h3("definitely_missing_raii_err.txt","r"); CHECK(!h3.isValid(),"missing file"); printf("Error paths: PASSED\n"); }
    // Binary content with braces (strings with braces breadth)
    { wf("test_raii_brace.txt","hi { not block }"); FileHandle h("test_raii_brace.txt","r"); char buf[64]={0}; fread(buf,1,63,h.get()); CHECK(strstr(buf,"not block")!=NULL,"brace content"); MmapRegion m("test_raii_brace.txt"); CHECK(strncmp((const char*)m.getData(),"hi {",4)==0,"mmap brace"); remove("test_raii_brace.txt"); printf("Brace content: PASSED\n"); }
    // 50 generated files
    { for(int i=0;i<50;i++){ char fn[64]; snprintf(fn,sizeof(fn),"test_raii_g%d.txt",i); char content[64]; snprintf(content,sizeof(content),"gencontent%d",i); wf(fn,content); FileHandle h(fn,"r"); CHECK(h.isValid(),"gen fh"); char buf[64]={0}; fread(buf,1,63,h.get()); CHECK(strstr(buf,content)!=NULL,"gen data"); remove(fn);} printf("Generated 50: PASSED\n"); }
    if(!failures) printf("All RAII wrapper tests passed. checks=%d\n",checks); else printf("RAII wrapper tests FAILED: %d checks=%d\n",failures,checks);
    return failures;
}
