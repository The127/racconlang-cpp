//
// Created by zoe on 12.09.24.
//


#pragma once
#include "Node.h"
#include "Signature.h"


class ReturnType final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isMut{};
    Signature type;

    explicit ReturnType(Signature type);

    ReturnType(const ReturnType&) = delete;
    ReturnType& operator=(const ReturnType&) = delete;
    ReturnType(ReturnType&&) noexcept;
    ReturnType& operator=(ReturnType&&) noexcept;
    ~ReturnType() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
