//
// Created by zoe on 07.09.24.
//


#pragma once

#include "predeclare.h"

#include "Node.h"
#include "lexer/Token.h"

#include <string_view>
#include <memory>

class racc::ast::Identifier final : public Node {
public:
    lexer::Token identifier;
    std::string_view name;

    Identifier(const Identifier &);

    Identifier &operator=(const Identifier &);

    Identifier(Identifier &&) noexcept;

    Identifier &operator=(Identifier &&) noexcept;

    ~Identifier() override;

    static Identifier make(const lexer::Token &identifier, const std::shared_ptr<sourcemap::Source> &source);

    [[nodiscard]] uint64_t start() const override;

    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;

    friend std::ostream &operator<<(std::ostream &out, const Identifier &identifier);

private:
    Identifier(const lexer::Token &identifier, const std::string_view &name);
};
