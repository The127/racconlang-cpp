//
// Created by zoe on 07.09.24.
//

#include "UseNode.h"

#include "utils/StringUtils.h"
#include "utils/NodeUtils.h"

namespace racc::ast {

    UseNode::UseNode() = default;

    UseNode::UseNode(UseNode &&) noexcept = default;

    UseNode &UseNode::operator=(UseNode &&) noexcept = default;

    UseNode::~UseNode() = default;

    uint64_t UseNode::start() const {
        return startPos;
    }

    uint64_t UseNode::end() const {
        return endPos;
    }

    std::string UseNode::toString(const sourcemap::SourceMap &sources, const int indent, const bool verbose) const {
        std::string result = utils::node::nameString(*this, "UseNode", verbose) + "{\n";

        result += std::string(indent, ' ') + "path: " + path.toString(sources, indent, verbose) + ",\n";


        // TODO
//    if(!names.empty())
//        result += std::string(indent, ' ') + "names: [" + utils::string::join(names, ", ") + "],\n";

        result += std::string(indent - 1, ' ') + "}";
        return result;
    }

}
