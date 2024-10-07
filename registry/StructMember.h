#pragma once

#include "predeclare.h"

#include "Id.h"
#include "TypeRef.h"

#include <string>

class racc::registry::StructMember {
public:
    Id name;
    ast::PropertyDeclaration *decl;
    TypeRef type;
    bool isPublic;
    bool isMutable;

    StructMember(Id name,
                 ast::PropertyDeclaration *decl,
                 TypeRef type,
                 bool isPublic,
                 bool isMutable);

    ~StructMember();

    StructMember(const StructMember &) = delete;

    StructMember &operator=(const StructMember &) = delete;

    StructMember(StructMember &&) noexcept;

    StructMember &operator=(StructMember &&) noexcept;

};
