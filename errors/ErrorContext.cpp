//
// Created by zoe on 15.09.24.
//

#include "ErrorContext.h"
#include "CompilerError.h"
#include "sourceMap/Source.h"

ErrorContext::ErrorContext() = default;

ErrorContext::ErrorContext(ErrorContext &&) noexcept = default;

ErrorContext & ErrorContext::operator=(ErrorContext &&) noexcept = default;

void ErrorContext::addError(CompilerError error) {
    errors.emplace_back(std::move(error));
}

void ErrorContext::apply(const std::shared_ptr<Source> &source) {
    for (auto & error : errors) {
        source->addError(std::move(error));
    }
    errors.clear();
}
