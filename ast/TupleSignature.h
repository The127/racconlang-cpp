//
// Created by zoe on 10.09.24.
//

#pragma once

#include "predeclare.h"

#include "Node.h"

namespace racc::ast {
    class TupleSignature final : public Node {
    public:
        uint64_t startPos{};
        uint64_t endPos{};
        std::vector<Signature> types;

        TupleSignature();

        TupleSignature(const TupleSignature &) = delete;

        TupleSignature &operator=(const TupleSignature &) = delete;

        TupleSignature(TupleSignature &&) noexcept;

        TupleSignature &operator=(TupleSignature &&) noexcept;

        ~TupleSignature() override;

        [[nodiscard]] uint64_t start() const override;

        [[nodiscard]] uint64_t end() const override;

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
    };

}