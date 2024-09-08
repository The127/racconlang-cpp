//
// Created by zoe on 07.09.24.
//

#include "TokenResult.h"
#include "sourceMap/SourceMap.h"

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

const Token & TokenResult::getOrErrorToken() const {
    if(isToken())
        return get();
    return getError().token;
}

Location TokenResult::getPosition(const SourceMap &sources) const {
    return sources.getLocation(getStart());
}

std::string TokenResult::toString(const SourceMap &sources) const {
    if(isError()) {
        const auto& err = getError();
        return err.token.toString(sources) + ": " + err.reason;
    }
    return get().toString(sources);
}
