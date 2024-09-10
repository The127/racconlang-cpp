//
// Created by zoe on 07.09.24.
//

#pragma once

#include <string>
#include <vector>

#include "Path.h"
#include "SignatureBase.h"


class TypeSignature final : public SignatureBase {
public:
    uint64_t startPos{};
    uint64_t endPos{};

    Path path;
    std::vector<std::unique_ptr<SignatureBase>> genericArguments;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;;
};
