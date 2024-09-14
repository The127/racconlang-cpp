//
// Created by zoe on 14.09.24.
//

#include "SimpleErrorLabel.h"

SimpleErrorLabel::SimpleErrorLabel(std::string text, const uint64_t start, const uint64_t end)
    : _text(std::move(text)),
      _start(start),
      _end(end) {
}

uint64_t SimpleErrorLabel::start() const {
    return _start;
}

uint64_t SimpleErrorLabel::end() const {
    return _end;
}

std::string_view SimpleErrorLabel::text() const {
    return _text;
}
