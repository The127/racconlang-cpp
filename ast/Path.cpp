//
// Created by zoe on 07.09.24.
//

#include "Path.h"

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
    return parts.begin()->start();
}

uint64_t Path::end() const {
    if(isTrailing()) {
        return trailer->end;
    }
    return parts.end()->end();
}
