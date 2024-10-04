#pragma once

#include "TypeRef.h"

#include "Struct.h"
#include "Interface.h"
#include "Enum.h"
#include "Alias.h"
#include "FunctionType.h"
#include "TupleType.h"
#include "TypeVar.h"
#include "BuiltinType.h"

#include <memory>
#include <variant>

class racc::registry::TypeRef {
private:
    std::shared_ptr<TypeVariant> _type;

    static TypeRef unknownRef;

    TypeRef() : _type(nullptr) {}

public:

    explicit TypeRef(const std::weak_ptr<TypeVariant> &ptr) : _type(ptr) {}

    template<typename T, typename...Args>
    [[nodiscard]] static TypeRef make(Args &&...args) {
        TypeRef t;
        t._type = std::make_shared<TypeVariant>(T(std::forward<Args>(args)...));
        std::get<T>(*t._type).type = t._type;
        return t;
    }

    template<typename...Args>
    [[nodiscard]] static TypeRef var(Args &&...args) {
        return make<TypeVar>(std::forward<Args>(args)...);
    }

    template<typename...Args>
    [[nodiscard]] static TypeRef builtin(Args &&...args) {
        return make<BuiltinType>(std::forward<Args>(args)...);
    }

    [[nodiscard]] static TypeRef unknown() {
        return unknownRef;
    }

    template<typename T>
    [[nodiscard]] std::optional<std::shared_ptr<T>> as() {
        if (_type && std::holds_alternative<T>(*_type)) {
            return std::shared_ptr<T>(_type, &std::get<T>(*_type));
        }
        return std::nullopt;
    }

    template<typename T>
    [[nodiscard]] std::optional<std::shared_ptr<const T>> as() const {
        if (_type && std::holds_alternative<T>(*_type)) {
            return std::shared_ptr<const T>(_type, &std::get<T>(*_type));
        }
        return std::nullopt;
    }

    template<typename T>
    [[nodiscard]] bool is() const {
        return _type && std::holds_alternative<T>(*_type);
    }

    void populate(ModuleRegistry &registry);

    [[nodiscard]] TypeRef concretize(ModuleRegistry &registry, const std::vector<TypeRef> &args) const;

    [[nodiscard]] TypeRef substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics) const;

    bool operator==(const TypeRef &other) const {
        return _type == other._type;
    }

    bool operator!=(const TypeRef &other) const {
        return _type != other._type;
    }

    bool operator<(const TypeRef &other) const {
        return _type < other._type;
    }

    [[nodiscard]] bool isUnknown() const {
        return _type == nullptr;
    }

    explicit operator bool() {
        return _type != nullptr;
    }

    [[nodiscard]] std::string_view declModulePath() const;

    [[nodiscard]] bool isPublic() const;

    [[nodiscard]] uint64_t declStart() const;
};