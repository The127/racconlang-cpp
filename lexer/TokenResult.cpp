//
// Created by zoe on 07.09.24.
//

#include "TokenResult.h"

uint64_t TokenResult::getStart() const {
    if (isError())
        return getError().token.start;
    return get().start;
}

uint64_t TokenResult::getEnd() const {
    if (isError())
        return getError().token.end;
    return get().end;
}

bool TokenResult::isError() const {
    return !value.has_value();
}

bool TokenResult::isToken() const {
    return value.has_value();
}

bool TokenResult::isToken(TokenType type) const {
    return isToken() && value->type == type;
}

Token &TokenResult::get() {
    return *value;
}

const Token & TokenResult::get() const {
    return *value;
}

const LexerErr &TokenResult::getError() const {
    return value.error();
}

Location TokenResult::getPosition(const SourceMap &sources) const {
    return sources.getLocation(getStart());
}
