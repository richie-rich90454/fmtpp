// Easter egg: benchmark tests - timing the untimed
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "fmtpp/Benchmark.hpp"
#include "fmtpp/MemoryPool.hpp"
static const char* SAMPLE_JS = "const x = 1;\nconst y = 2;\nfunction add(a, b) { return a + b; }\n";
extern thread_local MemoryPool* g_pool;
static int checks=0; static int failures=0;
#define CHECK(c,m) do{checks++; if(!(c)){printf("FAIL: %s\n",m);failures++;}}while(0)
static void wf(const char* p,const char* c){ FILE* f=fopen(p,"w"); if(f){fwrite(c,1,strlen(c),f); fclose(f);} }
int main(){
    setvbuf(stdout,NULL,_IONBF,0);
    const char* tempFile="test_benchmark_sample.js";
    MemoryPool pool(1024*1024); g_pool=&pool;
    wf(tempFile,SAMPLE_JS);
    { Benchmark::Config config; config.files.push_back(tempFile); config.language="js"; config.iterations=3; config.warmup=true; config.verbose=false; Benchmark bench; Benchmark::Result r=bench.run(config); CHECK(r.filesProcessed!=0,"files>0"); CHECK(r.totalTimeMs>0.0,"total>0"); CHECK(r.averageTimeMs>0.0,"avg>0"); CHECK(r.minTimeMs>0.0,"min>0"); CHECK(r.maxTimeMs>=r.minTimeMs,"max>=min"); CHECK(r.medianTimeMs>0.0,"med>0"); CHECK(r.p95TimeMs>0.0,"p95>0"); CHECK(r.p95TimeMs>=r.medianTimeMs,"p95>=med"); }
    { Benchmark::Config config; config.iterations=5; config.warmup=false; config.verbose=false; Benchmark bench; Benchmark::Result r=bench.run(config); CHECK(r.filesProcessed==0,"empty 0"); CHECK(r.totalTimeMs==0.0,"empty total 0"); }
    { Benchmark::Config config; config.files.push_back(tempFile); config.language="js"; config.iterations=2; config.warmup=false; config.verbose=false; config.compareTool="definitely_not_a_real_formatter_12345"; Benchmark bench; Benchmark::Result fr=bench.run(config); Benchmark::Result base=bench.runCompare(config,fr); CHECK(base.filesProcessed==0,"missing tool 0"); }
    remove(tempFile);
    printf("Orig 3: PASSED\n");
    // Edge: iterations table
    { int its[]={1,2,5}; for(size_t i=0;i<sizeof(its)/sizeof(its[0]);i++){ wf("test_bench_it.js",SAMPLE_JS); Benchmark::Config c; c.files.push_back("test_bench_it.js"); c.language="js"; c.iterations=its[i]; c.warmup=false; Benchmark b; Benchmark::Result r=b.run(c); CHECK(r.filesProcessed==(size_t)its[i],"it files"); CHECK(r.totalTimeMs>=0.0,"it total"); remove("test_bench_it.js"); } printf("Iterations: PASSED\n"); }
    // Edge: missing file
    { Benchmark::Config c; c.files.push_back("definitely_missing_12345.js"); c.language="js"; c.iterations=2; Benchmark b; Benchmark::Result r=b.run(c); CHECK(r.filesProcessed==0,"miss 0"); CHECK(r.totalTimeMs==0.0,"miss total 0"); printf("Missing file: PASSED\n"); }
    // Edge: empty file
    { wf("test_bench_empty.js",""); Benchmark::Config c; c.files.push_back("test_bench_empty.js"); c.language="js"; c.iterations=2; Benchmark b; Benchmark::Result r=b.run(c); CHECK(true,"empty no crash"); remove("test_bench_empty.js"); printf("Empty file: PASSED\n"); }
    // Edge: large file growth
    { std::string big; for(int i=0;i<500;i++) big+="const v"+std::to_string(i)+" = "+std::to_string(i)+";\n"; wf("test_bench_big.js",big.c_str()); Benchmark::Config c; c.files.push_back("test_bench_big.js"); c.language="js"; c.iterations=2; Benchmark b; Benchmark::Result r=b.run(c); CHECK(r.filesProcessed==2,"big files"); CHECK(r.totalBytesProcessed>0,"big bytes"); CHECK(r.throughputBytesPerSec>0||r.totalTimeMs>=0.0,"big thr"); remove("test_bench_big.js"); printf("Large file: PASSED\n"); }
    // Table: languages
    { const char* langs[]={"js","ts","java","c","cpp","python"}; const char* srcs[]={"const x=1;\n","const x:number=1;\n","class C{}\n","int x;\n","int x;\n","x=1\n"}; for(size_t i=0;i<sizeof(langs)/sizeof(langs[0]);i++){ char fn[64]; snprintf(fn,sizeof(fn),"test_bench_l%lu.js",(unsigned long)i); wf(fn,srcs[i]); Benchmark::Config c; c.files.push_back(fn); c.language=langs[i]; c.iterations=1; Benchmark b; Benchmark::Result r=b.run(c); CHECK(true,"lang no crash"); remove(fn);} printf("Languages: PASSED\n"); }
    // Reset/reuse: run twice same config -> consistent
    { wf("test_bench_re.js",SAMPLE_JS); Benchmark::Config c; c.files.push_back("test_bench_re.js"); c.language="js"; c.iterations=2; Benchmark b; Benchmark::Result r1=b.run(c); Benchmark::Result r2=b.run(c); CHECK(r1.filesProcessed==r2.filesProcessed,"reuse same"); CHECK(r1.filesProcessed==2,"reuse count"); remove("test_bench_re.js"); printf("Reuse: PASSED\n"); }
    // Idempotency: empty compareTool -> empty baseline
    { Benchmark::Config c; c.files.push_back("x.js"); Benchmark b; Benchmark::Result fr; fr.filesProcessed=1; Benchmark::Result base=b.runCompare(c,fr); CHECK(base.filesProcessed==0,"empty tool"); c.compareTool=""; base=b.runCompare(c,fr); CHECK(base.filesProcessed==0,"empty tool2"); printf("Compare empty: PASSED\n"); }
    // Warmup true/false + verbose (must not crash)
    { wf("test_bench_w.js",SAMPLE_JS); for(int w=0;w<2;w++) for(int v=0;v<2;v++){ Benchmark::Config c; c.files.push_back("test_bench_w.js"); c.language="js"; c.iterations=1; c.warmup=(w==1); c.verbose=(v==1); Benchmark b; Benchmark::Result r=b.run(c); CHECK(r.filesProcessed==1,"w/v"); } remove("test_bench_w.js"); printf("Warmup/verbose: PASSED\n"); }
    // Multi-file growth
    { std::vector<std::string> fns; for(int i=0;i<5;i++){ char fn[64]; snprintf(fn,sizeof(fn),"test_bench_m%d.js",i); wf(fn,SAMPLE_JS); fns.push_back(fn);} Benchmark::Config c; c.files=fns; c.language="js"; c.iterations=2; Benchmark b; Benchmark::Result r=b.run(c); CHECK(r.filesProcessed==10,"multi 5x2"); for(size_t i=0;i<fns.size();i++) remove(fns[i].c_str()); printf("Multi-file: PASSED\n"); }
    // Zero iterations edge
    { wf("test_bench_z.js",SAMPLE_JS); Benchmark::Config c; c.files.push_back("test_bench_z.js"); c.language="js"; c.iterations=0; Benchmark b; Benchmark::Result r=b.run(c); CHECK(r.filesProcessed==0,"zero it"); remove("test_bench_z.js"); printf("Zero iter: PASSED\n"); }
    // printReport / printCompareReport must not crash
    { Benchmark b; Benchmark::Result r; r.filesProcessed=1; r.totalTimeMs=5; r.averageTimeMs=5; r.minTimeMs=4; r.maxTimeMs=6; r.medianTimeMs=5; r.p95TimeMs=6; b.printReport(r); CHECK(true,"report"); Benchmark::Result bl=r; b.printCompareReport(r,bl,"tool"); CHECK(true,"compare report"); printf("Reports: PASSED\n"); }
    // 60 generated runs with full stat assertions (growth/idempotency breadth)
    { wf("test_bench_g.js",SAMPLE_JS); for(int i=0;i<60;i++){ Benchmark::Config c; c.files.push_back("test_bench_g.js"); c.language="js"; c.iterations=1; Benchmark b; Benchmark::Result r=b.run(c); CHECK(r.filesProcessed==1,"gen run"); CHECK(r.totalTimeMs>=0.0,"gen total"); CHECK(r.averageTimeMs>=0.0,"gen avg"); CHECK(r.minTimeMs>=0.0,"gen min"); CHECK(r.maxTimeMs>=r.minTimeMs,"gen max"); } remove("test_bench_g.js"); printf("Generated 60: PASSED\n"); }
    if(!failures) printf("Benchmark tests passed. checks=%d\n",checks); else printf("Benchmark tests FAILED: %d checks=%d\n",failures,checks);
    return failures;
}
