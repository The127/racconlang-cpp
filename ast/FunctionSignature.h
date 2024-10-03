//
// Created by zoe on 10.09.24.
//


#pragma once

#include "predeclare.h"

namespace racc::ast {
    class FunctionSignature final : public Node {
    public:
        uint64_t startPos{};
        uint64_t endPos{};

        std::vector<NamelessParameter> parameterTypes;
        std::unique_ptr<ReturnType> returnType; // this is a pointer to avoid circular nesting, nullptr means the function has no return type

        FunctionSignature();

        FunctionSignature(const FunctionSignature &) = delete;

        FunctionSignature &operator=(const FunctionSignature &) = delete;

        FunctionSignature(FunctionSignature &&) noexcept;

        FunctionSignature &operator=(FunctionSignature &&) noexcept;

        ~FunctionSignature() override;

        [[nodiscard]] uint64_t start() const override;

        [[nodiscard]] uint64_t end() const override;

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
    };

}