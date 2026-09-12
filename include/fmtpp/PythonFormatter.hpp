#pragma once
#include "fmtpp/IndentFormatterBase.hpp"

/**
 * Python code formatter.
 * def/class with : suites, if/elif/else, for/while (+else suites),
 * try/except/else/finally, with, match/case, imports, no semicolons.
 *
 * Easter egg: Python - significant whitespace, significant beauty
 */
class PythonFormatter : public IndentFormatterBase
{
public:
    PythonFormatter(OutputBuffer& out, const Config& config);
    virtual ~PythonFormatter();

    virtual void visitFunctionDecl(FunctionDeclNode& node);
    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitIfStmt(IfStmtNode& node);
    virtual void visitForStmt(ForStmtNode& node);
    virtual void visitWhileStmt(WhileStmtNode& node);
    virtual void visitSwitchStmt(SwitchStmtNode& node);
    virtual void visitTryCatchStmt(TryCatchStmtNode& node);
    virtual void visitImportDecl(ImportDeclNode& node);
    virtual void visitWith(WithNode& node);

protected:
    virtual void printEmptySuite();
    void printIfChain(IfStmtNode& node);
};
