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
    bool isMut{};
    bool isRef{};
    Identifier name;
    std::unique_ptr<SignatureBase> type;

    explicit Parameter(const bool isMut, const bool isRef, Identifier name, std::unique_ptr<SignatureBase> type)
        : isMut(isMut)
          , isRef(isRef)
          , name(std::move(name))
          , type(std::move(type)) {
    }

    [[nodiscard]] uint64_t start() const override;

    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
