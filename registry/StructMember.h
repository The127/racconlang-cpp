#pragma once

#include "predeclare.h"

#include "TypeRef.h"

#include <string>

class racc::registry::StructMember {
public:
    std::string name;
    ast::PropertyDeclaration *decl;
    TypeRef type;
    bool isPublic;
    bool isMutable;

    StructMember(std::string name,
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
