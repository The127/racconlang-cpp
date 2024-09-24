//
// Created by zoe on 15.09.24.
//

#pragma once

#include <vector>

#include "parser/Parser.h"

class Parser;
class CompilerError;

class ErrorContext {
public:
    std::vector<CompilerError> errors;
    ErrorContext();
    ErrorContext(const ErrorContext&) = delete;
    ErrorContext& operator=(const ErrorContext&) = delete;
    ErrorContext(ErrorContext&&) noexcept;
    ErrorContext& operator=(ErrorContext&&) noexcept;
    void addError(CompilerError error);
    void apply(const std::shared_ptr<Source> &source);
};
