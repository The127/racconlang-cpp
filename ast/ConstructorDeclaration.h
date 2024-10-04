//
// Created by zoe on 14.09.24.
//


#pragma once

#include "predeclare.h"

#include "Identifier.h"
#include "Node.h"

#include <vector>

class racc::ast::ConstructorDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};
    std::optional<Identifier> name;
    std::vector<Parameter> parameters;
    std::optional<Identifier> otherName;

    ConstructorDeclaration();

    ConstructorDeclaration(const ConstructorDeclaration &) = delete;

    ConstructorDeclaration &operator=(const ConstructorDeclaration &) = delete;

    ConstructorDeclaration(ConstructorDeclaration &&) noexcept;

    ConstructorDeclaration &operator=(ConstructorDeclaration &&) noexcept;

    ~ConstructorDeclaration() override;

    [[nodiscard]] uint64_t start() const override;

    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
};
