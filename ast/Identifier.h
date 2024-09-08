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
    Identifier(const Token &identifier, const Source &source);

    [[nodiscard]] uint64_t start() const override;
    [[nodiscard]] uint64_t end() const override;

    Token identifier;
    std::string_view name;
};
