//
// Created by zoe on 07.09.24.
//


#pragma once
#include <optional>
#include <vector>

#include "Identifier.h"
#include "Node.h"

namespace racc::ast {

    class Path final : public Node {
    public:
        std::optional<lexer::Token> rooted;
        std::vector<Identifier> parts;
        std::optional<lexer::Token> trailer;

        Path();

        Path(const Path &) = delete;

        Path &operator=(const Path &) = delete;

        Path(Path &&) noexcept;

        Path &operator=(Path &&) noexcept;

        ~Path() override;

        [[nodiscard]] bool isRooted() const;

        [[nodiscard]] bool isTrailing() const;

        [[nodiscard]] uint64_t start() const override;

        [[nodiscard]] uint64_t end() const override;

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;

        [[nodiscard]] Path clone() const;
    };

}