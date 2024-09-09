//
// Created by zoe on 09.09.24.
//


#pragma once
#include <sstream>
#include <string>
#include <vector>

namespace StringUtils {

    template<typename T>
    static std::string join(const std::vector<T> values, const std::string &delimiter) {
        std::stringstream result;

        for (const auto &value : values) {
            result << value;
            if(&value != &values.back()) {
                result << delimiter;
            }
        }

        return result.str();
    }
}