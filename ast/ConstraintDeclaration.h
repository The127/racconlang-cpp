//
// Created by zoe on 07.09.24.
//

#pragma once

#include <string>
#include <vector>

#include "Node.h"
#include "GenericConstraint.h"


class ConstraintDeclaration : Node {
public:
    std::string name;
    std::vector<GenericConstraint> constraints;
};
