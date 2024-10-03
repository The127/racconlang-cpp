#pragma once

#include <string>

#include "TypeRefImpl.h"

class Interface;


class InterfaceMethod {
public:
    std::string name;
    bool mutReturn;
    TypeRef returnType;


    Interface* interface;

    InterfaceMethod();
    ~InterfaceMethod();
    InterfaceMethod(const InterfaceMethod&) = delete;
    InterfaceMethod& operator=(const InterfaceMethod&) = delete;
    InterfaceMethod(InterfaceMethod&&) noexcept;
    InterfaceMethod& operator=(InterfaceMethod&&) noexcept;
};
