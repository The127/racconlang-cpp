#pragma once

#include "predeclare.h"

#include "Id.h"
#include "TypeRef.h"

#include <string>
#include <vector>

class racc::registry::EnumMember {
public:
    Id name;
    ast::EnumMemberDeclaration *decl;
    std::vector<TypeRef> types;

    EnumMember(Id name, ast::EnumMemberDeclaration *decl, std::vector<TypeRef> types);

    EnumMember(const EnumMember &) = delete;

    EnumMember &operator=(const EnumMember &) = delete;

    EnumMember(EnumMember &&) noexcept;

    EnumMember &operator=(EnumMember &&) noexcept;

    ~EnumMember();
};
