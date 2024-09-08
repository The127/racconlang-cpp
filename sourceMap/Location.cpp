//
// Created by zoe on 06.09.24.
//

#include "Location.h"

std::string Location::toString() const {
    return std::string(fileName) + ":" + std::to_string(line) + ":" + std::to_string(column);
}
