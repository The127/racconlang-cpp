//
// Created by zoe on 07.09.24.
//


#pragma once
#include <memory>
#include <string>
#include <vector>

#include "SignatureBase.h"
#include "Node.h"
#include "ConstraintDeclaration.h"
#include "Parameter.h"
#include "ReturnType.h"


class InterfaceMethodDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isMut{};
    std::optional<Identifier> name;
    std::vector<Identifier> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<Parameter> parameters;
    std::optional<ReturnType> returnType;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
