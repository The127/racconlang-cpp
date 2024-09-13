//
// Created by zoe on 12.09.24.
//

#pragma once

#include "Identifier.h"
#include "Node.h"
#include "Parameter.h"


class InterfaceSetter final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    std::optional<Identifier> name;
    std::optional<Parameter> parameter;

    InterfaceSetter();
    InterfaceSetter(InterfaceSetter&&) noexcept;
    InterfaceSetter& operator=(InterfaceSetter&&) noexcept;
    ~InterfaceSetter() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
