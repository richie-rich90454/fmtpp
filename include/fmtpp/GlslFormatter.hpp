#pragma once
#include "fmtpp/CFormatter.hpp"

/**
 * GLSL shader formatter.
 * Extends CFormatter (C-style functions). Structs keep their kind and
 * trailing semicolon, uniforms ride in the variable kind, #version and
 * precision statements pass through verbatim.
 *
 * Easter egg: GLSL - formatting shaders at 60 frames per second
 */
class GlslFormatter : public CFormatter
{
public:
    GlslFormatter(OutputBuffer& out, const Config& config);
    virtual ~GlslFormatter();

    virtual void visitClassDecl(ClassDeclNode& node);
};
