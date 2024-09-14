//
// Created by zoe on 14.09.24.
//


#pragma once
#include "ErrorHandler.h"


class ConsoleErrorHandler final : public ErrorHandler {
public:
    void handleError(const CompilerError &error, const SourceMap& sources) override;

    ~ConsoleErrorHandler() override;
};
