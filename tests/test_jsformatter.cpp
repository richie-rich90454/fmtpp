// Easter egg: JS formatter tests - semicolons optional, quality mandatory
#include <cstdio>
#include <cstring>
#include <string>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/JSFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ExportDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/ReturnStmtNode.hpp"
#include "fmtpp/ast/ThrowStmtNode.hpp"
#include "fmtpp/ast/TryCatchStmtNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/SwitchStmtNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/ArrowFunctionNode.hpp"
#include "fmtpp/ast/ArrayLiteralNode.hpp"
#include "fmtpp/ast/ObjectLiteralNode.hpp"
#include "fmtpp/ast/PropertyNode.hpp"
#include "fmtpp/ast/TemplateLiteralNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"

namespace {
int passed = 0;
int failed = 0;
int rowChecks = 0;

void check(bool ok, const char* name, const char* detail)
{
    rowChecks++;
    if (ok) { passed++; }
    else { printf("  FAIL: %s (%s)\n", name, detail); failed++; }
}

std::string fmtProg(ProgramNode* prog)
{
    OutputBuffer out(8192);
    Config config;
    JSFormatter formatter(out, config);
    formatter.format(prog);
    const char* d = out.getData();
    return d ? std::string(d) : std::string();
}

bool has(const std::string& s, const char* m) { return s.find(m) != std::string::npos; }

IdentifierNode* ident(const char* n)
{
    IdentifierNode* id = ASTNodeFactory::createIdentifierNode();
    id->m_name = n;
    return id;
}
LiteralNode* numLit(const char* v)
{
    LiteralNode* l = ASTNodeFactory::createLiteralNode();
    l->m_type = LiteralNode::TYPE_NUMBER;
    l->m_value = v;
    return l;
}
BlockNode* emptyBlock() { return ASTNodeFactory::createBlockNode(); }
BlockNode* retBlock(const char* v)
{
    BlockNode* b = ASTNodeFactory::createBlockNode();
    ReturnStmtNode* r = ASTNodeFactory::createReturnStmtNode();
    r->m_argument = ident(v);
    b->m_statements.push_back(r);
    return b;
}
}

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    // Table 1: variable decls (kinds x names x values) = 3*12*4 = 144 rows
    {
        const char* kinds[] = {"let", "const", "var"};
        const char* names[] = {"x", "foo", "count", "data", "result", "value", "item", "list", "cache", "state", "tmp", "el"};
        const char* vals[] = {"42", "0", "99", "7"};
        for (int ki = 0; ki < 3; ki++)
            for (int ni = 0; ni < 12; ni++)
                for (int vi = 0; vi < 4; vi++)
                {
                    ProgramNode* prog = ASTNodeFactory::createProgramNode();
                    VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
                    v->m_kind = kinds[ki];
                    v->m_name = names[ni];
                    v->m_initializer = numLit(vals[vi]);
                    prog->m_statements.push_back(v);
                    std::string out = fmtProg(prog);
                    bool ok = !out.empty() && !has(out, "placeholder")
                        && has(out, kinds[ki]) && has(out, names[ni])
                        && has(out, vals[vi]) && has(out, "=");
                    char nb[128]; snprintf(nb, sizeof(nb), "var %s/%s/%s", kinds[ki], names[ni], vals[vi]);
                    if (!ok) { printf("  FAIL: [%s] out=[%s]\n", nb, out.c_str()); failed++; }
                    else { passed++; }
                    rowChecks++;
                }
        printf("  PASS: var-table (144 rows)\n");
    }

    // Table 2: functions (names x modifiers) = 12*5 = 60 rows
    {
        const char* names[] = {"main", "foo", "bar", "fetchData", "render", "handleClick", "compute", "init", "destroy", "update", "save", "load"};
        const char* mods[] = {"", "async", "export", "export async", "static"};
        for (int ni = 0; ni < 12; ni++)
            for (int mi = 0; mi < 5; mi++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
                f->m_name = names[ni];
                f->m_modifiers = mods[mi];
                f->m_params.push_back(ident("a"));
                f->m_params.push_back(ident("b"));
                f->m_body = retBlock("a");
                prog->m_statements.push_back(f);
                std::string out = fmtProg(prog);
                bool ok = !has(out, "placeholder") && has(out, "function")
                    && has(out, names[ni]) && has(out, "a") && has(out, "b")
                    && has(out, "{") && has(out, "}");
                if (mods[mi][0] != '\0') { ok = ok && has(out, mods[mi]); }
                char nb[128]; snprintf(nb, sizeof(nb), "func %s [%s]", names[ni], mods[mi]);
                if (!ok) { printf("  FAIL: [%s] out=[%s]\n", nb, out.c_str()); failed++; }
                else { passed++; }
                rowChecks++;
            }
        printf("  PASS: func-table (60 rows)\n");
    }

    // Table 3: arrow functions (param counts x bodies)
    {
        const char* paramSets[][4] = {
            {NULL, NULL, NULL, NULL},
            {"x", NULL, NULL, NULL},
            {"a", "b", NULL, NULL},
            {"a", "b", "c", NULL},
            {"e", NULL, NULL, NULL},
            {"cb", "err", "data", NULL},
        };
        int paramCounts[] = {0, 1, 2, 3, 1, 3};
        for (int i = 0; i < 6; i++)
            for (int bodyKind = 0; bodyKind < 4; bodyKind++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
                char vname[32]; snprintf(vname, sizeof(vname), "fn%d_%d", i, bodyKind);
                v->m_kind = "const";
                v->m_name = vname;
                ArrowFunctionNode* aw = ASTNodeFactory::createArrowFunctionNode();
                for (int p = 0; p < paramCounts[i]; p++) { aw->m_params.push_back(ident(paramSets[i][p])); }
                if (bodyKind % 2 == 0) { aw->m_body = retBlock("a"); }
                else { aw->m_body = ident("x"); }
                v->m_initializer = aw;
                prog->m_statements.push_back(v);
                std::string out = fmtProg(prog);
                bool ok = has(out, "=>") && has(out, vname) && !has(out, "placeholder");
                for (int p = 0; p < paramCounts[i]; p++) { ok = ok && has(out, paramSets[i][p]); }
                char nb[64]; snprintf(nb, sizeof(nb), "arrow %d/%d", i, bodyKind);
                check(ok, nb, out.c_str());
            }
    }

    // Table 4: classes (names x supers x ifaces)
    {
        const char* names[] = {"Foo", "Dog", "App", "Widget", "Store", "View", "Ctrl", "Base"};
        const char* supers[] = {"", "Animal", "Base", "Component"};
        const char* ifaces[] = {"", "Pet", "Disposable"};
        for (int ni = 0; ni < 8; ni++)
            for (int si = 0; si < 4; si++)
                for (int ii = 0; ii < 3; ii++)
                {
                    ProgramNode* prog = ASTNodeFactory::createProgramNode();
                    ClassDeclNode* c = ASTNodeFactory::createClassDeclNode();
                    c->m_name = names[ni];
                    if (supers[si][0]) { c->m_superClass = ident(supers[si]); }
                    if (ifaces[ii][0]) { c->m_implements.push_back(ident(ifaces[ii])); }
                    c->m_body = emptyBlock();
                    prog->m_statements.push_back(c);
                    std::string out = fmtProg(prog);
                    bool ok = has(out, "class") && has(out, names[ni]) && !has(out, "placeholder");
                    if (supers[si][0]) { ok = ok && has(out, supers[si]) && has(out, "extends"); }
                    if (ifaces[ii][0]) { ok = ok && has(out, ifaces[ii]); }
                    if (!ok) { printf("  FAIL: [class %s] out=[%s]\n", names[ni], out.c_str()); failed++; }
                    else { passed++; }
                    rowChecks++;
                }
        printf("  PASS: class-table (96 rows)\n");
    }

    // Constructors bare (no function prefix)
    {
        const char* paramSets[] = {"", "x", "x, y", "a, b, c", "opts"};
        for (int i = 0; i < 5; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ClassDeclNode* c = ASTNodeFactory::createClassDeclNode();
            c->m_name = "Foo";
            BlockNode* body = ASTNodeFactory::createBlockNode();
            FunctionDeclNode* ctor = ASTNodeFactory::createFunctionDeclNode();
            ctor->m_name = "constructor";
            if (paramSets[i][0]) {
                if (strstr(paramSets[i], ",")) {
                    ctor->m_params.push_back(ident("a"));
                    ctor->m_params.push_back(ident("b"));
                } else { ctor->m_params.push_back(ident(paramSets[i])); }
            }
            ctor->m_body = emptyBlock();
            body->m_statements.push_back(ctor);
            c->m_body = body;
            prog->m_statements.push_back(c);
            std::string out = fmtProg(prog);
            bool ok = has(out, "constructor") && !has(out, "function constructor") && !has(out, "placeholder");
            char nb[64]; snprintf(nb, sizeof(nb), "ctor %d", i);
            check(ok, nb, out.c_str());
        }
    }

    // Getters (name carries get prefix)
    {
        const char* getters[] = {"get value", "get count", "get name", "get state", "get data"};
        for (int i = 0; i < 5; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ClassDeclNode* c = ASTNodeFactory::createClassDeclNode();
            c->m_name = "Box";
            BlockNode* body = ASTNodeFactory::createBlockNode();
            FunctionDeclNode* g = ASTNodeFactory::createFunctionDeclNode();
            g->m_name = getters[i];
            g->m_body = retBlock("x");
            body->m_statements.push_back(g);
            c->m_body = body;
            prog->m_statements.push_back(c);
            std::string out = fmtProg(prog);
            check(has(out, getters[i]) && !has(out, "placeholder"), getters[i], out.c_str());
        }
    }

    // Imports / exports
    {
        const char* sources[] = {"react", "lodash", "fs", "path", "express", "vue", "@app/util", "./local", "axios", "moment"};
        const char* specs[] = {"useState", "useEffect", "map", "readFile", "join", "ref", "get", "format", "render", "save"};
        for (int i = 0; i < 10; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
            imp->m_source = sources[i];
            imp->m_specifiers.push_back(ident(specs[i]));
            prog->m_statements.push_back(imp);
            std::string out = fmtProg(prog);
            bool ok = has(out, "import") && has(out, sources[i]) && has(out, specs[i]) && !has(out, "placeholder");
            char nb[64]; snprintf(nb, sizeof(nb), "import %s", sources[i]);
            check(ok, nb, out.c_str());
        }
        for (int i = 0; i < 8; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ExportDeclNode* e = ASTNodeFactory::createExportDeclNode();
            FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
            char fn[32]; snprintf(fn, sizeof(fn), "expFn%d", i);
            f->m_name = fn;
            f->m_body = emptyBlock();
            e->m_declaration = f;
            prog->m_statements.push_back(e);
            std::string out = fmtProg(prog);
            bool ok = has(out, "export") && has(out, "function") && has(out, fn) && !has(out, "placeholder");
            char nb[64]; snprintf(nb, sizeof(nb), "export %s", fn);
            check(ok, nb, out.c_str());
        }
        // default import + re-export with source
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
            imp->m_source = "react";
            imp->m_defaultImport = ident("React");
            imp->m_specifiers.push_back(ident("useState"));
            prog->m_statements.push_back(imp);
            std::string out = fmtProg(prog);
            check(has(out, "React") && has(out, "useState") && has(out, "react"), "import default+named", out.c_str());
        }
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ExportDeclNode* e = ASTNodeFactory::createExportDeclNode();
            e->m_declaration = ident("foo");
            e->m_source = "mod";
            prog->m_statements.push_back(e);
            std::string out = fmtProg(prog);
            check(has(out, "export") && has(out, "mod"), "export-from", out.c_str());
        }
    }

    // Template literals
    {
        const char* quasis[] = {"hello ", "hi ", "val=", "msg:", "out>"};
        const char* exprs[] = {"name", "x", "count", "data", "v"};
        for (int i = 0; i < 5; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
            v->m_kind = "const";
            char vn[32]; snprintf(vn, sizeof(vn), "tpl%d", i);
            v->m_name = vn;
            TemplateLiteralNode* t = ASTNodeFactory::createTemplateLiteralNode();
            t->m_quasis.push_back(ident(quasis[i]));
            t->m_expressions.push_back(ident(exprs[i]));
            t->m_quasis.push_back(ident("!"));
            v->m_initializer = t;
            prog->m_statements.push_back(v);
            std::string out = fmtProg(prog);
            bool ok = has(out, "`") && has(out, "${") && has(out, exprs[i]) && !has(out, "placeholder");
            char nb[32]; snprintf(nb, sizeof(nb), "tpl %d", i);
            check(ok, nb, out.c_str());
        }
    }

    // Destructuring
    {
        const char* pats[] = {"{a, b}", "{x, y, z}", "[x, y]", "[a, b, c]", "{p: q}", "{a = 1, b = 2}"};
        const char* inits[] = {"obj", "arr", "src", "data", "opts", "cfg"};
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
            v->m_kind = (i % 2 == 0) ? "const" : "let";
            v->m_name = pats[i];
            v->m_initializer = ident(inits[i]);
            prog->m_statements.push_back(v);
            std::string out = fmtProg(prog);
            check(has(out, pats[i]) && has(out, inits[i]), pats[i], out.c_str());
        }
    }

    // Arrays / objects
    {
        for (int n = 0; n < 8; n++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
            v->m_kind = "const";
            char vn[32]; snprintf(vn, sizeof(vn), "arr%d", n);
            v->m_name = vn;
            ArrayLiteralNode* a = ASTNodeFactory::createArrayLiteralNode();
            for (int k = 0; k <= n % 4; k++) { char bv[16]; snprintf(bv, sizeof(bv), "%d", k + 1); a->m_elements.push_back(numLit(bv)); }
            v->m_initializer = a;
            prog->m_statements.push_back(v);
            std::string out = fmtProg(prog);
            check(has(out, "[") && has(out, "]") && has(out, vn), vn, out.c_str());
        }
        const char* keys[] = {"key", "name", "id", "type", "val", "data", "x", "cfg"};
        for (int i = 0; i < 8; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
            v->m_kind = "const";
            char vn[32]; snprintf(vn, sizeof(vn), "obj%d", i);
            v->m_name = vn;
            ObjectLiteralNode* o = ASTNodeFactory::createObjectLiteralNode();
            PropertyNode* p = ASTNodeFactory::createPropertyNode();
            p->m_key = ident(keys[i]);
            LiteralNode* sv = ASTNodeFactory::createLiteralNode();
            sv->m_type = LiteralNode::TYPE_STRING;
            sv->m_value = "v";
            p->m_value = sv;
            o->m_properties.push_back(p);
            v->m_initializer = o;
            prog->m_statements.push_back(v);
            std::string out = fmtProg(prog);
            check(has(out, keys[i]) && has(out, "{") && has(out, "}"), keys[i], out.c_str());
        }
    }

    // Control flow: if / loops / try / switch / comments
    {
        const char* conds[] = {"x > 0", "a", "ok", "n === 1", "flag", "count < 10", "a && b", "done"};
        for (int i = 0; i < 8; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            IfStmtNode* s = ASTNodeFactory::createIfStmtNode();
            s->m_condition = ident(conds[i]);
            s->m_thenBranch = retBlock("a");
            s->m_elseBranch = retBlock("b");
            prog->m_statements.push_back(s);
            std::string out = fmtProg(prog);
            bool ok = has(out, "if") && has(out, conds[i]) && has(out, "else") && !has(out, "placeholder");
            char nb[64]; snprintf(nb, sizeof(nb), "if %s", conds[i]);
            check(ok, nb, out.c_str());
        }
        const char* wconds[] = {"running", "ok", "n > 0", "more", "alive", "cond"};
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            WhileStmtNode* w = ASTNodeFactory::createWhileStmtNode();
            w->m_condition = ident(wconds[i]);
            w->m_body = emptyBlock();
            prog->m_statements.push_back(w);
            std::string out = fmtProg(prog);
            check(has(out, "while") && has(out, wconds[i]), wconds[i], out.c_str());
        }
        const char* forins[] = {"x of items", "k in obj", "v of list", "i of range", "e of events"};
        for (int i = 0; i < 5; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ForStmtNode* f = ASTNodeFactory::createForStmtNode();
            f->m_classic = false;
            f->m_condition = ident(forins[i]);
            f->m_body = emptyBlock();
            prog->m_statements.push_back(f);
            std::string out = fmtProg(prog);
            check(has(out, "for") && has(out, forins[i]), forins[i], out.c_str());
        }
        // classic for
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ForStmtNode* f = ASTNodeFactory::createForStmtNode();
            f->m_classic = true;
            VariableDeclNode* init = ASTNodeFactory::createVariableDeclNode();
            init->m_kind = "let";
            init->m_name = "i";
            init->m_initializer = numLit("0");
            f->m_init = init;
            f->m_condition = ident("i < 10");
            f->m_update = ident("i++");
            f->m_body = emptyBlock();
            prog->m_statements.push_back(f);
            std::string out = fmtProg(prog);
            check(has(out, "for") && has(out, "i < 10") && has(out, "i++"), "classic-for", out.c_str());
        }
        // switch
        {
            const char* arms[] = {"case 1: one()", "case 2: two()", "default: other()"};
            for (int i = 0; i < 3; i++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                SwitchStmtNode* sw = ASTNodeFactory::createSwitchStmtNode();
                sw->m_discriminant = ident("x");
                sw->m_cases.push_back(ident(arms[i]));
                prog->m_statements.push_back(sw);
                std::string out = fmtProg(prog);
                check(has(out, "switch") && has(out, arms[i]), arms[i], out.c_str());
            }
        }
        // try/catch/finally
        {
            const char* clauses[] = {"e", "err", "ex", "error"};
            for (int i = 0; i < 4; i++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                TryCatchStmtNode* t = ASTNodeFactory::createTryCatchStmtNode();
                t->m_body = emptyBlock();
                t->m_catchClause = ident(clauses[i]);
                t->m_catchBody = emptyBlock();
                t->m_finallyBlock = emptyBlock();
                prog->m_statements.push_back(t);
                std::string out = fmtProg(prog);
                bool ok = has(out, "try") && has(out, "catch") && has(out, "finally") && has(out, clauses[i]);
                char nb[32]; snprintf(nb, sizeof(nb), "try %s", clauses[i]);
                check(ok, nb, out.c_str());
            }
        }
        // throw + comments
        {
            const char* msgs[] = {"e", "err", "new Error(x)", "msg"};
            for (int i = 0; i < 4; i++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                ThrowStmtNode* th = ASTNodeFactory::createThrowStmtNode();
                th->m_expression = ident(msgs[i]);
                prog->m_statements.push_back(th);
                std::string out = fmtProg(prog);
                check(has(out, "throw") && has(out, msgs[i]), msgs[i], out.c_str());
            }
            for (int i = 0; i < 6; i++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                CommentNode* c = ASTNodeFactory::createCommentNode();
                char txt[32]; snprintf(txt, sizeof(txt), " note %d", i);
                c->m_text = txt;
                c->m_isBlock = (i % 2 == 1);
                prog->m_statements.push_back(c);
                std::string out = fmtProg(prog);
                bool ok = has(out, txt) && (has(out, "//") || has(out, "/*"));
                char nb[32]; snprintf(nb, sizeof(nb), "comment %d", i);
                check(ok, nb, out.c_str());
            }
        }
    }

    // Edge: idempotency, empty, verbatim, invalid, no placeholder
    {
        ProgramNode* p1 = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
        f->m_name = "idem";
        f->m_params.push_back(ident("a"));
        f->m_body = retBlock("a");
        p1->m_statements.push_back(f);
        std::string once = fmtProg(p1);
        ProgramNode* p2 = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* f2 = ASTNodeFactory::createFunctionDeclNode();
        f2->m_name = "idem";
        f2->m_params.push_back(ident("a"));
        f2->m_body = retBlock("a");
        p2->m_statements.push_back(f2);
        std::string twice = fmtProg(p2);
        check(!once.empty() && once == twice, "idempotent", once.c_str());

        ProgramNode* empty = ASTNodeFactory::createProgramNode();
        std::string eout = fmtProg(empty);
        check(!has(eout, "placeholder"), "empty", eout.c_str());

        ProgramNode* raw = ASTNodeFactory::createProgramNode();
        RawTextNode* rt = ASTNodeFactory::createRawTextNode();
        rt->m_text = "@@@verbatim-kept###";
        raw->m_statements.push_back(rt);
        std::string rout = fmtProg(raw);
        check(has(rout, "@@@verbatim-kept###"), "raw-verbatim", rout.c_str());

        ProgramNode* nob = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* nb2 = ASTNodeFactory::createFunctionDeclNode();
        nb2->m_name = "ext";
        nb2->m_body = NULL;
        nob->m_statements.push_back(nb2);
        std::string nout = fmtProg(nob);
        check(has(nout, "function ext") && has(nout, ";"), "nobody-semi", nout.c_str());

        ProgramNode* many = ASTNodeFactory::createProgramNode();
        for (int i = 0; i < 5; i++) { VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode(); v->m_kind = "let"; char n[16]; snprintf(n, sizeof(n), "v%d", i); v->m_name = n; v->m_initializer = numLit("1"); many->m_statements.push_back(v); }
        std::string mout = fmtProg(many);
        check(!has(mout, "placeholder") && has(mout, "v4"), "no-placeholder", mout.c_str());
    }

    printf("\n========================================\n");
    printf("JSFormatter tests: %d passed, %d failed (%d checks)\n", passed, failed, rowChecks);
    printf("========================================\n");
    return failed > 0 ? 1 : 0;
}
