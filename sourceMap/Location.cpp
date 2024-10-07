//
// Created by zoe on 06.09.24.
//

#include "Location.h"

namespace racc::sourcemap {

    Location::Location(const std::string_view fileName, const uint32_t line, const uint32_t column) : fileName(fileName),
                                                                                                      line(line),
                                                                                                      column(column) {
    }

}
