//
// Created by zoe on 07.09.24.
//

#pragma once
#include <memory>
#include <string>
#include <vector>

#include "BaseSignature.h"
#include "ConstraintDeclaration.h"
#include "Node.h"

class FunctionDeclaration final : Node{
public:
    bool isPublic = false;
    std::string name;
    std::vector<std::string> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<BaseSignature> parameters;
    std::shared_ptr<BaseSignature> returnType;
    //TODO: body as expression

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;
};
