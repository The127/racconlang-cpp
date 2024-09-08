//
// Created by zoe on 07.09.24.
//

#pragma once
#include <string>
#include <vector>

#include "ConstraintDeclaration.h"
#include "TypeSignature.h"

class InterfaceDeclaration final :public Node {
public:
    bool isPublic = false;
    std::string name;
    std::vector<std::string> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<TypeSignature> requiredInterfaces;
    std::vector<TypeSignature> methods;


    [[nodiscard]] uint64_t start() const override {}
    [[nodiscard]] uint64_t end() const override {}
};
