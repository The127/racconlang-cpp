#include "EnumMember.h"

#include "TypeRef.h"

#include <utility>

namespace racc::registry {

    EnumMember::EnumMember(std::string name, ast::EnumMemberDeclaration *decl, std::vector<TypeRef> types)
            : name(std::move(name)),
              decl(decl),
              types(std::move(types)) {

    }

    EnumMember &EnumMember::operator=(EnumMember &&) noexcept = default;

    EnumMember::EnumMember(EnumMember &&) noexcept = default;

    EnumMember::~EnumMember() = default;

}
