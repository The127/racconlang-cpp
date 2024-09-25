//
// Created by zoe on 07.09.24.
//

#include "Signature.h"
#include "errors/InternalError.h"


Signature::Signature(TypeSignature sig) : signature(std::move(sig)) {

}

Signature::Signature(TupleSignature sig) : signature(std::move(sig)) {

}

Signature::Signature(FunctionSignature sig) : signature(std::move(sig)) {

}


Signature::~Signature() = default;

Signature::Signature(Signature &&) noexcept = default;

Signature &Signature::operator=(Signature &&) noexcept = default;

bool Signature::isType() const {
    return std::holds_alternative<TypeSignature>(signature);
}

bool Signature::isTuple() const {
    return std::holds_alternative<TupleSignature>(signature);
}

bool Signature::isFunction() const {
    return std::holds_alternative<FunctionSignature>(signature);
}

const TypeSignature &Signature::asType() const {
    COMPILER_ASSERT(std::holds_alternative<TypeSignature>(signature), "trying to access type signature in a non-type signature");
    return std::get<TypeSignature>(signature);
}

const TupleSignature &Signature::asTuple() const {
    COMPILER_ASSERT(std::holds_alternative<TupleSignature>(signature), "trying to access tuple signature in a non-tuple signature");
    return std::get<TupleSignature>(signature);
}

const FunctionSignature &Signature::asFunction() const {
    COMPILER_ASSERT(std::holds_alternative<FunctionSignature>(signature), "trying to access function signature in a non-function signature");
    return std::get<FunctionSignature>(signature);
}

TypeSignature &Signature::asType() {
    COMPILER_ASSERT(std::holds_alternative<TypeSignature>(signature), "trying to access type signature in a non-type signature");
    return std::get<TypeSignature>(signature);
}

TupleSignature &Signature::asTuple() {
    COMPILER_ASSERT(std::holds_alternative<TupleSignature>(signature), "trying to access tuple signature in a non-tuple signature");
    return std::get<TupleSignature>(signature);
}

FunctionSignature &Signature::asFunction() {
    COMPILER_ASSERT(std::holds_alternative<FunctionSignature>(signature), "trying to access function signature in a non-function signature");
    return std::get<FunctionSignature>(signature);
}

uint64_t Signature::start() const {
    if (isType()) {
        return asType().start();
    }
    if (isTuple()) {
        return asTuple().start();
    }
    if (isFunction()) {
        return asFunction().start();
    }
    COMPILER_ASSERT(false, "unreachable");
}

uint64_t Signature::end() const {
    if (isType()) {
        return asType().end();
    }
    if (isTuple()) {
        return asTuple().end();
    }
    if (isFunction()) {
        return asFunction().end();
    }
    COMPILER_ASSERT(false, "unreachable");
}

std::string Signature::toString(const SourceMap &sources, int indent, bool verbose) const {

    if (isType()) {
        return asType().toString(sources, indent, verbose);
    }
    if (isTuple()) {
        return asTuple().toString(sources, indent, verbose);
    }
    if (isFunction()) {
        return asFunction().toString(sources, indent, verbose);
    }
    COMPILER_ASSERT(false, "unreachable");
}
