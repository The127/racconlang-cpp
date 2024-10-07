#include "StructMember.h"

#include <utility>

namespace racc::registry {

    StructMember::StructMember(std::string name, ast::PropertyDeclaration *decl, TypeRef type, bool isPublic, bool isMutable)
            : name(std::move(name)),
              decl(decl),
              type(std::move(type)),
              isPublic(isPublic),
              isMutable(isMutable) {
    }

    StructMember &StructMember::operator=(StructMember &&) noexcept = default;

    StructMember::StructMember(StructMember &&) noexcept = default;

    StructMember::~StructMember() = default;

}