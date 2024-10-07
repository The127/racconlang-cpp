#pragma once

#include "predeclare.h"

#include "Id.h"
#include "TypeRef.h"

#include <string>

class racc::registry::InterfaceSetter {
public:
    Id name;
    TypeRef type;
    Id argName;
    bool argMut{};
    ast::InterfaceSetterDeclaration* decl{};
    Interface* interface{};

    InterfaceSetter();

    InterfaceSetter(const InterfaceSetter&) = delete;
    InterfaceSetter& operator=(const InterfaceSetter&) = delete;
    InterfaceSetter(InterfaceSetter&&) noexcept ;
    InterfaceSetter& operator=(InterfaceSetter&&) noexcept;
    ~InterfaceSetter();

    static InterfaceSetter make(ModuleRegistry &registry, ast::InterfaceSetterDeclaration &decl, Interface* interface);
};
