//
// Created by zoe on 10.09.24.
//


#pragma once
#include "Identifier.h"
#include "Node.h"
#include "SignatureBase.h"


class Parameter final : Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    Identifier name;
    std::unique_ptr<SignatureBase> type;

    explicit Parameter(Identifier name, std::unique_ptr<SignatureBase> type)
        : name(std::move(name)),
          type(std::move(type)) {
    }

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
