//
// Created by zoe on 22.09.24.
//

#include "Struct.h"

#include "sourceMap/Source.h"
#include "ast/FileUses.h"
#include "ast/StructDeclaration.h"
#include "ast/PropertyDeclaration.h"
#include "ModuleRegistry.h"


#include <utility>

class Type;

class StructMember {
public:
    std::string name;
    PropertyDeclaration *decl;
    Type *type;
    bool isPublic;
    bool isMutable;

    StructMember(std::string name, PropertyDeclaration *decl, Type *type, bool isPublic, bool isMutable) : name(name), decl(decl), type(type),
                                                                                                           isPublic(isPublic), isMutable(isMutable) {

    }
};

Struct::Struct(std::string name, const uint8_t arity, StructDeclaration *declaration, std::shared_ptr<Source> source,
               std::shared_ptr<FileUses> fileUses)
        : name(std::move(name)),
          arity(arity),
          declaration(declaration),
          source(std::move(source)),
          fileUses(std::move(fileUses)) {
}

void Struct::populate() {
    for (auto &decl: declaration->propertyDeclarations) {
//        auto name = std::string(decl.name.name);
//        ModuleRegistry r;
//        auto x = dynamic_cast<TypeSignature>(decl.ty)
//        auto type = r.lookupType(fileUses, );
//
//        members.emplace_back(name, &decl, type, decl.isPublic, decl.isMutable);
    }
}

Struct::~Struct() = default;

Struct::Struct(Struct &&) noexcept = default;

Struct &Struct::operator=(Struct &&) noexcept = default;
