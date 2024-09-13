//
// Created by zoe on 07.09.24.
//

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Node.h"
#include "Identifier.h"


class EnumMemberDeclaration;
class ConstraintDeclaration;

class EnumDeclaration final :public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};
    std::optional<Identifier> name;
    std::vector<Identifier> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<EnumMemberDeclaration> memberDeclarations;

    EnumDeclaration();
    EnumDeclaration(EnumDeclaration&&) noexcept;
    EnumDeclaration& operator=(EnumDeclaration&&) noexcept;
    ~EnumDeclaration() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
