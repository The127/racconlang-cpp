//
// Created by zoe on 22.09.24.
//


#pragma once

#include "predeclare.h"

#include "TypeRef.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <optional>

namespace racc::registry {
    class Struct {
    public:
        std::string name;
        std::string_view modulePath;
        uint8_t arity;
        bool isPublic;
        ast::StructDeclaration *declaration;
        std::shared_ptr<sourcemap::Source> source;
        std::shared_ptr<ast::UseMap> useMap;
        std::vector<TypeRef> genericParams;
        std::map<std::string, TypeRef, std::less<>> genericParamsMap;
        std::vector<StructMember> members;
        std::map<std::string, StructMember *, std::less<>> memberMap;
        std::optional<std::shared_ptr<Struct>> genericBase;
        WeakTypeRef type;

        Struct(std::string name, std::string_view module, uint8_t arity, ast::StructDeclaration *declaration, std::shared_ptr<sourcemap::Source> source,
               std::shared_ptr<ast::UseMap> useMap);

        Struct(const Struct &) = delete;

        Struct &operator=(const Struct &) = delete;

        Struct(Struct &&) noexcept;

        Struct &operator=(Struct &&) noexcept;

        ~Struct();

        void populate(ModuleRegistry &registry);

        [[nodiscard]] TypeRef concretize(ModuleRegistry &registry, std::vector<TypeRef> args) const;

        [[nodiscard]] TypeRef substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics) const;
    };

}
