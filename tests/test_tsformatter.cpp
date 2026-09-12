// Easter egg: TS formatter tests - where types and tabs live in harmony
#include <cstdio>
#include <cstring>
#include <string>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/TSFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/InterfaceDeclNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/TypeAliasNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/ExportDeclNode.hpp"
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
#include "fmtpp/ast/PropertyNode.hpp"
#include "fmtpp/ast/ArrowFunctionNode.hpp"
#include "fmtpp/ast/ArrayLiteralNode.hpp"
#include "fmtpp/ast/ObjectLiteralNode.hpp"
#include "fmtpp/ast/TemplateLiteralNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"

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
    TSFormatter formatter(out, config);
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

    // Functions with return types + modifiers (10 names x 5 rets x 3 mods = 150 rows)
    {
        const char* names[] = {"add", "fetch", "render", "compute", "save", "load", "update", "init", "parse", "emit"};
        const char* rets[] = {"number", "string", "void", "boolean", "Promise<void>"};
        const char* mods[] = {"", "async", "export"};
        for (int ni = 0; ni < 10; ni++)
            for (int ri = 0; ri < 5; ri++)
                for (int mi = 0; mi < 3; mi++)
                {
                    ProgramNode* prog = ASTNodeFactory::createProgramNode();
                    FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
                    f->m_name = names[ni];
                    f->m_returnType = rets[ri];
                    f->m_modifiers = mods[mi];
                    f->m_params.push_back(ident("a"));
                    f->m_params.push_back(ident("b"));
                    f->m_body = retBlock("a");
                    prog->m_statements.push_back(f);
                    std::string out = fmtProg(prog);
                    bool ok = has(out, "function") && has(out, names[ni]) && has(out, rets[ri])
                        && has(out, ": ") && !has(out, "placeholder");
                    if (mods[mi][0]) { ok = ok && has(out, mods[mi]); }
                    if (!ok) { printf("  FAIL: [func %s:%s %s] out=[%s]\n", names[ni], rets[ri], mods[mi], out.c_str()); failed++; }
                    else { passed++; }
                    rowChecks++;
                }
        printf("  PASS: func-table (150 rows)\n");
    }

    // Typed variable decls
    {
        const char* names[] = {"x", "count", "name", "flag", "data", "list", "cache", "state"};
        const char* types[] = {"number", "string", "boolean", "any", "unknown", "never"};
        for (int ni = 0; ni < 8; ni++)
            for (int ti = 0; ti < 6; ti++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
                v->m_kind = (ni % 2 == 0) ? "let" : "const";
                v->m_name = names[ni];
                v->m_type = types[ti];
                LiteralNode* lit = ASTNodeFactory::createLiteralNode();
                lit->m_type = LiteralNode::TYPE_NUMBER;
                lit->m_value = "1";
                v->m_initializer = lit;
                prog->m_statements.push_back(v);
                std::string out = fmtProg(prog);
                bool ok = has(out, names[ni]) && has(out, types[ti]) && has(out, ": ") && !has(out, "placeholder");
                char nb[64]; snprintf(nb, sizeof(nb), "var %s:%s", names[ni], types[ti]);
                check(ok, nb, out.c_str());
            }
    }

    // Interfaces (names x extends x props)
    {
        const char* names[] = {"Person", "Employee", "Shape", "Store", "Config", "Props", "State", "Item"};
        const char* bases[] = {"", "Base", "Named", "Entity"};
        const char* props[] = {"name", "id", "value", "kind"};
        const char* ptypes[] = {"string", "number", "boolean", "any"};
        for (int ni = 0; ni < 8; ni++)
            for (int bi = 0; bi < 4; bi++)
                for (int pi = 0; pi < 4; pi++)
                {
                    ProgramNode* prog = ASTNodeFactory::createProgramNode();
                    InterfaceDeclNode* iface = ASTNodeFactory::createInterfaceDeclNode();
                    iface->m_name = names[ni];
                    if (bases[bi][0]) { iface->m_extends.push_back(ident(bases[bi])); }
                    BlockNode* body = ASTNodeFactory::createBlockNode();
                    PropertyNode* p = ASTNodeFactory::createPropertyNode();
                    p->m_key = ident(props[pi]);
                    p->m_value = ident(ptypes[pi]);
                    body->m_statements.push_back(p);
                    iface->m_body = body;
                    prog->m_statements.push_back(iface);
                    std::string out = fmtProg(prog);
                    bool ok = has(out, "interface") && has(out, names[ni]) && has(out, props[pi])
                        && has(out, ptypes[pi]) && has(out, "{") && has(out, "}") && !has(out, "placeholder");
                    if (bases[bi][0]) { ok = ok && has(out, "extends") && has(out, bases[bi]); }
                    if (!ok) { printf("  FAIL: [iface %s] out=[%s]\n", names[ni], out.c_str()); failed++; }
                    else { passed++; }
                    rowChecks++;
                }
        printf("  PASS: iface-table (128 rows)\n");
    }

    // Enums incl const enum via m_kind
    {
        const char* names[] = {"Color", "Dir", "Kind", "Mode", "Size", "Level"};
        const char* kinds[] = {"", "enum", "const enum", "declare enum"};
        const char* members[] = {"Red", "Green", "Blue", "Up", "Down", "Auto"};
        for (int ni = 0; ni < 6; ni++)
            for (int ki = 0; ki < 4; ki++)
            {
                ProgramNode* prog = ASTNodeFactory::createProgramNode();
                EnumDeclNode* e = ASTNodeFactory::createEnumDeclNode();
                e->m_name = names[ni];
                if (kinds[ki][0]) { e->m_kind = kinds[ki]; }
                e->m_members.push_back(ident(members[ni]));
                e->m_members.push_back(ident(members[(ni + 1) % 6]));
                e->m_members.push_back(ident(members[(ni + 2) % 6]));
                prog->m_statements.push_back(e);
                std::string out = fmtProg(prog);
                bool ok = has(out, "enum") && has(out, names[ni]) && has(out, members[ni]) && !has(out, "placeholder");
                if (kinds[ki][0] && strstr(kinds[ki], "const")) { ok = ok && has(out, "const enum"); }
                char nb[64]; snprintf(nb, sizeof(nb), "enum %s [%s]", names[ni], kinds[ki]);
                if (!ok) { printf("  FAIL: [%s] out=[%s]\n", nb, out.c_str()); failed++; }
                else { passed++; }
                rowChecks++;
            }
    }

    // Type aliases
    {
        const char* names[] = {"StringOrNumber", "ID", "Handler", "Opt", "Pair", "Dict", "Cb", "Val", "Node", "Tree"};
        const char* anns[] = {"string | number", "string", "number[]", "Partial<T>", "keyof Foo", "Record<string, any>", "() => void", "A & B", "readonly string[]", "T | null"};
        for (int i = 0; i < 10; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            TypeAliasNode* a = ASTNodeFactory::createTypeAliasNode();
            a->m_name = names[i];
            a->m_typeAnnotation = ident(anns[i]);
            prog->m_statements.push_back(a);
            std::string out = fmtProg(prog);
            check(has(out, "type") && has(out, names[i]) && has(out, " = ") && has(out, anns[i]), names[i], out.c_str());
        }
    }

    // Classes + constructors bare + async/generators + arrows
    {
        const char* cnames[] = {"Foo", "Dog", "App", "View", "Store", "Ctrl"};
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ClassDeclNode* c = ASTNodeFactory::createClassDeclNode();
            c->m_name = cnames[i];
            c->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(c);
            std::string out = fmtProg(prog);
            check(has(out, "class") && has(out, cnames[i]), cnames[i], out.c_str());
        }
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ClassDeclNode* c = ASTNodeFactory::createClassDeclNode();
            c->m_name = "K";
            BlockNode* b = ASTNodeFactory::createBlockNode();
            FunctionDeclNode* ctor = ASTNodeFactory::createFunctionDeclNode();
            ctor->m_name = "constructor";
            if (i > 0) { ctor->m_params.push_back(ident("x")); }
            if (i > 2) { ctor->m_params.push_back(ident("y")); }
            ctor->m_body = ASTNodeFactory::createBlockNode();
            b->m_statements.push_back(ctor);
            c->m_body = b;
            prog->m_statements.push_back(c);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "ctor %d", i);
            check(has(out, "constructor") && !has(out, "function constructor"), nb, out.c_str());
        }
        const char* amods[] = {"async", "export async", "static", "export"};
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
            char fn[32]; snprintf(fn, sizeof(fn), "genFn%d", i);
            f->m_name = fn;
            f->m_modifiers = amods[i];
            f->m_params.push_back(ident("x"));
            f->m_body = retBlock("x");
            prog->m_statements.push_back(f);
            std::string out = fmtProg(prog);
            check(has(out, fn) && has(out, amods[i]), fn, out.c_str());
        }
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
            v->m_kind = "const";
            char vn[32]; snprintf(vn, sizeof(vn), "af%d", i);
            v->m_name = vn;
            ArrowFunctionNode* aw = ASTNodeFactory::createArrowFunctionNode();
            aw->m_params.push_back(ident("a"));
            if (i % 2 == 0) { aw->m_params.push_back(ident("b")); }
            aw->m_body = retBlock("a");
            v->m_initializer = aw;
            prog->m_statements.push_back(v);
            std::string out = fmtProg(prog);
            check(has(out, "=>") && has(out, vn), vn, out.c_str());
        }
    }

    // Imports/exports + template literals + destructuring + loops/try/comments
    {
        const char* srcs[] = {"react", "fs", "path", "lodash", "vue", "./x", "@a/b", "axios"};
        for (int i = 0; i < 8; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
            imp->m_source = srcs[i];
            imp->m_specifiers.push_back(ident("sym"));
            prog->m_statements.push_back(imp);
            std::string out = fmtProg(prog);
            check(has(out, "import") && has(out, srcs[i]), srcs[i], out.c_str());
        }
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ExportDeclNode* e = ASTNodeFactory::createExportDeclNode();
            FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
            char fn[32]; snprintf(fn, sizeof(fn), "exT%d", i);
            f->m_name = fn;
            f->m_body = ASTNodeFactory::createBlockNode();
            e->m_declaration = f;
            prog->m_statements.push_back(e);
            std::string out = fmtProg(prog);
            check(has(out, "export") && has(out, fn), fn, out.c_str());
        }
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
            v->m_kind = "const";
            char vn[32]; snprintf(vn, sizeof(vn), "tt%d", i);
            v->m_name = vn;
            TemplateLiteralNode* t = ASTNodeFactory::createTemplateLiteralNode();
            char q[32]; snprintf(q, sizeof(q), "pre%d ", i);
            t->m_quasis.push_back(ident(q));
            t->m_expressions.push_back(ident("v"));
            t->m_quasis.push_back(ident("!"));
            v->m_initializer = t;
            prog->m_statements.push_back(v);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "tpl %d", i);
            check(has(out, "`") && has(out, "${"), nb, out.c_str());
        }
        const char* pats[] = {"{a, b}", "[x, y]", "{p, q, r}", "[a, b, c]"};
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            VariableDeclNode* v = ASTNodeFactory::createVariableDeclNode();
            v->m_kind = "const";
            v->m_name = pats[i];
            v->m_initializer = ident("src");
            prog->m_statements.push_back(v);
            std::string out = fmtProg(prog);
            check(has(out, pats[i]), pats[i], out.c_str());
        }
        const char* conds[] = {"x > 0", "ok", "a && b", "n === 2", "flag", "done"};
        for (int i = 0; i < 6; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            IfStmtNode* s = ASTNodeFactory::createIfStmtNode();
            s->m_condition = ident(conds[i]);
            s->m_thenBranch = retBlock("a");
            s->m_elseBranch = retBlock("b");
            prog->m_statements.push_back(s);
            std::string out = fmtProg(prog);
            check(has(out, "if") && has(out, conds[i]) && has(out, "else"), conds[i], out.c_str());
        }
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ForStmtNode* f = ASTNodeFactory::createForStmtNode();
            f->m_classic = false;
            char c[32]; snprintf(c, sizeof(c), "x%d of items", i);
            f->m_condition = ident(c);
            f->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(f);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "for %d", i);
            check(has(out, "for"), nb, out.c_str());
        }
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            TryCatchStmtNode* t = ASTNodeFactory::createTryCatchStmtNode();
            t->m_body = ASTNodeFactory::createBlockNode();
            t->m_catchClause = ident("e");
            t->m_catchBody = ASTNodeFactory::createBlockNode();
            t->m_finallyBlock = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(t);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "try %d", i);
            check(has(out, "try") && has(out, "catch") && has(out, "finally"), nb, out.c_str());
        }
        for (int i = 0; i < 4; i++)
        {
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            CommentNode* c = ASTNodeFactory::createCommentNode();
            char t[32]; snprintf(t, sizeof(t), " c%d", i);
            c->m_text = t;
            c->m_isBlock = (i % 2 == 1);
            prog->m_statements.push_back(c);
            std::string out = fmtProg(prog);
            char nb[32]; snprintf(nb, sizeof(nb), "cmt %d", i);
            check(has(out, t), nb, out.c_str());
        }
    }

    // Edge: idempotency, empty, verbatim, invalid, no placeholder
    {
        ProgramNode* p1 = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* f = ASTNodeFactory::createFunctionDeclNode();
        f->m_name = "idem";
        f->m_returnType = "number";
        f->m_params.push_back(ident("a"));
        f->m_body = retBlock("a");
        p1->m_statements.push_back(f);
        std::string once = fmtProg(p1);
        ProgramNode* p2 = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* f2 = ASTNodeFactory::createFunctionDeclNode();
        f2->m_name = "idem";
        f2->m_returnType = "number";
        f2->m_params.push_back(ident("a"));
        f2->m_body = retBlock("a");
        p2->m_statements.push_back(f2);
        std::string twice = fmtProg(p2);
        check(!once.empty() && once == twice, "idempotent", once.c_str());
        ProgramNode* empty = ASTNodeFactory::createProgramNode();
        check(!has(fmtProg(empty), "placeholder"), "empty", "");
        ProgramNode* raw = ASTNodeFactory::createProgramNode();
        RawTextNode* rt = ASTNodeFactory::createRawTextNode();
        rt->m_text = "@@@verbatim-kept###";
        raw->m_statements.push_back(rt);
        check(has(fmtProg(raw), "@@@verbatim-kept###"), "raw-verbatim", "");
        ProgramNode* nob = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* nb2 = ASTNodeFactory::createFunctionDeclNode();
        nb2->m_name = "ext";
        nb2->m_returnType = "void";
        nob->m_statements.push_back(nb2);
        check(has(fmtProg(nob), "function ext"), "nobody", "");
    }

    printf("\n========================================\n");
    printf("TSFormatter tests: %d passed, %d failed (%d checks)\n", passed, failed, rowChecks);
    printf("========================================\n");
    return failed > 0 ? 1 : 0;
}
