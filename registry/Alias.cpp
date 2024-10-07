//
// Created by zoe on 23.09.24.
//

#include "Alias.h"

#include "ast/AliasDeclaration.h"
#include "ast/UseMap.h"
#include "sourceMap/Source.h"
#include "TypeRef.h"

#include <utility>
#include <algorithm>

namespace racc::registry {

    Alias::Alias(std::string name, std::string_view module, uint8_t arity, ast::AliasDeclaration *declaration, std::shared_ptr<sourcemap::Source> source,
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

    void Alias::populate(ModuleRegistry &registry) {
        if (declaration->signature) {
            auto res = registry.lookupType(*declaration->signature, genericParamsMap, modulePath, *useMap);
            if (res) {
                aliasedType = std::make_unique<TypeRef>(std::move(*res));
            } else {
                source->addError(std::move(res.error()));
            }
        }
        if (!aliasedType) {
            aliasedType = std::make_unique<TypeRef>(TypeRef::unknown());
        }
    }

    TypeRef Alias::concretize(ModuleRegistry &registry, std::vector<TypeRef> args) {
        std::map<TypeRef, TypeRef> generics;
        COMPILER_ASSERT(genericParams.size() == args.size(), "wrong number of generic arguments");
        for (size_t i = 0; i < args.size(); ++i) {
            const auto &[_, success] = generics.emplace(genericParams[i], args[i]);
            COMPILER_ASSERT(success, "insert into generics failed");
        }
        auto [typeRef, alias] = substituteGenerics(registry, generics);
        if (isPublic) {
            alias->isPublic = std::ranges::all_of(args, [](const auto &t) { return t.isPublic(); });
        }
        return typeRef;
    }

    std::pair<TypeRef, std::shared_ptr<Alias>> Alias::substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics) {
        auto [typeRef, alias] = TypeRef::makeAlias(name, modulePath, 0, declaration, source, useMap);
        alias->aliasedType = std::make_unique<TypeRef>(aliasedType->substituteGenerics(registry, generics));
        alias->genericBase = genericBase.value_or(ptr());
        return {typeRef, alias};
    }

    Alias::~Alias() = default;

    Alias::Alias(Alias &&) noexcept = default;

    Alias &Alias::operator=(Alias &&) noexcept = default;

}
