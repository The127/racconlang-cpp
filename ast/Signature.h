//
// Created by zoe on 07.09.24.
//

#pragma once

#include "Node.h"
#include "TypeSignature.h"
#include "TupleSignature.h"
#include "FunctionSignature.h"

namespace racc::ast {

    class Signature : public Node {
    public:
        std::variant<TypeSignature, TupleSignature, FunctionSignature> signature;

        explicit(false) Signature(TypeSignature sig);

        explicit(false) Signature(TupleSignature sig);

        explicit(false) Signature(FunctionSignature sig);

        Signature(const Signature &) = delete;

        Signature &operator=(const Signature &) = delete;

        Signature(Signature &&) noexcept;

        Signature &operator=(Signature &&) noexcept;

        ~Signature() override;


        [[nodiscard]] bool isType() const;

        [[nodiscard]] bool isTuple() const;

        [[nodiscard]] bool isFunction() const;

        [[nodiscard]] TypeSignature &asType();

        [[nodiscard]] TupleSignature &asTuple();

        [[nodiscard]] FunctionSignature &asFunction();

        [[nodiscard]] const TypeSignature &asType() const;

        [[nodiscard]] const TupleSignature &asTuple() const;

        [[nodiscard]] const FunctionSignature &asFunction() const;


        [[nodiscard]] uint64_t start() const override;

        [[nodiscard]] uint64_t end() const override;

        [[nodiscard]] std::string toString(const sourcemap::SourceMap &sources, int indent, bool verbose) const override;
    };

}