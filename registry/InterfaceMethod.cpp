#include "InterfaceMethod.h"

#include "Parameter.h"



namespace racc::registry {

    InterfaceMethod::InterfaceMethod() {

    }

    InterfaceMethod &InterfaceMethod::operator=(InterfaceMethod &&) noexcept = default;

    InterfaceMethod::InterfaceMethod(InterfaceMethod &&) noexcept = default;

    InterfaceMethod::~InterfaceMethod() = default;

}
