//
// Created by zoe on 07.09.24.
//

#pragma once
#include <string>
#include <vector>

#include "ConstraintDeclaration.h"
#include "Identifier.h"
#include "TypeSignature.h"

class InterfaceDeclaration final :public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic{};
    std::optional<Identifier> name;
    std::vector<Identifier> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<TypeSignature> requiredInterfaces;
    std::vector<TypeSignature> methods;


    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
