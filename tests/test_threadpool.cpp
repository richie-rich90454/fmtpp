// Easter egg: thread pool tests - many threads, one dream
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include "fmtpp/ThreadPool.hpp"
#include "fmtpp/MemoryPool.hpp"
extern thread_local MemoryPool* g_pool;
static int checks=0; static int failures=0;
#define CHECK(c,m) do{checks++; if(!(c)){printf("FAIL: %s\n",m);failures++;}}while(0)
static void writeFile(const char* path, const char* content){ FILE* f=fopen(path,"w"); if(f){fwrite(content,1,strlen(content),f); fclose(f);} }
int main(){
    setvbuf(stdout,NULL,_IONBF,0);
    MemoryPool pool(2*1024*1024); g_pool=&pool;
    const char* fileA="test_threadpool_a.js"; const char* fileB="test_threadpool_b.js"; const char* fileC="test_threadpool_c.js";
    writeFile(fileA,"const a = 1;"); writeFile(fileB,"const b = 2;"); writeFile(fileC,"const c = 3;");
    { ThreadPool pool2(0); CHECK(pool2.getWorkerCount()!=0,"workers>0"); }
    { ThreadPool pool2(2); std::vector<std::string> files; files.push_back(fileA); files.push_back(fileB); files.push_back(fileC); std::vector<std::string> results=pool2.parallelFormat(files,"js"); CHECK(results.size()==files.size(),"count"); for(size_t i=0;i<results.size();i++) CHECK(!results[i].empty(),"non-empty"); }
    { ThreadPool pool2(2); std::vector<std::string> files; files.push_back(fileA); std::vector<std::string> results=pool2.parallelFormat(files,"js"); CHECK(results.size()==1,"single count"); CHECK(!results[0].empty(),"single non-empty"); }
    { ThreadPool pool2(2); std::vector<std::string> files; std::vector<std::string> results=pool2.parallelFormat(files,"js"); CHECK(results.empty(),"empty files"); }
    remove(fileA); remove(fileB); remove(fileC);
    printf("Orig 4: PASSED\n");
    // Worker counts table
    { size_t ws[]={0,1,2,4,8}; for(size_t i=0;i<sizeof(ws)/sizeof(ws[0]);i++){ ThreadPool p(ws[i]); CHECK(p.getWorkerCount()>0,"wc>0"); } printf("Workers: PASSED\n"); }
    // Recreate files with distinct markers for order test
    writeFile("test_tp_0.js","const marker0 = 0;"); writeFile("test_tp_1.js","const marker1 = 1;"); writeFile("test_tp_2.js","const marker2 = 2;"); writeFile("test_tp_3.js","const marker3 = 3;"); writeFile("test_tp_4.js","const marker4 = 4;");
    // Order preservation + concurrency breadth
    { ThreadPool p(4); std::vector<std::string> fs; for(int i=0;i<5;i++){ char fn[32]; snprintf(fn,sizeof(fn),"test_tp_%d.js",i); fs.push_back(fn);} std::vector<std::string> rs=p.parallelFormat(fs,"js"); CHECK(rs.size()==5,"order count"); for(int i=0;i<5;i++){ char mk[32]; snprintf(mk,sizeof(mk),"marker%d",i); CHECK(rs[i].find(mk)!=std::string::npos,"order marker"); } printf("Order: PASSED\n"); }
    // Edge sizes: 0,1,5,10,20 files
    { int ns[]={0,1,5,10}; for(size_t ni=0;ni<sizeof(ns)/sizeof(ns[0]);ni++){ int n=ns[ni]; std::vector<std::string> fs; for(int i=0;i<n;i++){ char fn[64]; snprintf(fn,sizeof(fn),"test_tp_e_%d_%d.js",(int)ni,i); writeFile(fn,"const e = 1;"); fs.push_back(fn);} ThreadPool p(2); std::vector<std::string> rs=p.parallelFormat(fs,"js"); char m[64]; snprintf(m,sizeof(m),"edge n=%d",n); CHECK(rs.size()==(size_t)n,m); for(size_t k=0;k<rs.size();k++) CHECK(!rs[k].empty(),"edge non-empty"); for(size_t k=0;k<fs.size();k++) remove(fs[k].c_str()); } printf("Edge sizes: PASSED\n"); }
    // Missing files error path
    { ThreadPool p(2); std::vector<std::string> fs; fs.push_back("definitely_missing_tp_12345.js"); std::vector<std::string> rs=p.parallelFormat(fs,"js"); CHECK(rs.size()==1,"miss count"); CHECK(rs[0].empty(),"miss empty"); printf("Missing: PASSED\n"); }
    // Mixed valid+missing
    { writeFile("test_tp_valid.js","const okv = 1;"); ThreadPool p(2); std::vector<std::string> fs; fs.push_back("test_tp_valid.js"); fs.push_back("definitely_missing_tp2.js"); std::vector<std::string> rs=p.parallelFormat(fs,"js"); CHECK(rs.size()==2,"mixed count"); CHECK(!rs[0].empty(),"mixed valid"); CHECK(rs[1].empty(),"mixed miss empty"); remove("test_tp_valid.js"); printf("Mixed: PASSED\n"); }
    // Reuse/idempotency: same pool twice
    { ThreadPool p(2); std::vector<std::string> fs; fs.push_back("test_tp_0.js"); fs.push_back("test_tp_1.js"); std::vector<std::string> r1=p.parallelFormat(fs,"js"); std::vector<std::string> r2=p.parallelFormat(fs,"js"); CHECK(r1.size()==r2.size(),"reuse size"); CHECK(r1[0]==r2[0]&&r1[1]==r2[1],"reuse same"); printf("Reuse: PASSED\n"); }
    // Languages table
    { writeFile("test_tp_lang.py","x_var_lang = 1\n"); writeFile("test_tp_lang2.java","class Clang2 {}\n"); ThreadPool p(2); std::vector<std::string> f1; f1.push_back("test_tp_lang.py"); std::vector<std::string> r1=p.parallelFormat(f1,"python"); CHECK(r1.size()==1&&!r1[0].empty(),"py lang"); std::vector<std::string> f2; f2.push_back("test_tp_lang2.java"); std::vector<std::string> r2=p.parallelFormat(f2,"java"); CHECK(r2.size()==1&&!r2[0].empty(),"java lang"); remove("test_tp_lang.py"); remove("test_tp_lang2.java"); printf("Langs: PASSED\n"); }
    // Growth: 20 files
    { std::vector<std::string> fs; for(int i=0;i<20;i++){ char fn[64]; snprintf(fn,sizeof(fn),"test_tp_g%d.js",i); char content[64]; snprintf(content,sizeof(content),"const g%d = %d;",i,i); writeFile(fn,content); fs.push_back(fn);} ThreadPool p(4); std::vector<std::string> rs=p.parallelFormat(fs,"js"); CHECK(rs.size()==20,"grow 20"); for(size_t i=0;i<rs.size();i++) CHECK(!rs[i].empty(),"grow non-empty"); for(size_t i=0;i<fs.size();i++) remove(fs[i].c_str()); printf("Growth 20: PASSED\n"); }
    // 60 generated single-file runs (idempotency breadth)
    { writeFile("test_tp_gen.js","const genv = 1;"); for(int i=0;i<60;i++){ ThreadPool p(2); std::vector<std::string> fs; fs.push_back("test_tp_gen.js"); std::vector<std::string> rs=p.parallelFormat(fs,"js"); CHECK(rs.size()==1&&!rs[0].empty(),"gen run"); CHECK(rs[0].find("genv")!=std::string::npos,"gen marker"); } remove("test_tp_gen.js"); printf("Generated 60: PASSED\n"); }
    for(int i=0;i<5;i++){ char fn[32]; snprintf(fn,sizeof(fn),"test_tp_%d.js",i); remove(fn); }
    if(!failures) printf("ThreadPool tests passed. checks=%d\n",checks); else printf("ThreadPool tests FAILED: %d checks=%d\n",failures,checks);
    return failures;
}
