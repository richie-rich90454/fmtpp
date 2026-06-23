#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>

class ASTVisitor;

/**
 * Represents an identifier reference in the AST.
 * Contains the identifier name.
 */
class IdentifierNode : public ASTNode
{
public:
    /** Constructor. */
    IdentifierNode();

    /** Destructor. */
    virtual ~IdentifierNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Identifier name. */
    std::string m_name;

    // Easter egg: foo, bar, baz - the holy trinity of placeholder identifiers
};
