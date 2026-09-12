#pragma once
#include "fmtpp/CFormatter.hpp"

/**
 * Objective-C code formatter.
 * Extends CFormatter (plain C passes through). Classes print as
 * @interface/@implementation blocks ending in @end, methods keep
 * their raw header text, #import passes through verbatim.
 *
 * Easter egg: Objective-C - brackets within brackets within brackets
 */
class ObjectiveCFormatter : public CFormatter
{
public:
    ObjectiveCFormatter(OutputBuffer& out, const Config& config);
    virtual ~ObjectiveCFormatter();

    virtual void visitFunctionDecl(FunctionDeclNode& node);
    virtual void visitClassDecl(ClassDeclNode& node);
    virtual void visitInterfaceDecl(InterfaceDeclNode& node);
};
