//
// Created by zoe on 23.09.24.
//


#pragma once

#include "predeclare.h"

#include "ModuleRegistry.h"
#include "TypeRef.h"

#include <cstdint>
#include <string>
#include <memory>
#include <optional>

namespace racc::registry {
    class Enum {
    public:
        std::string name;
        std::string_view modulePath;
        uint8_t arity;
        bool isPublic;
        ast::EnumDeclaration *declaration;
        std::shared_ptr<sourcemap::Source> source;
        std::shared_ptr<ast::UseMap> useMap;
        std::vector<TypeRef> genericParams;
        std::map<std::string, TypeRef, std::less<>> genericParamsMap;
        std::vector<EnumMember> members;
        std::map<std::string, EnumMember *, std::less<>> memberMap;
        std::optional<std::shared_ptr<Enum>> genericBase;
        WeakTypeRef type;

        Enum(std::string name, std::string_view module, uint8_t arity, ast::EnumDeclaration *declaration, std::shared_ptr<sourcemap::Source> source,
             std::shared_ptr<ast::UseMap> useMap);

        Enum(const Enum &) = delete;

        Enum &operator=(const Enum &) = delete;

        Enum(Enum &&) noexcept;

        Enum &operator=(Enum &&) noexcept;

        ~Enum();

        void populate(ModuleRegistry &registry);

        [[nodiscard]] TypeRef concretize(ModuleRegistry &registry, std::vector<TypeRef> args) const;

        [[nodiscard]] TypeRef substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics) const;
    };
}