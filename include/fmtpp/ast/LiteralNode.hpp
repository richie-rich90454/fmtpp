#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>

class ASTVisitor;

/**
 * Represents a literal value in the AST.
 * Contains the literal value as a string and an enum indicating the literal type.
 */
class LiteralNode : public ASTNode
{
public:
    /** Enumeration of literal value types. */
    enum Type
    {
        TYPE_STRING,
        TYPE_NUMBER,
        TYPE_BOOLEAN,
        TYPE_NULL,
        TYPE_REGEX
    };

    /** Constructor. */
    LiteralNode();

    /** Destructor. */
    virtual ~LiteralNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Literal value as a string. */
    std::string m_value;

    /** Type of this literal. */
    Type m_type;

    // Easter egg: "Hello, World!" - the first literal every programmer meets
};
