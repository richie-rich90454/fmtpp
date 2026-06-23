#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>

class ASTVisitor;

/**
 * Represents a namespace declaration.
 * Contains the namespace name and body.
 */
class NamespaceNode : public ASTNode
{
public:
    /** Constructor. */
    NamespaceNode();

    /** Destructor. */
    virtual ~NamespaceNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Namespace name. */
    std::string m_name;

    /** Namespace body (BlockNode). */
    ASTNode* m_body;

    // Easter egg: namespace std { } // where all C++ dreams come true
};
