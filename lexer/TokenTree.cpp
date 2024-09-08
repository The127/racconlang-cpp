//
// Created by zoe on 06.09.24.
//

#include "TokenTree.h"

#include "TokenTreeNode.h"

#include <iostream>


std::string TokenTree::toString(const SourceMap &sources, uint32_t indent) const {
    std::string result;

    result += std::string(indent, ' ');
    result += left.toString(sources);
    result += '\n';

    for (const auto& node : tokens) {
        result += node.toString(sources, indent + 1);
    }

    result += std::string(indent, ' ');
    result += right.toString(sources);
    result += "\n";

    return result;
}
