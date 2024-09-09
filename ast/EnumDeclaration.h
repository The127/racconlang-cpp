//
// Created by zoe on 07.09.24.
//

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "ConstraintDeclaration.h"
#include "EnumMemberDeclaration.h"
#include "Node.h"
#include "GenericConstraintBase.h"
#include "Identifier.h"


class EnumDeclaration final :public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};
    std::optional<Identifier> name;
    std::vector<Identifier> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<EnumMemberDeclaration> memberDeclarations;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
