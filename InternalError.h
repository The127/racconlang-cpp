//
// Created by zoe on 07.09.24.
//


#pragma once
#include <format>
#include <stdexcept>

#define COMPILER_ASSERT(cond, msg) do { if (!(cond)) { throw InternalError(msg, __FILE__, __LINE__); }} while(0)

class InternalError final : std::runtime_error {
public:
    InternalError(const std::string &msg, const std::string &file, int line)
        : std::runtime_error(std::format("compiler error in {}:{}: {}", file, line, msg)) {
    }

    explicit InternalError(const std::string &_arg)
        : std::runtime_error(_arg) {
    }

    explicit InternalError(const char *string)
        : std::runtime_error(string) {
    }

    explicit InternalError(std::runtime_error &&runtime_error)
        : std::runtime_error(runtime_error) {
    }

    explicit InternalError(const std::runtime_error &runtime_error)
        : std::runtime_error(runtime_error) {
    }
};
