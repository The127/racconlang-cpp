//
// Created by zoe on 10.09.24.
//


#pragma once
#include "Identifier.h"
#include "Node.h"
#include "SignatureBase.h"

#include <optional>
#include <memory>


class Parameter final : Node {
public:
    uint64_t startPos{};
    uint64_t endPos{};
    bool isMut{};
    bool isRef{};
    Identifier name;
    std::optional<std::unique_ptr<SignatureBase>> type;

    explicit Parameter(Identifier name);
    Parameter(Parameter&&) noexcept;
    Parameter& operator=(Parameter&&) noexcept;
    ~Parameter() override;

    [[nodiscard]] uint64_t start() const override;

    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
