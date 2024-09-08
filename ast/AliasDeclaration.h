//
// Created by zoe on 07.09.24.
//


#pragma once


#include "Node.h"

class AliasDeclaration final : public Node {
public:
    [[nodiscard]] uint64_t start() const override {}
    [[nodiscard]] uint64_t end() const override {}
};
