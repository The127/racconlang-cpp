//
// Created by zoe on 22.09.24.
//

#include "Struct.h"

#include "sourceMap/Source.h"
#include "ast/UseMap.h"
#include "ast/StructDeclaration.h"
#include "ast/PropertyDeclaration.h"
#include "ModuleRegistry.h"
#include "TypeRefImpl.h"
#include "StructMember.h"


#include <utility>
#include <memory>
#include <algorithm>


Struct::Struct(std::string name,
               std::string_view module,
               const uint8_t arity,
               StructDeclaration *declaration,
               std::shared_ptr<Source> source,
               std::shared_ptr<UseMap> useMap)
        : name(std::move(name)),
          modulePath(module),
          arity(arity),
          declaration(declaration),
          source(std::move(source)),
          useMap(std::move(useMap)),
          isPublic(declaration->isPublic) {

    COMPILER_ASSERT(!declaration->isValue, "value structs are not implemented yet");

    for (const auto &item: declaration->genericParams) {
        auto &t = genericParams.emplace_back(TypeRef::var(std::string(item.name)));
        const auto &[_, success] = genericParamsMap.emplace(item.name, t);
        COMPILER_ASSERT(success, "insert into genericParamsMap failed");
    }
}

void Struct::populate(ModuleRegistry &registry) {
    for (auto &decl: declaration->propertyDeclarations) {
        if (decl.isPublic && !isPublic) {
            source->addError(CompilerError(ErrorCode::PublicMemberOnNonPublicStruct, decl.start()));
        }

        auto memberName = std::string(decl.name.name);
        auto typeResult = registry.lookupType(*decl.type, genericParamsMap, modulePath, *useMap);
        auto memberType = TypeRef::unknown();

        if (typeResult) {
            memberType = std::move(*typeResult);
        } else {
            source->addError(std::move(typeResult.error()));
        }

        auto &member = members.emplace_back(memberName, &decl, memberType, decl.isPublic, decl.isMutable);
        const auto &[_, success] = memberMap.emplace(memberName, &member);
        COMPILER_ASSERT(success, "insert into memberMap failed");

        if (isPublic && member.isPublic && !member.type.isPublic()) {
            auto err = CompilerError(ErrorCode::InaccessibleType, decl.type->start());
            err.setNote("types of public struct members must be publicly accessible");
            source->addError(std::move(err));
        }
    }
}

TypeRef Struct::concretize(ModuleRegistry &registry, std::vector<TypeRef> args) const {
    std::map<TypeRef, TypeRef> generics;
    COMPILER_ASSERT(genericParams.size() == args.size(), "wrong number of generic arguments");
    for (size_t i = 0; i < args.size(); ++i) {
        const auto &[_, success] = generics.emplace(genericParams[i], args[i]);
        COMPILER_ASSERT(success, "insert into generics failed");
    }
    auto concretized = substituteGenerics(registry, generics);
    if (isPublic) {
        auto s = *concretized.as<Struct>();
        s->isPublic = std::ranges::all_of(args, [](const auto &t) { return t.isPublic(); });
    }
    return concretized;
}

TypeRef Struct::substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics) const {
    auto typeRef = TypeRef::make<Struct>(name, modulePath, arity, declaration, source, useMap);
    auto s = *typeRef.as<Struct>();
    for (const auto &item: members) {
        auto &member = s->members.emplace_back(item.name, item.decl, item.type.substituteGenerics(registry, generics), item.isPublic, item.isMutable);
        const auto &[_, success] = s->memberMap.emplace(item.name, &member);
        COMPILER_ASSERT(success, "insert into s.memberMap failed");
    }
    s->genericBase = genericBase.value_or(*TypeRef(this->type).as<Struct>());
    return typeRef;
}

Struct::~Struct() = default;

Struct::Struct(Struct &&) noexcept = default;

Struct &Struct::operator=(Struct &&) noexcept = default;
