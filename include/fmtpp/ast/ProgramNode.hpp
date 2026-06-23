#pragma once
#include "fmtpp/ASTNode.hpp"
#include <vector>

class ASTVisitor;

/**
 * Represents the root program node of the AST.
 * Contains a list of top-level statements.
 *
 * Easter egg: the program node - where every source file begins its journey
 */
class ProgramNode : public ASTNode
{
public:
    /** Constructor. */
    ProgramNode();

    /** Destructor. */
    virtual ~ProgramNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Top-level statements. */
    std::vector<ASTNode*> m_statements;
};