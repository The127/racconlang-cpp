//
// Created by zoe on 12.09.24.
//

#include "InterfaceSetter.h"

#include "utils/NodeUtils.h"

InterfaceSetter::InterfaceSetter() = default;
InterfaceSetter::InterfaceSetter(InterfaceSetter &&) noexcept = default;
InterfaceSetter & InterfaceSetter::operator=(InterfaceSetter &&) noexcept = default;
InterfaceSetter::~InterfaceSetter() = default;

uint64_t InterfaceSetter::start() const {
    return startPos;
}

uint64_t InterfaceSetter::end() const {
    return endPos;
}

std::string InterfaceSetter::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "InterfaceSetter", verbose) + "{\n";

    if(name) {
        result += std::string(indent, ' ') + "name: " + name->toString(sources, indent+1, verbose) + ",\n";
    }

    if(parameter) {
        result += std::string(indent, ' ') + "parameter: " + parameter->toString(sources, indent+1, verbose) + ",\n";
    }

    result += std::string(indent - 1, ' ') + "}";
    return std::move(result);
}
