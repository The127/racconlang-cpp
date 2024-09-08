//
// Created by zoe on 07.09.24.
//

#pragma once
#include <string>
#include <vector>
#include "ConstraintDeclaration.h"
#include "Node.h"
#include "PropertyDeclaration.h"

class StructDeclaration final : public Node {
public:
    bool isPublic = false;
    std::string name;
    std::vector<std::string> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<PropertyDeclaration> structDeclarations;
    std::vector<std::string> destructureProperties;


    [[nodiscard]] uint64_t start() const override {}
    [[nodiscard]] uint64_t end() const override {}
};
