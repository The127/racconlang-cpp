//
// Created by zoe on 07.09.24.
//

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "EnumMemberDeclaration.h"
#include "Node.h"
#include "GenericConstraintBase.h"


class EnumDeclaration final :public Node {
public:
    bool isPublic = false;
    std::string name;
    std::vector<std::string> genericParams;
    std::vector<std::unique_ptr<GenericConstraintBase>> genericConstraints;
    std::vector<EnumMemberDeclaration> memberDeclarations;


    [[nodiscard]] uint64_t start() const override {}
    [[nodiscard]] uint64_t end() const override {}
};
