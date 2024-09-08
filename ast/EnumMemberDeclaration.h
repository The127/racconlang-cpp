//
// Created by zoe on 07.09.24.
//

#pragma once
#include <string>
#include <vector>

#include "BaseSignature.h"


class EnumMemberDeclaration {
public:
    std::string name;
    std::vector<BaseSignature> values;
};

