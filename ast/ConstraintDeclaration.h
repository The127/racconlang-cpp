//
// Created by zoe on 07.09.24.
//

#pragma once

#include "Node.h"
#include "Identifier.h"
#include "GenericConstraintBase.h"

#include <string>
#include <vector>
#include <memory>
#include <optional>

class racc::ast::ConstraintDeclaration final : public Node {
public:
    uint64_t startPos;
    uint64_t endPos;
    std::optional<Identifier> name;
    std::vector<std::unique_ptr<GenericConstraintBase>> constraints;

    ConstraintDeclaration();

    ConstraintDeclaration(const ConstraintDeclaration &) = delete;

    ConstraintDeclaration &operator=(const ConstraintDeclaration &) = delete;

    ConstraintDeclaration(ConstraintDeclaration &&) noexcept;

    ConstraintDeclaration &operator=(ConstraintDeclaration &&) noexcept;

    ~ConstraintDeclaration() override;

    [[nodiscard]] uint64_t start() const override;

    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
};