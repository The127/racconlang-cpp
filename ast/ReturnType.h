//
// Created by zoe on 12.09.24.
//


#pragma once
#include "Node.h"
#include "SignatureBase.h"


class ReturnType final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isMut{};
    std::unique_ptr<SignatureBase> type;

    ReturnType();
    ReturnType(ReturnType&&) noexcept;
    ReturnType& operator=(ReturnType&&) noexcept;
    ~ReturnType() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
