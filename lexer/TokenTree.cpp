//
// Created by zoe on 06.09.24.
//

#include "TokenTree.h"

#include "TokenTreeNode.h"

#include <iostream>


std::string TokenTree::toString(SourceMap &sources, uint32_t indent) const {
    std::string result;

    result += std::string(indent, ' ');
    result += left.toString(sources);
    result += '\n';

    for (const auto& token : tokens) {
        if (token.isTokenTree()) {
            result += token.getTokenTree().toString(sources, indent + 1);
        } else {
            result += std::string(indent+1, ' ');
            const auto& tokenRes = token.getTokenResult();
            if(tokenRes.isError()) {
                const auto& err = tokenRes.getError();
                result += err.token.toString(sources);
                result += ": " + err.reason;
            } else {
                result += tokenRes.get().toString(sources);
            }
            result += "\n";
        }
    }

    result += std::string(indent, ' ');
    if(right.isError()) {
        const auto& err = right.getError();
        result += err.token.toString(sources);
        result += ": " + err.reason;
    }else {
        result += right.get().toString(sources);
    }
    result += "\n";

    return result;
}
