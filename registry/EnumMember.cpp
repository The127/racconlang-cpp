#include "EnumMember.h"

#include "TypeRefImpl.h"

#include <utility>

EnumMember::EnumMember(std::string name, EnumMemberDeclaration *decl, std::vector<TypeRef> types)
        : name(std::move(name)),
          decl(decl),
          types(std::move(types)) {

}

EnumMember &EnumMember::operator=(EnumMember &&) noexcept = default;

EnumMember::EnumMember(EnumMember &&) noexcept = default;

EnumMember::~EnumMember() = default;
