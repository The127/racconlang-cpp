//
// Created by zoe on 14.09.24.
//


#pragma once
#include "CompilerError.h"


class ErrorHandler {
public:
    virtual ~ErrorHandler() = default;

    virtual void handleError(const CompilerError &error, const SourceMap& sources) = 0;
};
