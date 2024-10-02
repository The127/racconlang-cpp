//
// Created by zoe on 09.09.24.
//


#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace StringUtils {

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


//    template<typename T>
//    static std::string join(const std::vector<T>& values, const std::string &delimiter) {
//        std::stringstream result;
//
//        for (const auto &value : values) {
//            result << value;
//            if(&value != &values.back()) {
//                result << delimiter;
//            }
//        }
//
//        return result.str();
//    }
}