//
// Created by zoe on 07.09.24.
//


#pragma once
#include <string_view>

#include "Node.h"
#include "lexer/Token.h"


class Identifier final : public Node {
public:
    Identifier(const Token &identifier, const SourceMap &sources)
        : identifier(identifier) {
        name = sources.getText(identifier.start, identifier.end);
        if (name.starts_with('@'))
            name = name.substr(1, name.size());
    }

    [[nodiscard]] uint64_t start() const override;

    [[nodiscard]] uint64_t end() const override;

    Token identifier;
    std::string_view name;
};
