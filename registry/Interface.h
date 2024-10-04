//
// Created by zoe on 23.09.24.
//


#pragma once

#include "predeclare.h"

#include "ModuleRegistry.h"

#include <cstdint>
#include <string>
#include <memory>
#include <optional>

class racc::registry::Interface {
public:
    std::string name;
    std::string_view modulePath;
    uint8_t arity;
    bool isPublic;
    std::map<std::string, ast::InterfaceGetterDeclaration, std::less<>> getters;
    std::map<std::string, ast::InterfaceSetterDeclaration, std::less<>> setters;
    std::map<std::string, InterfaceMethod, std::less<>> methods;
    ast::InterfaceDeclaration *declaration;
    std::shared_ptr<sourcemap::Source> source;
    std::shared_ptr<ast::UseMap> useMap;
    std::vector<TypeRef> genericParams;
    std::map<std::string, TypeRef, std::less<>> genericParamsMap;
    std::optional<std::shared_ptr<Interface>> genericBase;
    WeakTypeRef type;

    Interface(std::string name, std::string_view module, uint8_t arity, ast::InterfaceDeclaration *declaration, std::shared_ptr<sourcemap::Source> source,
              std::shared_ptr<ast::UseMap> useMap);

    ~Interface();

    Interface(Interface &&) noexcept;

    Interface &operator=(Interface &&) noexcept;

    void populate(ModuleRegistry &registry);

    TypeRef concretize(ModuleRegistry &registry, const std::vector<TypeRef> &) const;
};
