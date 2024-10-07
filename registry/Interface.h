//
// Created by zoe on 23.09.24.
//


#pragma once

#include "predeclare.h"

#include "Id.h"
#include "ModuleRegistry.h"
#include "TypeBase.h"

#include <cstdint>
#include <string>
#include <memory>
#include <optional>

class racc::registry::Interface : public TypeBase<Interface> {
public:
    Id name;
    Id modulePath;
    uint8_t arity;
    bool isPublic;
    std::map<Id, InterfaceGetter, std::less<>> getters;
    std::map<Id, InterfaceSetter, std::less<>> setters;
    std::map<Id, InterfaceMethod, std::less<>> methods;
    ast::InterfaceDeclaration *declaration;
    std::shared_ptr<sourcemap::Source> source;
    std::shared_ptr<ast::UseMap> useMap;
    std::vector<TypeRef> genericParams;
    std::map<Id, TypeRef, std::less<>> genericParamsMap;
    std::optional<std::shared_ptr<Interface>> genericBase;

    Interface(Id name, Id module, uint8_t arity, ast::InterfaceDeclaration *declaration, std::shared_ptr<sourcemap::Source> source,
              std::shared_ptr<ast::UseMap> useMap);

    ~Interface();

    Interface(Interface &&) noexcept;

    Interface &operator=(Interface &&) noexcept;

    void populate(ModuleRegistry &registry);

    TypeRef concretize(ModuleRegistry &registry, const std::vector<TypeRef> &) const;
};
