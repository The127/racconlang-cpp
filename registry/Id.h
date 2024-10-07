#pragma once

#include "predeclare.h"

#include "ast/Identifier.h"
#include "errors/InternalError.h"

#include <format>
#include <set>
#include <string>

class racc::registry::Id {
    friend class std::formatter<Id>;
private:
    static std::set<std::string> ids;

    const std::string *name{};

public:
    Id() = default;

    explicit(false) Id(const std::string_view &v) {
        auto [it, inserted] = ids.emplace(v);
        name = &*it;
    }

    explicit(false) Id(const std::string &v) {
        auto [it, inserted] = ids.emplace(v);
        name = &*it;
    }

    explicit(false) Id(const char* v) {
        auto [it, inserted] = ids.emplace(v);
        name = &*it;
    }

    explicit(false) Id(const ast::Identifier &v) : Id(v.name) {}


    Id &operator=(const Id &other) = default;
    Id(const Id &) = default;


    bool operator==(const Id &other) const {
        return name == other.name;
    }

    template <typename T>
    bool operator==(const T &other) const {
        return name == other;
    }

    template <typename T>
    bool operator==(const T *other) const {
        return name == *other;
    }

    bool operator!=(const Id &other) const {
        return name != other.name;
    }

    template <typename T>
    bool operator!=(const T &other) const {
        return name != other;
    }

    template <typename T>
    bool operator!=(const T *other) const {
        return name != *other;
    }

    bool operator<(const Id &other) const {
        return name < other.name;
    }

    template<typename T>
    bool operator<(const T *other) const {
        return name < *other;
    }

    template<typename T>
    bool operator<(const T &other) const {
        return name < other;
    }

    Id operator+(const Id &other) const {
        return Id(*name + *other.name);
    }

    template<typename T>
    Id operator+(const T &other) const {
        return Id(*name + other);
    }

    explicit operator bool() const {
        return name != nullptr;
    }

    std::string_view operator*() const {
        COMPILER_ASSERT(name != nullptr, "empty id");
        return std::string_view(*name);
    }

    const std::string *operator->() const {
        COMPILER_ASSERT(name != nullptr, "empty id");
        return name;
    }

    [[nodiscard]] Id substr(size_t start, size_t length = std::string::npos) const {
        return Id(name->substr(start, length));
    }
};

template<class CharT>
struct std::formatter<racc::registry::Id, CharT> {
    template <typename FormatParseContext>
    constexpr auto parse(FormatParseContext &ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const racc::registry::Id &id, FormatContext &ctx) const {
        return std::format_to(ctx.out(), "{}", *id.name);
    }
};
