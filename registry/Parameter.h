#pragma once

#include "predeclare.h"

#include "Id.h"
#include "TypeRef.h"

#include <cstdint>
#include <string>


enum class racc::registry::ParameterMode {
    Normal,
    Mut,
    Ref,
};


class racc::registry::Parameter {
public:
    Id name;
    TypeRef type;
    ParameterMode mode;

    Parameter(Id name, TypeRef type, ParameterMode mode);
    Parameter(const Parameter&) = delete;
    Parameter& operator=(const Parameter&) = delete;
    Parameter(Parameter&&) noexcept;
    Parameter& operator=(Parameter&&) noexcept;
    ~Parameter();
};
