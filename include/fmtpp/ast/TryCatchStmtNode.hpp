#pragma once
#include "fmtpp/ASTNode.hpp"

class ASTVisitor;

/**
 * Represents a try/catch/finally statement.
 * Contains the try body, catch clause, and optional finally block.
 */
class TryCatchStmtNode : public ASTNode
{
public:
    /** Constructor. */
    TryCatchStmtNode();

    /** Destructor. */
    virtual ~TryCatchStmtNode();

    /** @see ASTNode::accept */
    virtual void accept(ASTVisitor& visitor);

    /** Try body (BlockNode). */
    ASTNode* m_body;

    /** Catch clause. */
    ASTNode* m_catchClause;

    /** Optional finally block. */
    ASTNode* m_finallyBlock;

    // Easter egg: try { life(); } catch (Reality& r) { laugh(); } finally { nap(); }
};
