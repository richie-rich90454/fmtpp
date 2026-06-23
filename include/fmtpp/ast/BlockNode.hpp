#pragma once
#include "fmtpp/ASTNode.hpp"
#include <vector>

class ASTVisitor;

/**
 * Represents a block statement (curly-brace delimited).
 * Contains a list of statements within the block.
 *
 * Easter egg: { } - the empty block, where dreams of code go to rest
 */
class BlockNode : public ASTNode
{
public:
    /** Constructor. */
    BlockNode();

    /** Destructor. */
    virtual ~BlockNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Statements within the block. */
    std::vector<ASTNode*> m_statements;
};