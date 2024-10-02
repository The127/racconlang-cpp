#include "NamelessParameter.h"
#include "Signature.h"

#include "utils/NodeUtils.h"

NamelessParameter::NamelessParameter(Signature type) : type(std::move(type)) {}

NamelessParameter::NamelessParameter(NamelessParameter &&) noexcept = default;
NamelessParameter &NamelessParameter::operator=(NamelessParameter &&) noexcept = default;

NamelessParameter::~NamelessParameter() = default;

uint64_t NamelessParameter::start() const {
    return startPos;
}

uint64_t NamelessParameter::end() const {
    return endPos;
}

std::string NamelessParameter::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "NamelessParameter", verbose) + "{\n";

    result += std::string(indent, ' ') + "isMut: " + std::to_string(isMut) + ",\n";
    result += std::string(indent, ' ') + "isRef: " + std::to_string(isRef) + ",\n";
    result += std::string(indent, ' ') + "type: ";
    result += std::string(indent, ' ') + "type: " + type.toString(sources, indent + 1, verbose) + ",\n";

    result += std::string(indent - 1, ' ') + "}";
    return result;
}
