// Easter egg: Solidity formatter tests - audited assertions, no reentrancy
#include <cstdio>
#include <cstring>
#include "fmtpp/MemoryPool.hpp"
#include "fmtpp/ASTNode.hpp"
#include "fmtpp/ASTNodeFactory.hpp"
#include "fmtpp/OutputBuffer.hpp"
#include "fmtpp/Config.hpp"
#include "fmtpp/SolidityFormatter.hpp"
#include "fmtpp/ast/ProgramNode.hpp"
#include "fmtpp/ast/FunctionDeclNode.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/ImportDeclNode.hpp"
#include "fmtpp/ast/PreprocessorDirectiveNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/IdentifierNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include "fmtpp/ast/EnumDeclNode.hpp"
#include "fmtpp/ast/VariableDeclNode.hpp"
#include "fmtpp/ast/LiteralNode.hpp"
#include "fmtpp/ast/IfStmtNode.hpp"
#include "fmtpp/ast/ForStmtNode.hpp"
#include "fmtpp/ast/WhileStmtNode.hpp"
#include "fmtpp/ast/CommentNode.hpp"
#include "fmtpp/ast/RawTextNode.hpp"

int main()
{
    MemoryPool pool;
    g_pool = &pool;

    int passed = 0;
    int failed = 0;

    // Test 1: contract with is-bases
    {
        OutputBuffer out(4096);
        Config config;
        SolidityFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "Token";
        IdentifierNode* base = ASTNodeFactory::createIdentifierNode();
        base->m_name = "ERC20";
        cls->m_superClass = base;
        IdentifierNode* second = ASTNodeFactory::createIdentifierNode();
        second->m_name = "Ownable";
        cls->m_implements.push_back(second);
        cls->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(cls);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 1 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "contract Token is ERC20, Ownable") == NULL)
        {
            printf("  FAIL: missing 'contract Token is ERC20, Ownable'\n");
            ok = false;
        }
        if (ok) { printf("Test 1 (contract): PASSED\n"); passed++; }
        else { printf("Test 1 (contract): FAILED\n"); failed++; }
    }

    // Test 2: function with modifiers and returns
    {
        OutputBuffer out(4096);
        Config config;
        SolidityFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
        func->m_name = "balanceOf";
        IdentifierNode* param = ASTNodeFactory::createIdentifierNode();
        param->m_name = "address owner";
        func->m_params.push_back(param);
        func->m_modifiers = "public view";
        func->m_returnType = "returns (uint256)";
        func->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(func);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 2 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "function balanceOf(address owner) public view returns (uint256)") == NULL)
        {
            printf("  FAIL: missing full signature\n");
            ok = false;
        }
        if (ok) { printf("Test 2 (function): PASSED\n"); passed++; }
        else { printf("Test 2 (function): FAILED\n"); failed++; }
    }

    // Test 3: quoted import and pragma passthrough
    {
        OutputBuffer out(4096);
        Config config;
        SolidityFormatter formatter(out, config);

        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        PreprocessorDirectiveNode* pragma = ASTNodeFactory::createPreprocessorDirectiveNode();
        pragma->m_directive = "pragma solidity ^0.8.0;";
        prog->m_statements.push_back(pragma);
        ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
        imp->m_source = "./SafeMath.sol";
        prog->m_statements.push_back(imp);

        formatter.format(prog);

        const char* result = out.getData();
        printf("Test 3 output: [%s]\n", result);

        bool ok = true;
        if (strstr(result, "pragma solidity ^0.8.0;") == NULL) { printf("  FAIL: missing pragma\n"); ok = false; }
        if (strstr(result, "import \"./SafeMath.sol\";") == NULL) { printf("  FAIL: missing import\n"); ok = false; }
        if (ok) { printf("Test 3 (pragma/import): PASSED\n"); passed++; }
        else { printf("Test 3 (pragma/import): FAILED\n"); failed++; }
    }

    // Test 4: table-driven funcs (modifiers x returns, 14 rows)
    {
        struct Case { const char* mods; const char* ret; };
        static const Case kCases[] = {
            {"public", "returns (uint256)"}, {"public view", "returns (uint256)"},
            {"external", ""}, {"private", "returns (bool)"},
            {"internal", "returns (address)"}, {"public payable", ""},
            {"external view", "returns (uint256)"}, {"public pure", "returns (bool)"},
            {"private view", ""}, {"external payable", ""},
            {"public", "returns (string memory)"}, {"internal pure", "returns (uint256)"},
            {"", ""}, {"public view", "returns (bool)"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            SolidityFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            FunctionDeclNode* func = ASTNodeFactory::createFunctionDeclNode();
            char fname[32];
            sprintf(fname, "sfn%02u", (unsigned)i);
            func->m_name = fname;
            if ((i % 2) == 0)
            {
                IdentifierNode* p = ASTNodeFactory::createIdentifierNode();
                p->m_name = "uint256 amount";
                func->m_params.push_back(p);
            }
            func->m_modifiers = kCases[i].mods;
            func->m_returnType = kCases[i].ret;
            func->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(func);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            if (strstr(result, "function") == NULL) { printf("  FAIL row %u: missing 'function'\n", (unsigned)i); ok = false; }
            if (strstr(result, fname) == NULL) { printf("  FAIL row %u: missing name\n", (unsigned)i); ok = false; }
            if (strstr(result, "(") == NULL) { printf("  FAIL row %u: missing '('\n", (unsigned)i); ok = false; }
            if (strstr(result, ")") == NULL) { printf("  FAIL row %u: missing ')'\n", (unsigned)i); ok = false; }
            if (strstr(result, "{") == NULL) { printf("  FAIL row %u: missing '{'\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 4 (func table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 5: table-driven contracts (kinds x bases, 10 rows)
    {
        struct Case { const char* kind; const char* name; };
        static const Case kCases[] = {
            {"", "Tok00"}, {"contract", "Tok01"}, {"interface", "ITok02"},
            {"library", "Lib03"}, {"abstract contract", "Tok04"},
            {"contract", "Tok05"}, {"interface", "ITok06"}, {"library", "Lib07"},
            {"", "Tok08"}, {"abstract contract", "Tok09"}
        };
        const size_t kCount = sizeof(kCases) / sizeof(kCases[0]);
        int tPassed = 0;
        for (size_t i = 0; i < kCount; i++)
        {
            OutputBuffer out(4096);
            Config config;
            SolidityFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
            cls->m_name = kCases[i].name;
            cls->m_kind = kCases[i].kind;
            IdentifierNode* base = ASTNodeFactory::createIdentifierNode();
            base->m_name = "BaseSol";
            cls->m_superClass = base;
            cls->m_body = ASTNodeFactory::createBlockNode();
            prog->m_statements.push_back(cls);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            if (strstr(result, kCases[i].name) == NULL) { printf("  FAIL row %u: missing name\n", (unsigned)i); ok = false; }
            if (strstr(result, "BaseSol") == NULL) { printf("  FAIL row %u: missing base\n", (unsigned)i); ok = false; }
            if (strstr(result, "is") == NULL) { printf("  FAIL row %u: missing 'is'\n", (unsigned)i); ok = false; }
            if (strstr(result, "{") == NULL) { printf("  FAIL row %u: missing '{'\n", (unsigned)i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 5 (contract table): %d/%u passed\n", tPassed, (unsigned)kCount);
    }

    // Test 6: state vars + events + enums + control flow + comments
    {
        OutputBuffer out(8192);
        Config config;
        SolidityFormatter formatter(out, config);
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        static const char* kVars[] = {
            "uint256 public totalSupply", "address public owner",
            "mapping(address => uint256) balances", "bool private paused"
        };
        for (int i = 0; i < 4; i++)
        {
            RawTextNode* v = ASTNodeFactory::createRawTextNode();
            char line[96];
            sprintf(line, "%s;", kVars[i]);
            v->m_text = line;
            prog->m_statements.push_back(v);
        }
        RawTextNode* ev = ASTNodeFactory::createRawTextNode();
        ev->m_text = "event Move(address from, uint256 n);";
        prog->m_statements.push_back(ev);
        EnumDeclNode* enm = ASTNodeFactory::createEnumDeclNode();
        enm->m_name = "Phase";
        static const char* kMembers[] = {"Idle", "Open", "Shut"};
        for (int i = 0; i < 3; i++)
        {
            IdentifierNode* m = ASTNodeFactory::createIdentifierNode();
            m->m_name = kMembers[i];
            enm->m_members.push_back(m);
        }
        prog->m_statements.push_back(enm);
        IfStmtNode* ifStmt = ASTNodeFactory::createIfStmtNode();
        IdentifierNode* ic = ASTNodeFactory::createIdentifierNode();
        ic->m_name = "paused";
        ifStmt->m_condition = ic;
        ifStmt->m_thenBranch = ASTNodeFactory::createBlockNode();
        ifStmt->m_elseBranch = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(ifStmt);
        ForStmtNode* forStmt = ASTNodeFactory::createForStmtNode();
        forStmt->m_classic = true;
        VariableDeclNode* init = ASTNodeFactory::createVariableDeclNode();
        init->m_kind = "uint256";
        init->m_name = "si";
        LiteralNode* z = ASTNodeFactory::createLiteralNode();
        z->m_type = LiteralNode::TYPE_NUMBER;
        z->m_value = "0";
        init->m_initializer = z;
        forStmt->m_init = init;
        IdentifierNode* fc = ASTNodeFactory::createIdentifierNode();
        fc->m_name = "si < 2";
        forStmt->m_condition = fc;
        IdentifierNode* fu = ASTNodeFactory::createIdentifierNode();
        fu->m_name = "si++";
        forStmt->m_update = fu;
        forStmt->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(forStmt);
        CommentNode* lc = ASTNodeFactory::createCommentNode();
        lc->m_text = " sol-line-note";
        lc->m_isBlock = false;
        prog->m_statements.push_back(lc);
        CommentNode* bc = ASTNodeFactory::createCommentNode();
        bc->m_text = " sol-block-note ";
        bc->m_isBlock = true;
        prog->m_statements.push_back(bc);
        formatter.format(prog);
        const char* result = out.getData();
        bool ok = true;
        for (int i = 0; i < 4; i++)
        {
            char want[96];
            sprintf(want, "%s;", kVars[i]);
            if (strstr(result, want) == NULL) { printf("  FAIL: missing '%s'\n", want); ok = false; }
        }
        if (strstr(result, "event Move(address from, uint256 n);") == NULL) { printf("  FAIL: event altered\n"); ok = false; }
        if (strstr(result, "enum Phase") == NULL) { printf("  FAIL: missing enum\n"); ok = false; }
        for (int i = 0; i < 3; i++)
        {
            if (strstr(result, kMembers[i]) == NULL) { printf("  FAIL: missing '%s'\n", kMembers[i]); ok = false; }
        }
        if (strstr(result, "if") == NULL) { printf("  FAIL: missing if\n"); ok = false; }
        if (strstr(result, "else") == NULL) { printf("  FAIL: missing else\n"); ok = false; }
        if (strstr(result, "uint256 si = 0") == NULL) { printf("  FAIL: missing init\n"); ok = false; }
        if (strstr(result, "si < 2") == NULL) { printf("  FAIL: missing cond\n"); ok = false; }
        if (strstr(result, "si++") == NULL) { printf("  FAIL: missing update\n"); ok = false; }
        if (strstr(result, "sol-line-note") == NULL) { printf("  FAIL: missing line comment\n"); ok = false; }
        if (strstr(result, "sol-block-note") == NULL) { printf("  FAIL: missing block comment\n"); ok = false; }
        if (ok) { printf("Test 6 (vars/events/enum/flow): PASSED\n"); passed++; }
        else { printf("Test 6 (vars/events/enum/flow): FAILED\n"); failed++; }
    }

    // Test 7: pragma/import table + empty + verbatim + idempotent
    {
        static const char* kPragmas[] = {"pragma solidity ^0.8.0;", "pragma solidity >=0.8.0 <0.9.0;"};
        static const char* kSrcs[] = {"./A.sol", "@open/B.sol"};
        int tPassed = 0;
        for (int i = 0; i < 2; i++)
        {
            OutputBuffer out(4096);
            Config config;
            SolidityFormatter formatter(out, config);
            ProgramNode* prog = ASTNodeFactory::createProgramNode();
            PreprocessorDirectiveNode* pragma = ASTNodeFactory::createPreprocessorDirectiveNode();
            pragma->m_directive = kPragmas[i];
            prog->m_statements.push_back(pragma);
            ImportDeclNode* imp = ASTNodeFactory::createImportDeclNode();
            imp->m_source = kSrcs[i];
            prog->m_statements.push_back(imp);
            formatter.format(prog);
            const char* result = out.getData();
            bool ok = true;
            if (strstr(result, kPragmas[i]) == NULL) { printf("  FAIL row %d: missing pragma\n", i); ok = false; }
            char wantImp[64];
            sprintf(wantImp, "import \"%s\";", kSrcs[i]);
            if (strstr(result, wantImp) == NULL) { printf("  FAIL row %d: missing import\n", i); ok = false; }
            if (ok) { tPassed++; passed++; }
            else { failed++; }
        }
        printf("Test 7 (pragma/import table): %d/2 passed\n", tPassed);
        OutputBuffer outEmpty(256);
        Config cEmpty;
        SolidityFormatter fEmpty(outEmpty, cEmpty);
        ProgramNode* emptyProg = ASTNodeFactory::createProgramNode();
        fEmpty.format(emptyProg);
        bool ok = true;
        if (outEmpty.getSize() != 0) { printf("  FAIL: expected empty output\n"); ok = false; }
        ProgramNode* prog = ASTNodeFactory::createProgramNode();
        RawTextNode* raw = ASTNodeFactory::createRawTextNode();
        raw->m_text = "@@@ bad sol ~~~";
        prog->m_statements.push_back(raw);
        ClassDeclNode* cls = ASTNodeFactory::createClassDeclNode();
        cls->m_name = "IdemSol";
        cls->m_body = ASTNodeFactory::createBlockNode();
        prog->m_statements.push_back(cls);
        OutputBuffer out1(4096);
        Config cc1;
        SolidityFormatter ff1(out1, cc1);
        ff1.format(prog);
        OutputBuffer out2(4096);
        Config cc2;
        SolidityFormatter ff2(out2, cc2);
        ff2.format(prog);
        if (strstr(out1.getData(), "@@@ bad sol ~~~") == NULL) { printf("  FAIL: raw altered\n"); ok = false; }
        if (strstr(out1.getData(), "contract IdemSol") == NULL) { printf("  FAIL: missing contract\n"); ok = false; }
        if (strcmp(out1.getData(), out2.getData()) != 0) { printf("  FAIL: not idempotent\n"); ok = false; }
        if (strstr(out1.getData(), "placeholder") != NULL) { printf("  FAIL: placeholder leak\n"); ok = false; }
        if (ok) { printf("Test 7b (empty/verbatim/idempotent): PASSED\n"); passed++; }
        else { printf("Test 7b (empty/verbatim/idempotent): FAILED\n"); failed++; }
    }

    printf("\n========================================\n");
    printf("SolidityFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
