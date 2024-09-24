//
// Created by zoe on 14.09.24.
//

#include "ImplSetter.h"
#include "Identifier.h"
#include "Parameter.h"

#include "utils/NodeUtils.h"

ImplSetter::ImplSetter() = default;
ImplSetter::ImplSetter(ImplSetter &&) noexcept = default;
ImplSetter & ImplSetter::operator=(ImplSetter &&) noexcept = default;
ImplSetter::~ImplSetter() = default;

uint64_t ImplSetter::start() const {
    return startPos;
}

uint64_t ImplSetter::end() const {
    return endPos;
}

std::string ImplSetter::toString(const SourceMap &sources, int indent, bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "ImplSetter", verbose) + "{\n";

    result += std::string(indent, ' ') + "isPublic: " + std::to_string(isPublic) + ",\n";

    if(name) {
        result += std::string(indent, ' ') + "name: " + name->toString(sources, indent+1, verbose) + ",\n";
    }
    if(parameter) {
        result += std::string(indent, ' ') + "parameter: " + parameter->toString(sources, indent+1, verbose) + ",\n";
    }

    result += std::string(indent - 1, ' ') + "}";
    return result;
}
