//
// Created by zoe on 07.09.24.
//

#pragma once

#include <string>
#include <vector>

#include "BaseSignature.h"


class TypeSignature final : BaseSignature {
public:
    std::string path;
    std::vector<std::pmr::string> genericParams;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;
};
