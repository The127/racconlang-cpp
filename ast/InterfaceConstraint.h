//
// Created by zoe on 07.09.24.
//

#pragma once

#include "GenericConstraintBase.h"
#include "TypeSignature.h"


class InterfaceConstraint final : public GenericConstraintBase {
public:
    TypeSignature typeSignature;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    InterfaceConstraint();
    InterfaceConstraint(const InterfaceConstraint&) = delete;
    InterfaceConstraint& operator=(const InterfaceConstraint&) = delete;
    InterfaceConstraint(InterfaceConstraint&& other) noexcept;
    InterfaceConstraint& operator=(InterfaceConstraint&& other) noexcept;
    ~InterfaceConstraint() override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
