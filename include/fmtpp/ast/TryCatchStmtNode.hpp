#pragma once
#include "fmtpp/ASTNode.hpp"
#include <vector>

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

    /** Catch body (BlockNode). NULL when absent. */
    ASTNode* m_catchBody;

    /** Further catch clauses for multi-catch (each a TryCatchStmtNode
        carrying its own m_catchClause/m_catchBody). */
    std::vector<ASTNode*> m_extraCatches;

    /** Optional else suite (Python try/else). NULL when absent. */
    ASTNode* m_elseBlock;

    /** Optional finally block. */
    ASTNode* m_finallyBlock;

    /**
     * True when `catch` started on its own line (Allman style:
     * `}` newline `catch`). Printers join it with a space otherwise.
     */
    bool m_catchOnNewLine = false;

    /**
     * True when `finally` started on its own line. Printers join it
     * with a space otherwise.
     */
    bool m_finallyOnNewLine = false;

    // Easter egg: try { life(); } catch (Reality& r) { laugh(); } finally { nap(); }
};
