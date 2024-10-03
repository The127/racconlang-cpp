//
// Created by zoe on 09.09.24.
//


#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace racc::utils::string {

    template<typename T1, typename T2>
    static std::string join(const T1 &values, const T2 &delimiter) {
        std::stringstream result;

        bool first = true;
        for (const auto &value: values) {
            if (first) {
                first = false;
            } else {
                result << delimiter;
            }
            result << value;
        }

        return result.str();
    }

}