//
// Created by zoe on 07.09.24.
//

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Identifier.h"
#include "Signature.h"

class racc::ast::EnumMemberDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    Identifier name;
    std::vector<Signature> values;

    explicit EnumMemberDeclaration(Identifier name);

    EnumMemberDeclaration(const EnumMemberDeclaration &) = delete;

    EnumMemberDeclaration &operator=(const EnumMemberDeclaration &) = delete;

    EnumMemberDeclaration(EnumMemberDeclaration &&) noexcept;

    EnumMemberDeclaration &operator=(EnumMemberDeclaration &&) noexcept;

    ~EnumMemberDeclaration() override;

    [[nodiscard]] uint64_t start() const override;

    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
};
