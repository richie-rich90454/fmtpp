#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>

class ASTVisitor;

/**
 * Represents a code comment.
 * Contains the comment text and a flag indicating whether it is a block comment.
 */
class CommentNode : public ASTNode
{
public:
    /** Constructor. */
    CommentNode();

    /** Destructor. */
    virtual ~CommentNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Comment text content. */
    std::string m_text;

    /** True if this is a block comment (/* ... * /), false for line comment. */
    bool m_isBlock;

    // Easter egg: // TODO: fix this later - famous last words of every developer
};
