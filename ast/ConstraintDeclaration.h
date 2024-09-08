//
// Created by zoe on 07.09.24.
//

#pragma once

#include<string>
#include <vector>
#include <memory>

#include "Node.h"
#include "GenericConstraintBase.h"
#include "Identifier.h"


class ConstraintDeclaration final : public Node {
public:
    uint64_t startPos;
    uint64_t endPos;
    std::optional<Identifier> name;
    std::vector<std::unique_ptr<GenericConstraintBase>> constraints;


    [[nodiscard]] uint64_t start() const override;

    [[nodiscard]] uint64_t end() const override;
};
