#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>

class ASTVisitor;

/**
 * Represents a type alias declaration.
 * Contains the alias name and the type annotation.
 */
class TypeAliasNode : public ASTNode
{
public:
    /** Constructor. */
    TypeAliasNode();

    /** Destructor. */
    virtual ~TypeAliasNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Alias name. */
    std::string m_name;

    /** Type annotation. */
    ASTNode* m_typeAnnotation;

    // Easter egg: type StringOrNumber = string | number; - the union of two worlds
};
