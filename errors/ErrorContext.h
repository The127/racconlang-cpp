//
// Created by zoe on 15.09.24.
//

#pragma once

#include "predeclare.h"

#include <vector>
#include <memory>

class racc::errors::ErrorContext {
public:
    std::vector<CompilerError> errors;

    ErrorContext();

    ErrorContext(const ErrorContext &) = delete;

    ErrorContext &operator=(const ErrorContext &) = delete;

    ErrorContext(ErrorContext &&) noexcept;

    ErrorContext &operator=(ErrorContext &&) noexcept;

    void addError(CompilerError error);

    void apply(const std::shared_ptr<sourcemap::Source> &source);
};
