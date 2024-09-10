//
// Created by zoe on 07.09.24.
//

#pragma once

#include <memory>
#include <string>

#include "Identifier.h"
#include "SignatureBase.h"
#include "Node.h"


class PropertyDeclaration final : public Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isPublic = false;
    Identifier name;
    std::unique_ptr<SignatureBase> type;

    PropertyDeclaration(Identifier name)
        : name(std::move(name)) {
    }

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
