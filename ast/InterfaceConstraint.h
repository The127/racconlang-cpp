//
// Created by zoe on 07.09.24.
//

#pragma once

#include "GenericConstraintBase.h"
#include "TypeSignature.h"


class InterfaceConstraint final : GenericConstraintBase {
public:
    TypeSignature typeSignature;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
