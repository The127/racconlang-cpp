//
// Created by zoe on 07.09.24.
//

#pragma once
#include <string>
#include <vector>
#include <memory>

#include "SignatureBase.h"


class EnumMemberDeclaration final : public Node {
public:
    std::string name;
    std::vector<std::unique_ptr<SignatureBase>> values;

    [[nodiscard]] uint64_t start() const override {}
    [[nodiscard]] uint64_t end() const override {}
};

