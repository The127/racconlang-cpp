#include "TupleType.h"

#include "TypeRefImpl.h"

TupleType::TupleType(std::vector<TypeRef> types) : types(std::move(types)) {

}

TupleType &TupleType::operator=(TupleType &&) noexcept = default;

TupleType::TupleType(TupleType &&) noexcept = default;

TupleType::~TupleType() = default;
