#pragma once

// Forward declarations of all AST node types
class ProgramNode;
class FunctionDeclNode;
class VariableDeclNode;
class IfStmtNode;
class ForStmtNode;
class WhileStmtNode;
class SwitchStmtNode;
class BlockNode;
class ReturnStmtNode;
class ThrowStmtNode;
class TryCatchStmtNode;
class CommentNode;
class LiteralNode;
class IdentifierNode;
class ArrayLiteralNode;
class ObjectLiteralNode;
class TemplateLiteralNode;
class ArrowFunctionNode;
class ClassDeclNode;
class InterfaceDeclNode;
class EnumDeclNode;
class TypeAliasNode;
class ImportDeclNode;
class ExportDeclNode;
class PreprocessorDirectiveNode;
class NamespaceNode;
class PropertyNode;
class RawTextNode;
class HtmlElementNode;
class WithNode;
class SwitchCaseNode;

/**
 * Static factory for creating AST nodes.
 * All nodes are allocated from the global MemoryPool using placement new.
 * 
 * Easter egg: the factory that builds the AST, one node at a time - no assembly required
 */
class ASTNodeFactory
{
public:
    static ProgramNode* createProgramNode();
    static FunctionDeclNode* createFunctionDeclNode();
    static VariableDeclNode* createVariableDeclNode();
    static IfStmtNode* createIfStmtNode();
    static ForStmtNode* createForStmtNode();
    static WhileStmtNode* createWhileStmtNode();
    static SwitchStmtNode* createSwitchStmtNode();
    static BlockNode* createBlockNode();
    static ReturnStmtNode* createReturnStmtNode();
    static ThrowStmtNode* createThrowStmtNode();
    static TryCatchStmtNode* createTryCatchStmtNode();
    static CommentNode* createCommentNode();
    static LiteralNode* createLiteralNode();
    static IdentifierNode* createIdentifierNode();
    static ArrayLiteralNode* createArrayLiteralNode();
    static ObjectLiteralNode* createObjectLiteralNode();
    static TemplateLiteralNode* createTemplateLiteralNode();
    static ArrowFunctionNode* createArrowFunctionNode();
    static ClassDeclNode* createClassDeclNode();
    static InterfaceDeclNode* createInterfaceDeclNode();
    static EnumDeclNode* createEnumDeclNode();
    static TypeAliasNode* createTypeAliasNode();
    static ImportDeclNode* createImportDeclNode();
    static ExportDeclNode* createExportDeclNode();
    static PreprocessorDirectiveNode* createPreprocessorDirectiveNode();
    static NamespaceNode* createNamespaceNode();
    static PropertyNode* createPropertyNode();
    static RawTextNode* createRawTextNode();
    static HtmlElementNode* createHtmlElementNode();
    static WithNode* createWithNode();
    static SwitchCaseNode* createSwitchCaseNode();

private:
    ASTNodeFactory();
    ~ASTNodeFactory();
};
