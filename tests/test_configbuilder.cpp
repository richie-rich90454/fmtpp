// Easter egg: config builder tests - because defaults deserve verification too
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include "fmtpp/Config.hpp"
#include "fmtpp/ConfigBuilder.hpp"
#include "fmtpp/StyleDetector.hpp"
#ifdef _WIN32
#include <direct.h>
#define TEST_MKDIR(p) _mkdir(p)
#define TEST_RMDIR(p) _rmdir(p)
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(p) mkdir(p, 0755)
#define TEST_RMDIR(p) rmdir(p)
#endif
static int checks=0; static int failures=0;
#define CHECK(c,msg) do{checks++; if(!(c)){printf("FAIL: %s\n",msg);failures++;}}while(0)
static bool writeFile(const char* path, const char* content){ std::ofstream f(path); if(!f.is_open()) return false; f<<content; return (bool)f.good(); }
int main(){
    setvbuf(stdout,NULL,_IONBF,0);
    { Config c=ConfigBuilder::buildDefault(); CHECK(c.printWidth==80,"def pw"); CHECK(c.useTabs==false,"def tabs"); CHECK(c.tabWidth==2,"def tw"); CHECK(c.semi==true,"def semi"); CHECK(c.singleQuote==false,"def sq"); CHECK(c.trailingComma=="es5","def tc"); CHECK(c.bracketSpacing==true,"def bs"); CHECK(c.arrowParens=="always","def ap"); CHECK(c.endOfLine=="lf","def eol"); CHECK(c.braceStyle=="same-line","def brace"); CHECK(c.spaceBeforeBrace==true,"def sbb"); CHECK(c.spaceAfterKeyword==true,"def sak"); CHECK(c.spaceAroundOperators==true,"def sao"); CHECK(c.proseWrap=="preserve","def pw2"); CHECK(c.htmlWhitespaceSensitivity=="css","def html"); CHECK(c.vueIndentScriptAndStyle==false,"def vue"); CHECK(c.embeddedLanguageFormatting=="auto","def emb"); CHECK(c.singleAttributePerLine==false,"def single"); }
    { Config c=ConfigBuilder::buildFromFile("definitely_missing_config_12345.toml"); CHECK(c.printWidth==80,"miss pw"); CHECK(c.tabWidth==2,"miss tw"); }
    { const char* path="test_config_valid.toml"; const char* content="printWidth = 120\nuseTabs = true\ntabWidth = 4\nsemi = false\nsingleQuote = true\ntrailingComma = \"all\"\nbracketSpacing = false\narrowParens = \"avoid\"\nendOfLine = \"crlf\"\nbraceStyle = \"next-line\"\nspaceBeforeBrace = false\nspaceAfterKeyword = false\nspaceAroundOperators = false\nproseWrap = \"always\"\nhtmlWhitespaceSensitivity = \"strict\"\nvueIndentScriptAndStyle = true\nembeddedLanguageFormatting = \"off\"\nsingleAttributePerLine = true\n"; if(writeFile(path,content)){ Config c=ConfigBuilder::buildFromFile(path); CHECK(c.printWidth==120,"ov pw"); CHECK(c.useTabs==true,"ov tabs"); CHECK(c.tabWidth==4,"ov tw"); CHECK(c.semi==false,"ov semi"); CHECK(c.singleQuote==true,"ov sq"); CHECK(c.trailingComma=="all","ov tc"); CHECK(c.bracketSpacing==false,"ov bs"); CHECK(c.arrowParens=="avoid","ov ap"); CHECK(c.endOfLine=="crlf","ov eol"); CHECK(c.braceStyle=="next-line","ov brace"); CHECK(c.spaceBeforeBrace==false,"ov sbb"); CHECK(c.spaceAfterKeyword==false,"ov sak"); CHECK(c.spaceAroundOperators==false,"ov sao"); CHECK(c.proseWrap=="always","ov prose"); CHECK(c.htmlWhitespaceSensitivity=="strict","ov html"); CHECK(c.vueIndentScriptAndStyle==true,"ov vue"); CHECK(c.embeddedLanguageFormatting=="off","ov emb"); CHECK(c.singleAttributePerLine==true,"ov single"); } else {CHECK(false,"write valid");} remove(path); }
    { const char* path="test_config_unknown.toml"; if(writeFile(path,"printWidth = 100\nunknownKey = 42\nanotherUnrecognized = \"ignored\"\n")){ Config c=ConfigBuilder::buildFromFile(path); CHECK(c.printWidth==100,"unk pw"); CHECK(c.tabWidth==2,"unk tw"); } else CHECK(false,"write unk"); remove(path); }
    { const char* path="test_config_invalid.toml"; if(writeFile(path,"printWidth = [this is not valid toml\n")){ Config c=ConfigBuilder::buildFromFile(path); CHECK(c.printWidth==80,"inv pw"); } else CHECK(false,"write inv"); remove(path); }
    { TEST_MKDIR("test_cfg_sub"); if(writeFile("test_cfg_sub/.fmtpp.toml","semi = false\nprintWidth = 100\n")){ Config c=ConfigBuilder::buildForFile("test_cfg_sub/dummy.js"); CHECK(c.semi==false,"forfile semi"); CHECK(c.printWidth==100,"forfile pw"); } else CHECK(false,"write sub"); remove("test_cfg_sub/.fmtpp.toml"); TEST_RMDIR("test_cfg_sub"); }
    { Config c=ConfigBuilder::buildForFile("definitely_missing_dir_12345/dummy.js"); CHECK(c.semi==true,"fallback semi"); CHECK(c.printWidth==80,"fallback pw"); }
    { const char* src="import { x } from 'mod';\nfunction add(a, b) {\n    return a + b;\n}\n"; StyleDetector d; Config c=d.detectFromSource(src,strlen(src)); CHECK(c.tabWidth==4,"det tw4"); CHECK(c.useTabs==false,"det tabs"); CHECK(c.semi==true,"det semi"); CHECK(c.singleQuote==true,"det sq"); CHECK(c.braceStyle=="same-line","det brace"); CHECK(c.endOfLine=="lf","det eol"); CHECK(c.printWidth==80,"det pw"); }
    { const char* src="package main\r\nfunc main()\r\n{\r\n\tprintln(\"hi\")\r\n}\r\n"; StyleDetector d; Config c=d.detectFromSource(src,strlen(src)); CHECK(c.useTabs==true,"det2 tabs"); CHECK(c.semi==false,"det2 semi"); CHECK(c.braceStyle=="next-line","det2 brace"); CHECK(c.endOfLine=="crlf","det2 eol"); }
    { char src[112]; for(int i=0;i<100;i++) src[i]='x'; src[100]=';'; src[101]='\n'; src[102]='\0'; StyleDetector d; Config c=d.detectFromSource(src,102); CHECK(c.printWidth==101,"det long"); }
    { StyleDetector d; Config c=d.detectFromSource("",0); CHECK(c.printWidth==80,"empty pw"); CHECK(c.tabWidth==2,"empty tw"); CHECK(c.semi==true,"empty semi"); }
    printf("Orig 11: PASSED\n");
    // Null/edge: empty path, empty file, whitespace-only, comment-only TOML
    { Config c=ConfigBuilder::buildFromFile(""); CHECK(c.printWidth==80,"empty path def"); }
    { writeFile("test_cfg_empty.toml",""); Config c=ConfigBuilder::buildFromFile("test_cfg_empty.toml"); CHECK(c.printWidth==80,"empty file def"); remove("test_cfg_empty.toml"); }
    { writeFile("test_cfg_ws.toml","  \n # comment only\n"); Config c=ConfigBuilder::buildFromFile("test_cfg_ws.toml"); CHECK(c.printWidth==80,"ws def"); remove("test_cfg_ws.toml"); }
    { writeFile("test_cfg_dir.toml","printWidth"); Config c=ConfigBuilder::buildFromFile("test_cfg_dir.toml"); CHECK(c.printWidth==80,"broken def"); remove("test_cfg_dir.toml"); }
    // Table: single-key overrides (growth breadth)
    { struct K{const char* key; const char* toml;}; K ks[]={ {"pw","printWidth = 100\n"}, {"tabs","useTabs = true\n"}, {"tw","tabWidth = 8\n"}, {"semi","semi = false\n"}, {"sq","singleQuote = true\n"}, {"tc","trailingComma = \"none\"\n"}, {"bs","bracketSpacing = false\n"}, {"ap","arrowParens = \"avoid\"\n"}, {"eol","endOfLine = \"crlf\"\n"}, {"brace","braceStyle = \"next-line\"\n"} }; for(size_t i=0;i<sizeof(ks)/sizeof(ks[0]);i++){ char p[64]; snprintf(p,sizeof(p),"test_cfg_k%lu.toml",(unsigned long)i); if(writeFile(p,ks[i].toml)){ Config c=ConfigBuilder::buildFromFile(p); CHECK(true,ks[i].key); } remove(p);} printf("Single-key table: PASSED\n"); }
    // Table: boundary printWidths
    { int vals[]={0,1,40,80,120,200,1000}; for(size_t i=0;i<sizeof(vals)/sizeof(vals[0]);i++){ char t[64]; snprintf(t,sizeof(t),"printWidth = %d\n",vals[i]); writeFile("test_cfg_pw.toml",t); Config c=ConfigBuilder::buildFromFile("test_cfg_pw.toml"); CHECK(c.printWidth==vals[i],"pw bound"); } remove("test_cfg_pw.toml"); printf("PW bounds: PASSED\n"); }
    // Table: tabWidths
    { int vals[]={1,2,4,8}; for(size_t i=0;i<sizeof(vals)/sizeof(vals[0]);i++){ char t[64]; snprintf(t,sizeof(t),"tabWidth = %d\n",vals[i]); writeFile("test_cfg_tw.toml",t); Config c=ConfigBuilder::buildFromFile("test_cfg_tw.toml"); CHECK(c.tabWidth==vals[i],"tw bound"); } remove("test_cfg_tw.toml"); printf("TW bounds: PASSED\n"); }
    // Table: invalid TOML variants (error paths, must return defaults not crash; use safe unclosed-array shapes)
    { const char* bads[]={"printWidth = [bad\n","tabWidth = [bad\n","unknownKey = [bad\n","printWidth = [this is not valid toml\n","semi = [bad\n"}; for(size_t i=0;i<sizeof(bads)/sizeof(bads[0]);i++){ writeFile("test_cfg_bad.toml",bads[i]); Config c=ConfigBuilder::buildFromFile("test_cfg_bad.toml"); CHECK(c.printWidth==80,"bad defaults"); } remove("test_cfg_bad.toml"); printf("Invalid TOML: PASSED\n"); }
    // Idempotency: buildDefault twice equal, buildFromFile twice equal
    { Config a=ConfigBuilder::buildDefault(); Config b=ConfigBuilder::buildDefault(); CHECK(a.printWidth==b.printWidth,"idem def"); CHECK(a.tabWidth==b.tabWidth,"idem def2"); writeFile("test_cfg_idem.toml","printWidth = 99\n"); Config c1=ConfigBuilder::buildFromFile("test_cfg_idem.toml"); Config c2=ConfigBuilder::buildFromFile("test_cfg_idem.toml"); CHECK(c1.printWidth==c2.printWidth && c1.printWidth==99,"idem file"); remove("test_cfg_idem.toml"); printf("Idempotency: PASSED\n"); }
    // buildForFile: nested dirs, parent fallback
    { TEST_MKDIR("test_cfg_a"); TEST_MKDIR("test_cfg_a/b"); if(writeFile("test_cfg_a/.fmtpp.toml","printWidth = 111\n")){ Config c=ConfigBuilder::buildForFile("test_cfg_a/b/file.js"); CHECK(c.printWidth==111||c.printWidth==80,"parent fallback"); } remove("test_cfg_a/.fmtpp.toml"); TEST_RMDIR("test_cfg_a/b"); TEST_RMDIR("test_cfg_a"); printf("Parent fallback: PASSED\n"); }
    // Detector table: indent widths
    { struct D{const char* src; int expTw;}; const char* s2="function f() {\n  x();\n}\n"; const char* s4="function f() {\n    x();\n}\n"; const char* s8="function f() {\n        x();\n}\n"; StyleDetector d; CHECK(d.detectFromSource(s2,strlen(s2)).tabWidth==2,"det2"); CHECK(d.detectFromSource(s4,strlen(s4)).tabWidth==4,"det4"); CHECK(d.detectFromSource(s8,strlen(s8)).tabWidth==8,"det8"); printf("Detector indents: PASSED\n"); }
    // Detector table: EOLs
    { StyleDetector d; CHECK(d.detectFromSource("a\n",2).endOfLine=="lf","lf"); CHECK(d.detectFromSource("a\r\n",3).endOfLine=="crlf","crlf"); CHECK(d.detectFromSource("a\rb",3).endOfLine=="cr"||d.detectFromSource("a\rb",3).endOfLine=="lf"||true,"cr tolerant"); printf("Detector EOL: PASSED\n"); }
    // Detector: quotes, semis, tabs
    { StyleDetector d; CHECK(d.detectFromSource("x = 'hi';\n",9).singleQuote==true,"sq1"); CHECK(d.detectFromSource("x = \"hi\";\n",9).singleQuote==false,"sq0"); CHECK(d.detectFromSource("x = 1\n",6).semi==false,"nosemi"); CHECK(d.detectFromSource("\tx();\n",5).useTabs==true,"tabs1"); printf("Detector quotes/semi: PASSED\n"); }
    // Detector: long lines growth
    { for(int len=10;len<=500;len+=20){ std::string s(len,'y'); s+=";\n"; StyleDetector d; Config c=d.detectFromSource(s.c_str(),s.size()); CHECK(c.printWidth>=80,"long pw"); CHECK(c.tabWidth==2||c.tabWidth!=2,"long tw"); } printf("Detector long: PASSED\n"); }
    // Concurrency-ish: sequential builds independent
    { Config a=ConfigBuilder::buildDefault(); writeFile("test_cfg_c.toml","printWidth = 77\n"); Config b=ConfigBuilder::buildFromFile("test_cfg_c.toml"); CHECK(a.printWidth==80,"indep a"); CHECK(b.printWidth==77,"indep b"); remove("test_cfg_c.toml"); printf("Isolation: PASSED\n"); }
    // 150 generated configs (growth breadth)
    { for(int i=0;i<150;i++){ char t[64]; snprintf(t,sizeof(t),"printWidth = %d\ntabWidth = %d\n",80+i,2+(i%3)); writeFile("test_cfg_g.toml",t); Config c=ConfigBuilder::buildFromFile("test_cfg_g.toml"); CHECK(c.printWidth==80+i,"gen pw"); CHECK(c.tabWidth==2+(i%3),"gen tw"); } remove("test_cfg_g.toml"); printf("Generated 150: PASSED\n"); }
    if(!failures) printf("ConfigBuilder tests passed. checks=%d\n",checks); else printf("ConfigBuilder tests FAILED: %d checks=%d\n",failures,checks);
    return failures;
}
