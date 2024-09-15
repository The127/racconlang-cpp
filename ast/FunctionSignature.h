//
// Created by zoe on 10.09.24.
//


#pragma once
#include "Parameter.h"
#include "ReturnType.h"
#include "SignatureBase.h"


class ConstraintDeclaration;

class FunctionSignature final : public SignatureBase {
public:
    uint64_t startPos{};
    uint64_t endPos{};

    std::vector<std::unique_ptr<SignatureBase>> parameterTypes;
    std::optional<ReturnType> returnType;

    FunctionSignature();
    FunctionSignature(FunctionSignature&&) noexcept;
    FunctionSignature& operator=(FunctionSignature&&) noexcept;
    ~FunctionSignature() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
