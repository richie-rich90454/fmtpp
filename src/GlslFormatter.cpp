#include "fmtpp/GlslFormatter.hpp"
#include "fmtpp/ast/ClassDeclNode.hpp"
#include "fmtpp/ast/BlockNode.hpp"
#include <cstdio>

// Easter egg: GLSL formatter - vertex in, beautifully formatted fragment out

GlslFormatter::GlslFormatter(OutputBuffer& out, const Config& config)
    : CFormatter(out, config)
{
}

GlslFormatter::~GlslFormatter()
{
}

void GlslFormatter::visitClassDecl(ClassDeclNode& node)
{
    // GLSL: struct Name { members }; (always struct, always semicolon)
    printToken("struct");
    printSpace();
    printToken(node.m_name.c_str());
    if (node.m_body != NULL) { printSpace(); node.m_body->accept(*this); }
    printToken(";");
    printNewline();
}

// Easter egg: GlslFormatter complete - may your varyings never mismatch
