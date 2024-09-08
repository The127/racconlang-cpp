//
// Created by zoe on 07.09.24.
//

#pragma once
#include <memory>
#include <string>
#include <vector>

#include "SignatureBase.h"
#include "ConstraintDeclaration.h"
#include "Identifier.h"
#include "Node.h"

class FunctionDeclaration final : public Node{
public:
    uint64_t startPos;
    uint64_t endPos;
    bool isPublic = false;
    std::optional<Identifier> name;
    std::vector<Identifier> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<std::unique_ptr<SignatureBase>> parameters;
    std::unique_ptr<SignatureBase> returnType;
    //TODO: body as expression

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;
};
