//
// Created by zoe on 09.09.24.
//


#pragma once

#include "ast/Node.h"

#include <vector>
#include <memory>
#include <string>

namespace racc::utils::node {
    static std::string nameString(const ast::Node &node, const std::string &name, const bool verbose) {
        std::string result = name;
        if (verbose) {
            result += "(" + std::to_string(node.start()) + ", " + std::to_string(node.end()) + ")";
        }
        return result;
    }

    template<class T>
    static std::string nodeListString(const sourcemap::SourceMap &sources, const std::vector<T> &nodes, const int indent, const bool verbose) {
        static_assert(std::is_base_of_v<ast::Node, T>, "T must derive from Node");

        std::string result = "[\n";

        for (const auto &node: nodes) {
            result += std::string(indent, ' ') + node.toString(sources, indent + 1, verbose) + "\n";
        }

        result += std::string(indent - 1, ' ') + "],";
        return result;
    }

    template<class T>
    static std::string nodeListString(const sourcemap::SourceMap &sources, const std::vector<std::unique_ptr<T>> &nodes, const int indent, const bool verbose) {
        static_assert(std::is_base_of_v<ast::Node, T>, "T must derive from Node");

        std::string result = "[\n";

        for (const auto &node: nodes) {
            result += std::string(indent, ' ') + node->toString(sources, indent + 1, verbose) + "\n";
        }

        result += std::string(indent - 1, ' ') + "],";
        return result;
    }
}