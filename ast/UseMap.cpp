//
// Created by gwendolyn on 9/26/24.
//

#include "UseMap.h"

namespace racc::ast {

    std::optional<std::string> UseMap::lookup(std::string_view name) const {
        auto it = uses.find(name);
        if (it == uses.end()) {
            return std::nullopt;
        }
        return it->second;
    }

}