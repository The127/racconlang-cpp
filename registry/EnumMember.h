#pragma once

#include "TypeRef.h"

#include <string>
#include <vector>

class EnumMemberDeclaration;


class EnumMember {
public:
    std::string name;
    EnumMemberDeclaration *decl;
    std::vector<TypeRef> types;

    EnumMember(std::string name, EnumMemberDeclaration *decl, std::vector<TypeRef> types);

    EnumMember(const EnumMember&) = delete;
    EnumMember& operator=(const EnumMember&) = delete;
    EnumMember(EnumMember&&) noexcept;
    EnumMember& operator=(EnumMember&&) noexcept;
    ~EnumMember();
};
