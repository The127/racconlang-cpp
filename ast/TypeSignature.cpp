//
// Created by zoe on 07.09.24.
//

#include "TypeSignature.h"

#include "utils/NodeUtils.h"

uint64_t TypeSignature::start() const {
    return startPos;
}

uint64_t TypeSignature::end() const {
    return endPos;
}

std::string TypeSignature::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "TypeSignature", verbose) + "{\n";

    result += std::string(indent, ' ') + "path: " + path.toString(sources, indent+1, verbose) + ",\n";

    if(!genericArguments.empty()) {
        result += std::string(indent, ' ') + "genericArguments: " + NodeUtils::nodeListString(sources, genericArguments, indent + 1, verbose) + "\n";
    }

    return std::move(result);
}
