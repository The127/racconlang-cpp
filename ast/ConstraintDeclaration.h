//
// Created by zoe on 07.09.24.
//

#pragma once

#include<string>
#include <vector>
#include <memory>

#include "Node.h"
#include "GenericConstraintBase.h"


class ConstraintDeclaration final : public Node {
public:
    std::string name;
    std::vector<std::unique_ptr<GenericConstraintBase>> constraints;


    [[nodiscard]] uint64_t start() const override {}

    [[nodiscard]] uint64_t end() const override {}
};
