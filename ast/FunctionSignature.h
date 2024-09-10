//
// Created by zoe on 10.09.24.
//


#pragma once
#include "SignatureBase.h"


class ConstraintDeclaration;

class FunctionSignature final : public SignatureBase {
public:
    uint64_t startPos{};
    uint64_t endPos{};

    std::vector<std::unique_ptr<SignatureBase>> genericArguments;
    std::vector<ConstraintDeclaration> genericConstraints;
    std::vector<std::unique_ptr<SignatureBase>> parameterTypes;
    std::optional<std::unique_ptr<SignatureBase>> returnType;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
