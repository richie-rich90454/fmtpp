// Easter egg: output buffer tests - because every byte deserves a verified home
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "fmtpp/OutputBuffer.hpp"
static int checks=0; static int failures=0;
#define CHECK(c,msg) do{checks++; if(!(c)){printf("FAIL: %s\n",msg);failures++;}}while(0)
int main()
{
    setvbuf(stdout,NULL,_IONBF,0);
    { OutputBuffer buf; CHECK(buf.getSize()==0,"default size 0"); CHECK(buf.getCapacity()>=OutputBuffer::DEFAULT_CAPACITY,"default cap"); CHECK(buf.getData()!=NULL,"data non-null"); }
    { OutputBuffer buf; buf.append("Hello"); CHECK(buf.getSize()==5,"hello size"); CHECK(strcmp(buf.getData(),"Hello")==0,"hello data"); }
    { OutputBuffer buf; buf.append("HelloWorld",5); CHECK(buf.getSize()==5,"len5 size"); CHECK(strcmp(buf.getData(),"Hello")==0,"len5 data"); }
    { OutputBuffer buf; buf.appendChar('A'); buf.appendChar('B'); buf.appendChar('C'); CHECK(buf.getSize()==3,"abc size"); CHECK(strcmp(buf.getData(),"ABC")==0,"abc data"); }
    { OutputBuffer buf; buf.printf("int=%d str=%s",42,"world"); CHECK(strcmp(buf.getData(),"int=42 str=world")==0,"printf"); }
    { OutputBuffer buf; buf.append("ToStringTest"); CHECK(buf.toString()=="ToStringTest","tostring"); }
    { OutputBuffer buf; buf.append("something"); buf.clear(); CHECK(buf.getSize()==0,"clear size"); CHECK(buf.getData()[0]=='\0',"clear nul"); }
    { OutputBuffer buf; buf.append("Hello"); buf.appendChar(' '); buf.append("World"); buf.appendChar('!'); CHECK(strcmp(buf.getData(),"Hello World!")==0,"multi"); CHECK(buf.getSize()==12,"multi size"); }
    { OutputBuffer buf(16); const char* s="This is a long string that should exceed the initial capacity of 16 bytes"; buf.append(s); CHECK(strcmp(buf.getData(),s)==0,"grow data"); CHECK(buf.getSize()==strlen(s),"grow size"); }
    { OutputBuffer buf(16); buf.printf("This is a formatted string with %d and %s that should grow",12345,"value"); CHECK(strcmp(buf.getData(),"This is a formatted string with 12345 and value that should grow")==0,"printf grow"); }
    { OutputBuffer buf; buf.append("FlushTest"); FILE* tmp=fopen("test_flush_output.txt","w"); if(tmp){ buf.flushTo(tmp); fclose(tmp); FILE* in=fopen("test_flush_output.txt","r"); if(in){ char rb[64]={0}; fread(rb,1,sizeof(rb)-1,in); fclose(in); CHECK(strcmp(rb,"FlushTest")==0,"flush"); } remove("test_flush_output.txt"); } }
    printf("Orig 11: PASSED\n");
    // Null handling table
    { OutputBuffer buf; size_t s0=buf.getSize(); buf.append((const char*)NULL); CHECK(buf.getSize()==s0,"append NULL no-op"); buf.append(NULL,5); CHECK(buf.getSize()==s0,"append NULL len no-op"); buf.append("hi",0); CHECK(buf.getSize()==s0,"len0 no-op"); buf.append(""); CHECK(buf.getSize()==s0,"empty no-op"); buf.printf(NULL); CHECK(buf.getSize()==s0,"printf NULL no-op"); buf.printf(""); CHECK(buf.getSize()==s0,"printf empty no-op"); buf.flushTo(NULL); CHECK(buf.getSize()==s0,"flush NULL no-op"); printf("Null handling: PASSED\n"); }
    // Edge sizes table
    { size_t lens[]={0,1,2,7,8,15,16,31,32,64,100,1000,4096,10000}; for(size_t i=0;i<sizeof(lens)/sizeof(lens[0]);i++){ OutputBuffer buf(16); std::string s(lens[i],'x'); buf.append(s.c_str(),s.size()); char m[64]; snprintf(m,sizeof(m),"edge len %lu",(unsigned long)lens[i]); CHECK(buf.getSize()==lens[i],m); CHECK(buf.toString()==s,m); } printf("Edge sizes: PASSED\n"); }
    // Growth table: capacities
    { size_t caps[]={1,2,8,16,64,256,4096}; for(size_t i=0;i<sizeof(caps)/sizeof(caps[0]);i++){ OutputBuffer buf(caps[i]); std::string big(5000,'y'); buf.append(big.c_str()); CHECK(buf.getSize()==5000,"grow5000"); CHECK(buf.getCapacity()>=5000,"cap grow"); CHECK(buf.toString()==big,"grow data"); } printf("Growth caps: PASSED\n"); }
    // printf variants table
    { {OutputBuffer b; b.printf("%d",42); CHECK(strcmp(b.getData(),"42")==0,"p d");} {OutputBuffer b; b.printf("%s","hi"); CHECK(strcmp(b.getData(),"hi")==0,"p s");} {OutputBuffer b; b.printf("%x",255); CHECK(strcmp(b.getData(),"ff")==0,"p x");} {OutputBuffer b; b.printf("%05d",42); CHECK(strcmp(b.getData(),"00042")==0,"p 05d");} {OutputBuffer b; b.printf("a%db%c",7,'Z'); CHECK(strcmp(b.getData(),"a7bZ")==0,"p multi");} {OutputBuffer b; b.printf("%.2f",3.14159); CHECK(strcmp(b.getData(),"3.14")==0,"p float");} {OutputBuffer b; b.printf("%ld",(long)123456789L); CHECK(strstr(b.getData(),"123456789")!=NULL,"p long");} {OutputBuffer b; b.printf("%zu",(size_t)999); CHECK(strcmp(b.getData(),"999")==0,"p zu");} printf("Printf variants: PASSED\n"); }
    // Reset/reuse + idempotency
    { OutputBuffer buf; for(int c=0;c<10;c++){ buf.append("cycle"); CHECK(buf.getSize()==5,"cycle size"); buf.clear(); CHECK(buf.getSize()==0,"cycle clear"); CHECK(buf.getData()[0]=='\0',"cycle nul"); } buf.clear(); buf.clear(); CHECK(buf.getSize()==0,"double clear"); buf.append("after"); CHECK(strcmp(buf.getData(),"after")==0,"reuse after clear"); printf("Reset/reuse: PASSED\n"); }
    // appendChar table
    { const char* alpha="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"; OutputBuffer buf; for(size_t i=0;i<strlen(alpha);i++) buf.appendChar(alpha[i]); CHECK(buf.getSize()==strlen(alpha),"chars size"); CHECK(strcmp(buf.getData(),alpha)==0,"chars data"); printf("appendChar table: PASSED\n"); }
    // toString edge
    { OutputBuffer buf; CHECK(buf.toString().empty(),"empty tostring"); buf.append("a"); CHECK(buf.toString()=="a","single"); buf.clear(); CHECK(buf.toString().empty(),"cleared tostring"); std::string big(10000,'z'); buf.append(big.c_str()); CHECK(buf.toString()==big,"big tostring"); printf("toString edge: PASSED\n"); }
    // flushTo variants
    { OutputBuffer buf; buf.flushTo(NULL); CHECK(true,"flush null safe"); OutputBuffer e; FILE* t=fopen("test_flush_e.txt","w"); if(t){ e.flushTo(t); fclose(t); FILE* in=fopen("test_flush_e.txt","r"); if(in){ char b[8]={0}; size_t n=fread(b,1,7,in); fclose(in); CHECK(n==0,"empty flush 0"); } remove("test_flush_e.txt"); } OutputBuffer m; m.append("line1\nline2\n"); FILE* t2=fopen("test_flush_m.txt","w"); if(t2){ m.flushTo(t2); fclose(t2); FILE* in=fopen("test_flush_m.txt","r"); if(in){ char b[64]={0}; fread(b,1,63,in); fclose(in); CHECK(strcmp(b,"line1\nline2\n")==0,"multiline flush"); } remove("test_flush_m.txt"); } printf("Flush variants: PASSED\n"); }
    // sequential appends growth stress
    { OutputBuffer buf(8); std::string exp; for(int i=0;i<200;i++){ char tmp[16]; snprintf(tmp,sizeof(tmp),"n%d;",i); buf.append(tmp); exp+=tmp; } CHECK(buf.toString()==exp,"stress 200 appends"); CHECK(buf.getSize()==exp.size(),"stress size"); printf("Stress appends: PASSED n=%lu\n",(unsigned long)exp.size()); }
    // concurrency-ish: independent buffers
    { OutputBuffer a,b; a.append("aaa"); b.append("bbb"); CHECK(strcmp(a.getData(),"aaa")==0,"indep a"); CHECK(strcmp(b.getData(),"bbb")==0,"indep b"); a.clear(); CHECK(strcmp(b.getData(),"bbb")==0,"clear isolated"); printf("Isolation: PASSED\n"); }
    // error paths: huge printf
    { OutputBuffer buf(16); std::string big(8000,'q'); buf.printf("%s",big.c_str()); CHECK(buf.getSize()==8000,"huge printf"); printf("Huge printf: PASSED\n"); }
    // table: 100 generated contents (growth + idempotency breadth)
    { for(int i=0;i<100;i++){ OutputBuffer buf(32); char tmp[64]; snprintf(tmp,sizeof(tmp),"gen_%03d_",i); std::string exp; for(int r=0;r<5;r++) exp+=tmp; buf.append(exp.c_str()); CHECK(buf.getSize()==exp.size(),"gen size"); CHECK(buf.toString()==exp,"gen data"); } printf("Generated 100: PASSED\n"); }
    if(!failures) printf("OutputBuffer tests passed. checks=%d\n",checks); else printf("OutputBuffer tests FAILED: %d checks=%d\n",failures,checks);
    return failures;
}
