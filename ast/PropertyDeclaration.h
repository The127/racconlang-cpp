//
// Created by zoe on 07.09.24.
//


#pragma once
#include <memory>
#include <string>

#include "BaseSignature.h"
#include "Node.h"


class PropertyDeclaration : Node {
public:
    bool isPublic = false;
    std::string name;
    std::shared_ptr<BaseSignature> type;
};
