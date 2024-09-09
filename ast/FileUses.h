//
// Created by zoe on 09.09.24.
//


#pragma once
#include "UseNode.h"


class FileUses {
public:
    bool isVerified{};
    std::vector<UseNode> uses;
};
