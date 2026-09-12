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

    printf("\n========================================\n");
    printf("SolidityFormatter tests: %d passed, %d failed\n", passed, failed);
    printf("========================================\n");

    if (failed > 0) { return 1; }
    return 0;
}
