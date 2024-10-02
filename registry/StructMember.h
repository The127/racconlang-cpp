#pragma once

#include "TypeRefImpl.h"

#include <string>

class PropertyDeclaration;


class StructMember {
public:
    std::string name;
    PropertyDeclaration *decl;
    TypeRef type;
    bool isPublic;
    bool isMutable;

    StructMember(std::string name,
                 PropertyDeclaration *decl,
                 TypeRef type,
                 bool isPublic,
                 bool isMutable);

    ~StructMember();
    StructMember(const StructMember&) = delete;
    StructMember& operator=(const StructMember&) = delete;
    StructMember(StructMember&&) noexcept;
    StructMember& operator=(StructMember&&) noexcept;

};