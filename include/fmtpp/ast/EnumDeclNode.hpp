#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>
#include <vector>

class ASTVisitor;

/**
 * Represents an enum declaration.
 * Contains the enum name and member list.
 */
class EnumDeclNode : public ASTNode
{
public:
    /** Constructor. */
    EnumDeclNode();

    /** Destructor. */
    virtual ~EnumDeclNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Enum name. */
    std::string m_name;

    /** Enum members. */
    std::vector<ASTNode*> m_members;

    // Easter egg: enum { RED, GREEN, BLUE } - the RGB of programming
};
