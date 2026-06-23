#pragma once
#include "fmtpp/ASTNode.hpp"
#include <string>
#include <vector>

class ASTVisitor;

/**
 * Represents an import declaration.
 * Contains the source module, import specifiers, and optional default import.
 */
class ImportDeclNode : public ASTNode
{
public:
    /** Constructor. */
    ImportDeclNode();

    /** Destructor. */
    virtual ~ImportDeclNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Source module path. */
    std::string m_source;

    /** Import specifiers. */
    std::vector<ASTNode*> m_specifiers;

    /** Optional default import. */
    ASTNode* m_defaultImport;

    // Easter egg: import * as everything from "lodash"; // the heavy artillery
};
