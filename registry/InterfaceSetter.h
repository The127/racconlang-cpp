#pragma once

#include "predeclare.h"

#include "TypeRef.h"

#include <string>

class racc::registry::InterfaceSetter {
public:
    std::string name;
    TypeRef type;
    std::string argName;
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
