#pragma once


#include <memory>
#include <variant>

#include "TypeRef.h"

#include "Struct.h"
#include "Interface.h"
#include "Enum.h"
#include "Alias.h"
#include "FunctionType.h"
#include "TupleType.h"
#include "TypeVar.h"
#include "BuiltinType.h"


class TypeRef {
private:
    std::shared_ptr<TypeVariant> type;

    static TypeRef unknownRef;


    TypeRef() : type(nullptr) {}
public:
    using Weak = std::weak_ptr<TypeVariant>;


    explicit TypeRef(const std::weak_ptr<TypeVariant> &ptr) : type(ptr) {}

    template<typename T, typename...Args>
    [[nodiscard]] static TypeRef make(Args&&...args) {
        TypeRef t;
        t.type = std::make_shared<TypeVariant>(T(std::forward<Args>(args)...));
        std::get<T>(*t.type).type = t.type;
        return t;
    }

    template<typename...Args>
    [[nodiscard]] static TypeRef var(Args&&...args) {
        return make<TypeVar>(std::forward<Args>(args)...);
    }

    template<typename...Args>
    [[nodiscard]] static TypeRef builtin(Args&&...args) {
        return make<BuiltinType>(std::forward<Args>(args)...);
    }

    [[nodiscard]] static TypeRef unknown() {
        return unknownRef;
    }

    template<typename T>
    [[nodiscard]] std::optional<std::shared_ptr<T>> as() {
        if (type && std::holds_alternative<T>(*type)) {
            return std::shared_ptr<T>(type, &std::get<T>(*type));
        }
        return std::nullopt;
    }

    template<typename T>
    [[nodiscard]] std::optional<std::shared_ptr<const T>> as() const {
        if (type && std::holds_alternative<T>(*type)) {
            return std::shared_ptr<const T>(type, &std::get<T>(*type));
        }
        return std::nullopt;
    }

    template<typename T>
    [[nodiscard]] bool is() const {
        return type && std::holds_alternative<T>(*type);
    }

    void populate(ModuleRegistry &registry);

    [[nodiscard]] TypeRef concretize(ModuleRegistry &registry, const std::vector<TypeRef> &args) const;

    [[nodiscard]] TypeRef substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef>& generics) const;

    bool operator==(const TypeRef &other) const {
        return type == other.type;
    }

    bool operator!=(const TypeRef &other) const {
        return type != other.type;
    }

    bool operator<(const TypeRef &other) const {
        return type < other.type;
    }

    [[nodiscard]] bool isUnknown() const {
        return type == nullptr;
    }

    explicit operator bool() {
        return type != nullptr;
    }

    [[nodiscard]] std::string_view declModulePath() const;

    [[nodiscard]] bool isPublic() const;

    [[nodiscard]] uint64_t declStart() const;
};

using WeakTypeRef = TypeRef::Weak;