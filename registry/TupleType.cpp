#include "TupleType.h"

#include "TypeRef.h"

namespace racc::registry {

    TupleType::TupleType(std::vector<TypeRef> types) : types(std::move(types)) {

    }

    TupleType &TupleType::operator=(TupleType &&) noexcept = default;

    TupleType::TupleType(TupleType &&) noexcept = default;

    TupleType::~TupleType() = default;

}