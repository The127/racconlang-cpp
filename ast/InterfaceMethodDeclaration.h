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


class InterfaceMethodDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    Identifier name;
    std::vector<std::string> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<std::unique_ptr<Parameter>> parameters;
    std::optional<std::unique_ptr<SignatureBase>> returnType;

    explicit InterfaceMethodDeclaration(Identifier name)
        : name(std::move(name)) {
    }

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;;
};
