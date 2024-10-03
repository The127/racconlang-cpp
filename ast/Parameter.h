//
// Created by zoe on 10.09.24.
//


#pragma once
#include "Identifier.h"
#include "Node.h"
#include "Signature.h"

#include <optional>
#include <memory>

namespace racc::ast {

    class Parameter final : Node {
    public:
        uint64_t startPos{};
        uint64_t endPos{};
        bool isMut{};
        bool isRef{};
        Identifier name;
        std::optional<Signature> type;

        explicit Parameter(Identifier name);

        Parameter(const Parameter &) = delete;

        Parameter &operator=(const Parameter &) = delete;

        Parameter(Parameter &&) noexcept;

        Parameter &operator=(Parameter &&) noexcept;

        ~Parameter() override;

        [[nodiscard]] uint64_t start() const override;

        [[nodiscard]] uint64_t end() const override;

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
    };

}