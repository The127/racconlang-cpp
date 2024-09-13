//
// Created by zoe on 07.09.24.
//

#pragma once
#include <string>
#include <vector>
#include <memory>

#include "Identifier.h"
#include "Parameter.h"
#include "SignatureBase.h"


class EnumMemberDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    Identifier name;
    std::vector<std::unique_ptr<SignatureBase>> values;

    explicit EnumMemberDeclaration(Identifier name);
    EnumMemberDeclaration(EnumMemberDeclaration&&) noexcept;
    EnumMemberDeclaration& operator=(EnumMemberDeclaration&&) noexcept;
    ~EnumMemberDeclaration() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};

