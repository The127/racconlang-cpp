//
// Created by zoe on 23.09.24.
//


#pragma once

#include "ModuleRegistry.h"
#include "TypeRef.h"

#include <cstdint>
#include <memory>
#include <string>
#include <optional>


class Source;
class AliasDeclaration;
class UseMap;

class Alias {
public:
    std::string name;
    std::string_view modulePath;
    uint8_t arity;
    bool isPublic;
    std::unique_ptr<TypeRef> aliasedType;
    AliasDeclaration* declaration;
    std::shared_ptr<Source> source;
    std::shared_ptr<UseMap> useMap;
    std::vector<TypeRef> genericParams;
    std::map<std::string, TypeRef, std::less<>> genericParamsMap;
    std::optional<std::shared_ptr<Alias>> genericBase;
    WeakTypeRef type;

    Alias(std::string name, std::string_view module, uint8_t arity, AliasDeclaration* declaration, std::shared_ptr<Source> source, std::shared_ptr<UseMap> useMap);
    Alias(const Alias&) = delete;
    Alias& operator=(const Alias&) = delete;
    Alias(Alias&&) noexcept;
    Alias& operator=(Alias&&) noexcept;
    ~Alias();

    void populate(ModuleRegistry& registry);

    TypeRef concretize(ModuleRegistry& registry, std::vector<TypeRef> args) const;

    TypeRef substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef>& generics) const;
};
