//
// Created by zoe on 07.09.24.
//

#pragma once

#include "predeclare.h"

#include "Path.h"

#include <string>
#include <vector>

namespace racc::ast {
    class TypeSignature final : public Node {
    public:
        uint64_t startPos{};
        uint64_t endPos{};

        uint64_t genericStartPos{};
        uint64_t genericEndPos{};

        Path path;
        std::vector<Signature> genericArguments;

        TypeSignature();

        TypeSignature(const TypeSignature &) = delete;

        TypeSignature &operator=(const TypeSignature &) = delete;

        TypeSignature(TypeSignature &&) noexcept;

        TypeSignature &operator=(TypeSignature &&) noexcept;

        ~TypeSignature() override;

        [[nodiscard]] uint64_t start() const override;

        [[nodiscard]] uint64_t end() const override;

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
    };

}