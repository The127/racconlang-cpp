#pragma once

#include "predeclare.h"

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
    std::string name;
    TypeRef type;
    ParameterMode mode;

    Parameter(std::string name, TypeRef type, ParameterMode mode);
    Parameter(const Parameter&) = delete;
    Parameter& operator=(const Parameter&) = delete;
    Parameter(Parameter&&) noexcept;
    Parameter& operator=(Parameter&&) noexcept;
    ~Parameter();
};
