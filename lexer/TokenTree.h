//
// Created by zoe on 06.09.24.
//

#pragma once

#include "predeclare.h"

#include "Token.h"
#include "TokenResult.h"

#include <vector>

namespace racc::lexer {
    class TokenTree {
    public:
        Token left;
        std::vector<TokenTreeNode> tokens;
        TokenResult right;

        explicit TokenTree(const Token &left);

        TokenTree(const TokenTree &) = delete;

        TokenTree &operator=(const TokenTree &) = delete;

        TokenTree(TokenTree &&) noexcept;

        TokenTree &operator=(TokenTree &&) noexcept;

        ~TokenTree();

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, uint32_t indent = 0) const;
    };

}