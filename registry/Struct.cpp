//
// Created by zoe on 22.09.24.
//

#include "Struct.h"

#include "sourceMap/Source.h"
#include "ast/UseMap.h"
#include "ast/StructDeclaration.h"
#include "ast/PropertyDeclaration.h"
#include "ModuleRegistry.h"
#include "TypeRef.h"
#include "StructMember.h"


#include <utility>
#include <memory>
#include <algorithm>

namespace racc::registry {

    Struct::Struct(Id name,
                   Id module,
                   const uint8_t arity,
                   ast::StructDeclaration *declaration,
                   std::shared_ptr<sourcemap::Source> source,
                   std::shared_ptr<ast::UseMap> useMap)
            : name(std::move(name)),
              modulePath(module),
              arity(arity),
              isPublic(declaration->isPublic),
              declaration(declaration),
              source(std::move(source)),
              useMap(std::move(useMap)) {

        COMPILER_ASSERT(!declaration->isValue, "value structs are not implemented yet");

        for (const auto &item: declaration->genericParams) {
            auto &t = genericParams.emplace_back(TypeRef::var(item));
            const auto &[_, success] = genericParamsMap.emplace(item.name, t);
            COMPILER_ASSERT(success, "insert into genericParamsMap failed");
        }
    }

    void Struct::populate(ModuleRegistry &registry) {
        for (auto &decl: declaration->propertyDeclarations) {
            if (decl.isPublic && !isPublic) {
                source->addError(errors::CompilerError(errors::ErrorCode::PublicMemberOnNonPublicStruct, decl.start()));
            }

            auto memberName = Id(decl.name);
            auto typeResult = registry.lookupType(*decl.type, genericParamsMap, modulePath, *useMap);
            auto memberType = TypeRef::unknown();

            if (typeResult) {
                memberType = std::move(*typeResult);
            } else {
                source->addError(std::move(typeResult.error()));
            }

            if (memberMap.contains(memberName)) {
                source->addError(errors::CompilerError(errors::ErrorCode::DuplicateMemberName, decl.name.start()));
            } else {
                auto &member = members.emplace_back(memberName, &decl, memberType, decl.isPublic, decl.isMutable);
                const auto &[_, success] = memberMap.emplace(memberName, &member);

                COMPILER_ASSERT(success, "insert into memberMap failed");

                if (isPublic && member.isPublic && !member.type.isPublic()) {
                    auto err = errors::CompilerError(errors::ErrorCode::InaccessibleType, decl.type->start());
                    err.setNote("types of public struct members must be publicly accessible");
                    source->addError(std::move(err));
                }
            }
        }
    }

    TypeRef Struct::concretize(ModuleRegistry &registry, std::vector<TypeRef> args) {
        std::map<TypeRef, TypeRef> generics;
        COMPILER_ASSERT(genericParams.size() == args.size(), "wrong number of generic arguments");
        for (size_t i = 0; i < args.size(); ++i) {
            const auto &[_, success] = generics.emplace(genericParams[i], args[i]);
            COMPILER_ASSERT(success, "insert into generics failed");
        }
        auto [typeRef, s] = substituteGenerics(registry, generics);
        if (isPublic) {
            s->isPublic = std::ranges::all_of(args, [](const auto &t) { return t.isPublic(); });
        }
        return typeRef;
    }

    std::pair<TypeRef, std::shared_ptr<Struct>> Struct::substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics) {
        auto [typeRef, s] = TypeRef::makeStruct(name, modulePath, arity, declaration, source, useMap);
        for (const auto &item: members) {
            auto &member = s->members.emplace_back(item.name, item.decl, item.type.substituteGenerics(registry, generics), item.isPublic, item.isMutable);
            const auto &[_, success] = s->memberMap.emplace(item.name, &member);
            COMPILER_ASSERT(success, "insert into s.memberMap failed");
        }
        s->genericBase = genericBase.value_or(ptr());
        return {typeRef, s};
    }

    Struct::~Struct() = default;

    Struct::Struct(Struct &&) noexcept = default;

    Struct &Struct::operator=(Struct &&) noexcept = default;

}
