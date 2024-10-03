#include "InterfaceMethod.h"

InterfaceMethod::InterfaceMethod() {

}

InterfaceMethod &InterfaceMethod::operator=(InterfaceMethod &&) noexcept = default;

InterfaceMethod::InterfaceMethod(InterfaceMethod &&) noexcept = default;

InterfaceMethod::~InterfaceMethod() = default;
