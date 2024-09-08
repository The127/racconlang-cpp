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


class InterfaceMethodDeclaration final : public Node {
public:
    std::string name;
    std::vector<std::string> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<std::unique_ptr<SignatureBase>> parameterTypes;
    std::unique_ptr<SignatureBase> returnType;

    [[nodiscard]] uint64_t start() const override{};
    [[nodiscard]] uint64_t end() const override{};
};
