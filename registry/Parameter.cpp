#include "Parameter.h"

#include "TypeRef.h"

#include <utility>

namespace racc::registry {

    Parameter::Parameter(Id name, TypeRef type, ParameterMode mode) : name(std::move(name)), type(std::move(type)), mode(mode) {}

    Parameter::~Parameter() = default;

    Parameter &Parameter::operator=(Parameter &&) noexcept = default;

    Parameter::Parameter(Parameter &&) noexcept = default;

}