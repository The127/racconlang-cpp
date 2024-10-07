//
// Created by zoe on 23.09.24.
//

#include "Enum.h"

#include "EnumMember.h"
#include "ast/UseMap.h"
#include "ast/EnumDeclaration.h"
#include "ast/EnumMemberDeclaration.h"
#include "sourceMap/Source.h"
#include "ModuleRegistry.h"
#include "TypeRef.h"

#include <utility>
#include <memory>
#include <algorithm>

namespace racc::registry {

    Enum::Enum(std::string name, std::string_view module, uint8_t arity, ast::EnumDeclaration *declaration, std::shared_ptr<sourcemap::Source> source,
               std::shared_ptr<ast::UseMap> useMap)
            : name(std::move(name)),
              modulePath(module),
              arity(arity),
              isPublic(declaration->isPublic),
              declaration(declaration),
              source(std::move(source)),
              useMap(std::move(useMap)) {

        for (const auto &item: declaration->genericParams) {
            auto &t = genericParams.emplace_back(TypeRef::var(std::string(item.name)));
            const auto &[_, success] = genericParamsMap.emplace(item.name, t);
            COMPILER_ASSERT(success, "insert into genericParamsMap failed");
        }
    }

    void Enum::populate(ModuleRegistry &registry) {
        for (auto &decl: declaration->memberDeclarations) {
            auto memberName = std::string(decl.name.name);
            std::vector<TypeRef> types;
            for (const auto &sig: decl.values) {
                auto typeResult = registry.lookupType(sig, genericParamsMap, modulePath, *useMap);
                if (!typeResult) {
                    source->addError(std::move(typeResult.error()));
                    types.emplace_back(TypeRef::unknown());
                } else {
                    types.emplace_back(std::move(*typeResult));
                }
            }
            auto &member = members.emplace_back(memberName, &decl, types);
            const auto &[_, success] = memberMap.emplace(memberName, &member);
            COMPILER_ASSERT(success, "insert into memberMap failed");

            if (isPublic) {
                for (auto it = member.types.begin(); it != member.types.end(); it++) {
                    if (!it->isPublic()) {
                        auto err = errors::CompilerError(errors::ErrorCode::InaccessibleType, member.decl->values[std::distance(member.types.begin(), it)].start());
                        err.setNote("types of enum values of public enums must be publicly accessible");
                        source->addError(std::move(err));
                    }
                }
            }
        }
    }

    TypeRef Enum::concretize(ModuleRegistry &registry, std::vector<TypeRef> args) {
        std::map<TypeRef, TypeRef> generics;
        COMPILER_ASSERT(genericParams.size() == args.size(), "wrong number of generic arguments");
        for (size_t i = 0; i < args.size(); ++i) {
            const auto &[_, success] = generics.emplace(genericParams[i], args[i]);
            COMPILER_ASSERT(success, "insert into generics failed");
        }
        auto [typeRef, e] = substituteGenerics(registry, generics);
        if (isPublic) {
            e->isPublic = std::ranges::all_of(args, [](const auto &t) { return t.isPublic(); });
        }
        return typeRef;
    }

    std::pair<TypeRef, std::shared_ptr<Enum>> Enum::substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics) {
        auto [typeRef, e] = TypeRef::makeEnum(name, modulePath, 0, declaration, source, useMap);
        for (const auto &item: members) {
            std::vector<TypeRef> memberTypes;
            for (const auto &t: item.types) {
                memberTypes.emplace_back(t.substituteGenerics(registry, generics));
            }
            auto &member = e->members.emplace_back(item.name, item.decl, memberTypes);
            const auto &[_, success] = e->memberMap.emplace(member.name, &member);
            COMPILER_ASSERT(success, "insert into s.memberMap failed");
        }
        e->genericBase = genericBase.value_or(ptr());
        return {typeRef, e};
    }

    Enum::~Enum() = default;

    Enum::Enum(Enum &&) noexcept = default;

    Enum &Enum::operator=(Enum &&) noexcept = default;

}
