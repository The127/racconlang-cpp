//
// Created by zoe on 12.09.24.
//

#include "utils/NodeUtils.h"

#include "ReturnType.h"

ReturnType::ReturnType(Signature type) : type(std::move(type)) {};

ReturnType::ReturnType(ReturnType &&) noexcept = default;

ReturnType & ReturnType::operator=(ReturnType &&) noexcept = default;

ReturnType::~ReturnType() = default;

uint64_t ReturnType::start() const {
    return startPos;
}

uint64_t ReturnType::end() const {
    return endPos;
}

std::string ReturnType::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "ReturnType", verbose) + "{\n";

    result += std::string(indent, ' ') + "isMut: " + std::to_string(isMut) + ",\n";

    result += std::string(indent, ' ') + "type: " + type.toString(sources, indent+1, verbose) + "\n";

    result += std::string(indent - 1, ' ') + "}";
    return result;
}
