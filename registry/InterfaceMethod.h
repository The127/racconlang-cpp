#pragma once

#include "predeclare.h"

#include "TypeRefImpl.h"

#include <string>
#include <memory>

class racc::registry::InterfaceMethod {
public:
    std::string name;
    bool mutReturn;
    std::vector<Parameter> params;
    std::unique_ptr<TypeRef> returnType;
    Interface *interface;

    InterfaceMethod();

    ~InterfaceMethod();

    InterfaceMethod(const InterfaceMethod &) = delete;

    InterfaceMethod &operator=(const InterfaceMethod &) = delete;

    InterfaceMethod(InterfaceMethod &&) noexcept;

    InterfaceMethod &operator=(InterfaceMethod &&) noexcept;
};
