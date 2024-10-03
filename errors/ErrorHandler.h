//
// Created by zoe on 14.09.24.
//


#pragma once

#include "predeclare.h"

#include "CompilerError.h"

namespace racc::errors {
    class ErrorHandler {
    public:
        virtual ~ErrorHandler() = default;

        virtual void handleError(const CompilerError &error, const sourcemap::SourceMap &sources) = 0;
    };
}