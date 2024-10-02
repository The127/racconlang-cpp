#include "StructMember.h"


StructMember::StructMember(std::string name, PropertyDeclaration *decl, TypeRef type, bool isPublic, bool isMutable)
        : name(std::move(name)),
          decl(decl),
          type(std::move(type)),
          isPublic(isPublic),
          isMutable(isMutable) {}

StructMember &StructMember::operator=(StructMember &&) noexcept = default;

StructMember::StructMember(StructMember &&) noexcept = default;

StructMember::~StructMember() = default;

