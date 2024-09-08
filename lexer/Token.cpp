//
// Created by zoe on 06.09.24.
//

#include "Token.h"
#include "sourceMap/SourceMap.h"

#include <stdexcept>

bool Token::isComment() const {
    return type == TokenType::LineComment
           || type == TokenType::MultiLineComment;
}

bool Token::isAllowedInAngleBrackets() const {
    return isComment()
           || type == TokenType::Identifier
           || type == TokenType::Fn
           || type == TokenType::PathSeparator
           || type == TokenType::DashArrow
           || type == TokenType::Comma;
}

bool Token::isDeclaratorKeyword() const {
    return type == TokenType::Let
           || type == TokenType::Interface
           || type == TokenType::Struct
           || type == TokenType::Enum
           || type == TokenType::Alias
           || type == TokenType::Fn;
}

bool Token::isOpening() const {
    return type == TokenType::Bof
           || type == TokenType::OpenCurly
           || type == TokenType::OpenParen
           || type == TokenType::OpenAngle;
}

bool Token::isClosing() const {
    return type == TokenType::Eof
           || type == TokenType::CloseCurly
           || type == TokenType::CloseParen
           || type == TokenType::CloseAngle;
}

bool Token::isTopLevelStarter() const {
    return isDeclaratorKeyword()
           || type == TokenType::Pub
           || type == TokenType::Mod
           || type == TokenType::Use;
}

bool Token::isModifier() const {
    return type == TokenType::Pub;
}

TokenType Token::expectedClosing() const {
    switch (type) {
        case TokenType::OpenCurly:
            return TokenType::CloseCurly;
        case TokenType::Bof:
            return TokenType::Eof;
        case TokenType::OpenParen:
            return TokenType::CloseParen;
        case TokenType::OpenAngle:
            return TokenType::CloseAngle;
        default:
            throw std::runtime_error("Unexpected token type");
    }
}

std::string Token::toString(const SourceMap &sources) const {
    std::string result;
    result += TokenTypeName(type);
    result += " <";
    result += sources.getText(start, end);
    result += "> at ";
    result += sources.getLocation(start).toString();
    return result;
}