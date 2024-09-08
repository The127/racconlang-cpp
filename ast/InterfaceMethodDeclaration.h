//
// Created by zoe on 07.09.24.
//


#pragma once
#include <memory>
#include <string>
#include <vector>

#include "BaseSignature.h"
#include "Node.h"


class ConstraintDeclaration;

class InterfaceMethodDeclaration final : Node {
public:
    std::string name;
    std::vector<std::string> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<BaseSignature> parameterTypes;
    std::shared_ptr<BaseSignature> returnType;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;
};
