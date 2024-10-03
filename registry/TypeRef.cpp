#include "TypeRefImpl.h"

#include "ast/StructDeclaration.h"
#include "ast/EnumDeclaration.h"

namespace racc::registry {

    TypeRef TypeRef::unknownRef = {};

    void TypeRef::populate(ModuleRegistry &registry) {
        if (auto s = as<Struct>()) {
            (*s)->populate(registry);
        } else if (auto i = as<Interface>()) {
            (*i)->populate(registry);
        } else if (auto e = as<Enum>()) {
            (*e)->populate(registry);
        } else if (auto a = as<Alias>()) {
            (*a)->populate(registry);
        } else {
            COMPILER_ASSERT(false, "cannot populate this type");
        }
    }

    TypeRef TypeRef::concretize(ModuleRegistry &registry, const std::vector<TypeRef> &args) const {
        COMPILER_ASSERT(!args.empty(), "no generic arguments given");
        if (auto s = as<Struct>()) {
            return (*s)->concretize(registry, args);
        } else if (auto i = as<Interface>()) {
            return (*i)->concretize(registry, args);
        } else if (auto e = as<Enum>()) {
            return (*e)->concretize(registry, args);
        } else if (auto a = as<Alias>()) {
            return (*a)->concretize(registry, args);
        }

        COMPILER_ASSERT(false, "cannot concretize this type");
    }

    TypeRef TypeRef::substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics) const {
        if (auto s = as<Struct>()) {
            return (*s)->substituteGenerics(registry, generics);
        } else if (auto i = as<Interface>()) {
//        return (*i)->substituteGenerics(registry, generics);
        } else if (auto e = as<Enum>()) {
            return (*e)->substituteGenerics(registry, generics);
        } else if (auto a = as<Alias>()) {
//        return (*a)->substituteGenerics(registry, generics);
        } else if (auto f = as<FunctionType>()) {
            return (*f)->substituteGenerics(registry, generics);
        } else if (auto t = as<TupleType>()) {
//        return (*f)->substituteGenerics(registry, generics);
        } else if (auto v = as<TypeVar>()) {
            auto it = generics.find(*this);
            if (it != generics.end()) {
                return it->second;
            } else {
                return *this;
            }
        } else if (auto b = as<BuiltinType>()) {
            return *this;
        } else if (isUnknown()) {
            return unknown();
        }

        COMPILER_ASSERT(false, "cannot substitute generics on this type");
    }

    std::string_view TypeRef::declModulePath() const {
        if (auto s = as<Struct>()) {
            return (*s)->modulePath;
        } else if (auto i = as<Interface>()) {
            return (*i)->modulePath;
        } else if (auto e = as<Enum>()) {
            return (*e)->modulePath;
        } else if (auto a = as<Alias>()) {
            return (*a)->modulePath;
        }

        COMPILER_ASSERT(false, "this type does not have a module");
    }

    bool TypeRef::isPublic() const {
        if (auto s = as<Struct>()) {
            return (*s)->isPublic;
        } else if (auto i = as<Interface>()) {
//        return (*i)->isPublic;
        } else if (auto e = as<Enum>()) {
            return (*e)->isPublic;
        } else if (auto a = as<Alias>()) {
//        return (*a)->isPublic;
        } else if (auto f = as<FunctionType>()) {
//        return (*f)->isPublic();
        } else if (auto t = as<TupleType>()) {
//        return (*t)->isPublic();
        } else if (is<TypeVar>() || is<BuiltinType>() || isUnknown()) {
            return true;
        }
        COMPILER_UNREACHABLE();
    }

    uint64_t TypeRef::declStart() const {
        if (auto s = as<Struct>()) {
            return (*s)->declaration->start();
        } else if (auto i = as<Interface>()) {
//        return (*i)->isPublic;
        } else if (auto e = as<Enum>()) {
            return (*e)->declaration->start();
        } else if (auto a = as<Alias>()) {
//        return (*a)->isPublic;
        } else if (auto v = as<TypeVar>()) {
//        return (*v)->declaration->start;
        }
        COMPILER_ASSERT(false, "type does not have a declaration");
    }

}
