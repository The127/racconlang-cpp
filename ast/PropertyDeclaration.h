//
// Created by zoe on 07.09.24.
//

#pragma once

#include <memory>
#include <string>

#include "SignatureBase.h"
#include "Node.h"


class PropertyDeclaration final : public Node {
public:
    bool isPublic = false;
    std::string name;
    std::unique_ptr<SignatureBase> type;

    [[nodiscard]] uint64_t start() const override {}
    [[nodiscard]] uint64_t end() const override {}
};
