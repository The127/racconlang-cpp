#pragma once

#include "predeclare.h"
#include "TypeRef.h"
#include "errors/CompilerError.h"

#include <type_traits>
#include <memory>

namespace racc::registry {
template<typename T>
class TypeBase {
    friend class TypeRef;
protected:
    std::weak_ptr<TypeVariant> _typeref;

public:
    [[nodiscard]] TypeRef type() const {
        return TypeRef(_typeref);
    }

    [[nodiscard]] std::shared_ptr<const T> ptr() const  {
        auto p = _typeref.lock();
        return std::shared_ptr<const T>(p, static_cast<T*>(this));
    };

    [[nodiscard]] std::shared_ptr<T> ptr()  {
        auto p = _typeref.lock();
        return std::shared_ptr<T>(p, static_cast<T*>(this));
    };
};
}
