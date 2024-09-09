//
// Created by zoe on 07.09.24.
//


#pragma once


#include <optional>
#include <vector>

#include "ConstraintDeclaration.h"
#include "Identifier.h"
#include "Node.h"

class AliasDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};
    std::optional<Identifier> name;
    std::vector<Identifier> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    
    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
