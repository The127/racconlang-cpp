//
// Created by zoe on 07.09.24.
//

#pragma once

#include <string>
#include <vector>

#include "EnumMemberDeclaration.h"
#include "Node.h"


class GenericConstraint;

class EnumDeclaration : Node {
public:
    bool isPublic = false;
    std::string name;
    std::vector<std::string> genericParams;
    std::vector<GenericConstraint> genericConstraints;
    std::vector<EnumMemberDeclaration> memberDeclarations;
};
