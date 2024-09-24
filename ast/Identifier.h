//
// Created by zoe on 07.09.24.
//


#pragma once
#include <string_view>

#include "Node.h"
#include "lexer/Token.h"


class Source;

class Identifier final : public Node {
public:
    Token identifier;
    std::string_view name;

    Identifier(const Identifier&);
    Identifier& operator=(const Identifier&);
    Identifier(Identifier&&) noexcept;
    Identifier& operator=(Identifier&&) noexcept;
    ~Identifier() override;

    static Identifier make(const Token &identifier, const std::shared_ptr<Source> &source);

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    [[nodiscard]] std::string toString(const SourceMap &sources, int indent, bool verbose) const override;

    friend std::ostream & operator<< (std::ostream &out, const Identifier &identifier);

private:
    Identifier(const Token &identifier, const std::string_view& name);
};
