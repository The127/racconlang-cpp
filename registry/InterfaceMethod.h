#pragma once

#include "predeclare.h"

#include "TypeRef.h"

#include <string>
#include <memory>

class racc::registry::InterfaceMethod {
private:
    InterfaceMethod();

public:
    std::string name;
    bool returnMut{};
    bool isMut{};
    std::vector<Parameter> params;
    TypeRef returnType;
    std::weak_ptr<Interface> interface;
    ast::InterfaceMethodDeclaration *decl{};
    std::vector<TypeRef> genericParams;
    std::map<std::string, TypeRef, std::less<>> genericParamsMap;
    std::optional<std::shared_ptr<Interface>> genericBase;

    ~InterfaceMethod();
    InterfaceMethod(const InterfaceMethod &) = delete;
    InterfaceMethod &operator=(const InterfaceMethod &) = delete;
    InterfaceMethod(InterfaceMethod &&) noexcept;
    InterfaceMethod &operator=(InterfaceMethod &&) noexcept;


    static InterfaceMethod make(ModuleRegistry &registry, ast::InterfaceMethodDeclaration& decl, Interface& interface);
};
