#pragma once

#include "predeclare.h"

#include "TypeRefImpl.h"

#include <string>

namespace racc::registry {
    class StructMember {
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

}
