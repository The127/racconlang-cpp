//
// Created by zoe on 07.09.24.
//

#include "UseNode.h"

#include "utils/StringUtils.h"
#include "utils/NodeUtils.h"

uint64_t UseNode::start() const {
    return startPos;
}

uint64_t UseNode::end() const {
    return endPos;
}

std::string UseNode::toString(const SourceMap &sources, const int indent, const bool verbose) const {
    std::string result = NodeUtils::nameString(*this, "UseNode", verbose) + "{\n";

    result += std::string(indent, ' ') + "path: " + path.toString(sources, indent, verbose) + ",\n";

    if(!names.empty())
        result += std::string(indent, ' ') + "names: [" + StringUtils::join(names, ", ") + "],\n";

    result += std::string(indent - 1, ' ') + "}";
    return std::move(result);
}
