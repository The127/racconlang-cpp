//
// Created by zoe on 23.09.24.
//


#pragma once

#include "predeclare.h"

#include "Id.h"
#include "ModuleRegistry.h"
#include "TypeRef.h"
#include "TypeBase.h"

#include <cstdint>
#include <string>
#include <memory>
#include <optional>

class racc::registry::Enum : public TypeBase<Enum> {
public:
    Id name;
    Id modulePath;
    uint8_t arity;
    bool isPublic;
    ast::EnumDeclaration *declaration;
    std::shared_ptr<sourcemap::Source> source;
    std::shared_ptr<ast::UseMap> useMap;
    std::vector<TypeRef> genericParams;
    std::map<Id, TypeRef, std::less<>> genericParamsMap;
    std::vector<EnumMember> members;
    std::map<Id, EnumMember *, std::less<>> memberMap;
    std::optional<std::shared_ptr<Enum>> genericBase;

    Enum(Id name, Id module, uint8_t arity, ast::EnumDeclaration *declaration, std::shared_ptr<sourcemap::Source> source,
         std::shared_ptr<ast::UseMap> useMap);

    Enum(const Enum &) = delete;

    Enum &operator=(const Enum &) = delete;

    Enum(Enum &&) noexcept;

    Enum &operator=(Enum &&) noexcept;

    ~Enum();

    void populate(ModuleRegistry &registry);

    [[nodiscard]] TypeRef concretize(ModuleRegistry &registry, std::vector<TypeRef> args);

    [[nodiscard]] std::pair<TypeRef, std::shared_ptr<Enum>> substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics);
};
