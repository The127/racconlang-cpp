//
// Created by zoe on 06.09.24.
//

#pragma once

#include <cstdint>
#include <string>

#include "TokenType.h"
#include "sourceMap/SourceMap.h"



class Token {
public:
    TokenType type;
    uint64_t start;
    uint64_t end;

    Token(TokenType type, u_int64_t start, u_int64_t end)
        : type(type),
          start(start),
          end(end) {
    }

    [[nodiscard]] bool isComment() const;
    [[nodiscard]] bool isAllowedInAngleBrackets() const;
    [[nodiscard]] bool isDeclaratorKeyword() const;
    [[nodiscard]] bool isOpening() const;
    [[nodiscard]] bool isClosing() const;
    [[nodiscard]] bool isTopLevelStarter() const;
    [[nodiscard]] TokenType expectedClosing() const;

    std::string toString(const SourceMap& sources) const;
};
