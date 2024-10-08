//
// Created by zoe on 07.09.24.
//

#pragma once

#include "predeclare.h"

#include "Identifier.h"
#include "TypeSignature.h"

#include <string>
#include <vector>

class racc::ast::InterfaceDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};
    std::optional<Identifier> name;
    std::vector<Identifier> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<TypeSignature> requiredInterfaces;
    std::vector<InterfaceMethodDeclaration> methods;
    std::vector<InterfaceGetterDeclaration> getters;
    std::vector<InterfaceSetterDeclaration> setters;

    InterfaceDeclaration();

    InterfaceDeclaration(const InterfaceDeclaration &) = delete;

    InterfaceDeclaration &operator=(const InterfaceDeclaration &) = delete;

    InterfaceDeclaration(InterfaceDeclaration &&) noexcept;

    InterfaceDeclaration &operator=(InterfaceDeclaration &&) noexcept;

    ~InterfaceDeclaration() override;

    [[nodiscard]] uint64_t start() const override;

    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
};
