//
// Created by zoe on 07.09.24.
//

#pragma once
#include <string>
#include <vector>
#include "ConstraintDeclaration.h"
#include "Node.h"
#include "PropertyDeclaration.h"

class StructDeclaration : Node {
public:
    bool isPublic = false;
    std::string name;
    std::vector<std::string> genericParams;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<PropertyDeclaration> structDeclarations;
    std::vector<std::string> destructureProperties;
};
