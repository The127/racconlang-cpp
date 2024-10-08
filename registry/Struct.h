//
// Created by zoe on 22.09.24.
//


#pragma once

#include "predeclare.h"

#include "Id.h"
#include "TypeRef.h"
#include "TypeBase.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <optional>

class racc::registry::Struct : public TypeBase<Struct> {
public:
    Id name;
    Id modulePath;
    uint8_t arity;
    bool isPublic;
    ast::StructDeclaration *declaration;
    std::shared_ptr<sourcemap::Source> source;
    std::shared_ptr<ast::UseMap> useMap;
    std::vector<TypeRef> genericParams;
    std::map<Id, TypeRef, std::less<>> genericParamsMap;
    std::vector<StructMember> members;
    std::map<Id, StructMember *, std::less<>> memberMap;
    std::optional<std::shared_ptr<Struct>> genericBase;

    Struct(Id name, Id module, uint8_t arity, ast::StructDeclaration *declaration, std::shared_ptr<sourcemap::Source> source,
           std::shared_ptr<ast::UseMap> useMap);

    Struct(const Struct &) = delete;

    Struct &operator=(const Struct &) = delete;

    Struct(Struct &&) noexcept;

    Struct &operator=(Struct &&) noexcept;

    ~Struct();

    void populate(ModuleRegistry &registry);

    [[nodiscard]] TypeRef concretize(ModuleRegistry &registry, std::vector<TypeRef> args);

    [[nodiscard]] std::pair<TypeRef, std::shared_ptr<Struct>> substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics);
};
