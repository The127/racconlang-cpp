//
// Created by zoe on 14.09.24.
//


#pragma once

#include "predeclare.h"

#include "CompilerError.h"

class racc::errors::ErrorHandler {
public:
    virtual ~ErrorHandler() = default;

    virtual void handleError(const CompilerError &error, const sourcemap::SourceMap &sources) = 0;
};
