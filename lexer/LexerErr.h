//
// Created by zoe on 07.09.24.
//

#pragma once

#include <utility>

#include "Token.h"


class LexerErr {
public:
    Token token;
    std::string reason;

    LexerErr(const Token &token, std::string reason)
        : token(token)
          , reason(std::move(reason)) {
    }
};
