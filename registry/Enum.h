//
// Created by zoe on 23.09.24.
//


#pragma once

#include "ModuleRegistry.h"
#include "TypeRef.h"

#include <cstdint>
#include <string>
#include <memory>
#include <optional>


class EnumDeclaration;

class UseMap;

class Source;

class EnumMember;

class Enum {
public:
    std::string name;
    std::string_view modulePath;
    uint8_t arity;
    bool isPublic;
    EnumDeclaration *declaration;
    std::shared_ptr<Source> source;
    std::shared_ptr<UseMap> useMap;
    std::vector<TypeRef> genericParams;
    std::map<std::string, TypeRef, std::less<>> genericParamsMap;
    std::vector<EnumMember> members;
    std::map<std::string, EnumMember *, std::less<>> memberMap;
    std::optional<std::shared_ptr<Enum>> genericBase;
    WeakTypeRef type;

    Enum(std::string name, std::string_view module, uint8_t arity, EnumDeclaration *declaration, std::shared_ptr<Source> source,
         std::shared_ptr<UseMap> useMap);

    Enum(const Enum &) = delete;

    Enum &operator=(const Enum &) = delete;

    Enum(Enum &&) noexcept;

    Enum &operator=(Enum &&) noexcept;

    ~Enum();

    void populate(ModuleRegistry &registry);

    [[nodiscard]] TypeRef concretize(ModuleRegistry &registry, std::vector<TypeRef> args) const;

    [[nodiscard]] TypeRef substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics) const;
};
