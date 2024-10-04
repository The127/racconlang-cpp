//
// Created by zoe on 07.09.24.
//

#pragma once

#include "predeclare.h"

#include "Identifier.h"
#include "Node.h"

#include <string>
#include <vector>

class racc::ast::StructDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};
    bool isValue{};
    std::optional<Identifier> name;
    std::vector<Identifier> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<PropertyDeclaration> propertyDeclarations;
    std::vector<Identifier> destructureProperties;

    StructDeclaration();

    StructDeclaration(const StructDeclaration &) = delete;

    StructDeclaration &operator=(const StructDeclaration &) = delete;

    StructDeclaration(StructDeclaration &&) noexcept;

    StructDeclaration &operator=(StructDeclaration &&) noexcept;

    ~StructDeclaration() override;

    [[nodiscard]] uint64_t start() const override;

    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
};