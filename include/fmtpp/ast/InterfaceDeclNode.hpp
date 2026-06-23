#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>
#include <vector>

class ASTVisitor;

/**
 * Represents an interface declaration.
 * Contains the interface name, extended interfaces, and body.
 */
class InterfaceDeclNode : public ASTNode
{
public:
    /** Constructor. */
    InterfaceDeclNode();

    /** Destructor. */
    virtual ~InterfaceDeclNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Interface name. */
    std::string m_name;

    /** Extended interfaces. */
    std::vector<ASTNode*> m_extends;

    /** Interface body (BlockNode). */
    ASTNode* m_body;

    // Easter egg: interface IReality { void exists(); } - the contract we all break
};
