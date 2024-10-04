//
// Created by zoe on 12.09.24.
//
#pragma once

#include "Identifier.h"
#include "Node.h"
#include "ReturnType.h"

class racc::ast::InterfaceGetterDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isMut{};
    std::optional<Identifier> name;
    std::optional<ReturnType> returnType;

    InterfaceGetterDeclaration();

    InterfaceGetterDeclaration(const InterfaceGetterDeclaration &) = delete;

    InterfaceGetterDeclaration &operator=(const InterfaceGetterDeclaration &) = delete;

    InterfaceGetterDeclaration(InterfaceGetterDeclaration &&) noexcept;

    InterfaceGetterDeclaration &operator=(InterfaceGetterDeclaration &&) noexcept;

    ~InterfaceGetterDeclaration() override;

    [[nodiscard]] uint64_t start() const override;

    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;

};
