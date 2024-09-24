//
// Created by zoe on 10.09.24.
//

#pragma once

#include "Node.h"
#include "SignatureBase.h"


class TupleSignature final : public SignatureBase {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    std::vector<std::unique_ptr<SignatureBase>> types;

    TupleSignature();
    TupleSignature(const TupleSignature&) = delete;
    TupleSignature& operator=(const TupleSignature&) = delete;
    TupleSignature(TupleSignature&&) noexcept;
    TupleSignature& operator=(TupleSignature&&) noexcept;
    ~TupleSignature() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
