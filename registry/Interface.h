//
// Created by zoe on 23.09.24.
//


#pragma once

#include "ModuleRegistry.h"

#include <cstdint>
#include <string>
#include <memory>
#include <optional>

class InterfaceDeclaration;
class UseMap;
class Source;

class Interface {
public:
    std::string name;
    std::string_view modulePath;
    uint8_t arity;
    InterfaceDeclaration *declaration;
    std::shared_ptr<Source> source;
    std::shared_ptr<UseMap> useMap;
    std::optional<std::shared_ptr<Interface>> genericBase;
    WeakTypeRef type;

    Interface(std::string name, std::string_view module, uint8_t arity, InterfaceDeclaration *declaration, std::shared_ptr<Source> source,
              std::shared_ptr<UseMap> useMap);
    ~Interface();
    Interface(Interface &&) noexcept;
    Interface &operator=(Interface &&) noexcept;

    void populate(ModuleRegistry& registry);

    TypeRef concretize(ModuleRegistry& registry, const std::vector<TypeRef>&) const;
};
