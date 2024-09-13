//
// Created by zoe on 07.09.24.
//


#pragma once
#include <optional>
#include <vector>

#include "Identifier.h"
#include "Node.h"


class Path final : public Node {
public:
    std::optional<Token> rooted;
    std::vector<Identifier> parts;
    std::optional<Token> trailer;

    Path();
    Path(Path&&) noexcept;
    Path& operator=(Path&&) noexcept;
    ~Path() override;

    [[nodiscard]] bool isRooted() const;
    [[nodiscard]] bool isTrailing() const;

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;
};
