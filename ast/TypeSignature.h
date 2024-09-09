//
// Created by zoe on 07.09.24.
//

#pragma once

#include <string>
#include <vector>

#include "SignatureBase.h"


class TypeSignature final : SignatureBase {
public:
    std::string path;
    std::vector<std::string> genericParams;

    [[nodiscard]] uint64_t start() const override {};
    [[nodiscard]] uint64_t end() const override {}

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;;
};
