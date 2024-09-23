//
// Created by zoe on 19.09.24.
//

#include "Module.h"

#include "Struct.h"
#include "ast/EnumDeclaration.h"
#include "ast/AliasDeclaration.h"
#include "ast/InterfaceDeclaration.h"
#include "sourceMap/Source.h"

Module::~Module() = default;
Module::Module(Module &&) noexcept = default;
Module &Module::operator=(Module &&) noexcept = default;

Module::Module(const std::string path)
    : path(path) {
}

void Module::addStruct(const std::shared_ptr<Source> &source, std::string name, const uint8_t arity,
                       StructDeclaration &structDeclaration) {
    auto it = std::ranges::find_if(structs, [&](Struct &s) {
        return s.name == name && s.arity == arity;
    });
    if (it != structs.end()) {
        source->errors.emplace_back(ErrorCode::DuplicateStructDeclaration, structDeclaration.start());
        return;
    }

    structs.emplace_back(
        std::move(name),
        arity,
        &structDeclaration);
}

void Module::addEnum(const std::shared_ptr<Source> &source, std::string name, uint8_t arity,
                     EnumDeclaration &enumDeclaration) {
    auto it = std::ranges::find_if(enums, [&](Enum &e) {
        return e.name == name && e.arity == arity;
    });
    if (it != enums.end()) {
        source->errors.emplace_back(ErrorCode::DuplicateEnumDeclaration, enumDeclaration.start());
        return;
    }

    enums.emplace_back(
        std::move(name),
        arity,
        &enumDeclaration);
}

void Module::addAlias(const std::shared_ptr<Source> &source, std::string name, uint8_t arity,
                      AliasDeclaration &aliasDeclaration) {
    auto it = std::ranges::find_if(aliases, [&](Alias &a) {
        return a.name == name && a.arity == arity;
    });
    if (it != aliases.end()) {
        source->errors.emplace_back(ErrorCode::DuplicateEnumDeclaration, aliasDeclaration.start());
        return;
    }

    aliases.emplace_back(
        std::move(name),
        arity,
        &aliasDeclaration);
}

void Module::addInterface(const std::shared_ptr<Source> &source, std::string name, uint8_t arity,
                          InterfaceDeclaration &interfaceDeclaration) {
    auto it = std::ranges::find_if(interfaces, [&](Interface &i) {
        return i.name == name && i.arity == arity;
    });
    if (it != interfaces.end()) {
        source->errors.emplace_back(ErrorCode::DuplicateInterfaceDeclaration, interfaceDeclaration.start());
        return;
    }

    interfaces.emplace_back(
        std::move(name),
        arity,
        &interfaceDeclaration);
}
