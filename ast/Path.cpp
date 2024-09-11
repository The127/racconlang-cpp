//
// Created by zoe on 07.09.24.
//

#include "Path.h"

#include "utils/StringUtils.h"

bool Path::isRooted() const {
    return rooted.has_value();
}

bool Path::isTrailing() const {
    return trailer.has_value();
}

uint64_t Path::start() const {
    if (isRooted()) {
        return rooted->start;
    }
    COMPILER_ASSERT(!parts.empty(), "path parts is empty");
    return parts.front().start();
}

uint64_t Path::end() const {
    if(isTrailing()) {
        return trailer->end;
    }
    COMPILER_ASSERT(!parts.empty(), "path parts is empty");
    return parts.back().end();
}

std::string Path::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    return StringUtils::join(parts, "::");
}
