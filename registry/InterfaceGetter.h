#pragma once

#include "predeclare.h"

#include "TypeRef.h"

#include <string>


class racc::registry::InterfaceGetter {
public:

    std::string name;
    TypeRef type;
    bool mut{};
    bool retMut{};
    ast::InterfaceGetterDeclaration* decl{};
    Interface* interface{};

    InterfaceGetter();

    InterfaceGetter(const InterfaceGetter&) = delete;
    InterfaceGetter& operator=(const InterfaceGetter&) = delete;
    InterfaceGetter(InterfaceGetter&&) noexcept;
    InterfaceGetter& operator=(InterfaceGetter&&) noexcept;
    ~InterfaceGetter();

    static InterfaceGetter make(ModuleRegistry &registry, ast::InterfaceGetterDeclaration &decl, Interface* interface);
};
