//
// Created by zoe on 07.09.24.
//

#pragma once

#include <iostream>
#include <memory>
#include <string>

#include "Identifier.h"
#include "Signature.h"
#include "Node.h"


class PropertyDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic = false;
    bool isMutable = false;
    Identifier name;
    std::optional<Signature> type;

    explicit PropertyDeclaration(const Identifier& name);
    PropertyDeclaration(const PropertyDeclaration&) = delete;
    PropertyDeclaration& operator=(const PropertyDeclaration&) = delete;
    PropertyDeclaration(PropertyDeclaration&&) noexcept;
    PropertyDeclaration& operator=(PropertyDeclaration&&) noexcept;
    ~PropertyDeclaration() override;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
