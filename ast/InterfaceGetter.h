//
// Created by zoe on 12.09.24.
//
#pragma once

#include "Identifier.h"
#include "Node.h"
#include "ReturnType.h"


class InterfaceGetter final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isMut{};
    std::optional<Identifier> name;
    std::optional<ReturnType> returnType;

    InterfaceGetter();
    InterfaceGetter(const InterfaceGetter&) = delete;
    InterfaceGetter& operator=(const InterfaceGetter&) = delete;
    InterfaceGetter(InterfaceGetter&&) noexcept;
    InterfaceGetter& operator=(InterfaceGetter&&) noexcept;
    ~InterfaceGetter() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;

};
