#include "TypeRef.h"

#include "ast/StructDeclaration.h"
#include "ast/EnumDeclaration.h"
#include "ast/InterfaceDeclaration.h"
#include "ast/AliasDeclaration.h"
#include "TypeVar.h"
#include "Struct.h"
#include "Interface.h"
#include "Alias.h"
#include "BuiltinType.h"
#include "Enum.h"
#include "TupleType.h"
#include "FunctionType.h"
#include "errors/CompilerError.h"
#include "utils/Overload.h"


namespace racc::registry {
    struct TypeVariant {
        using Variant = std::variant<Struct, Interface, Enum, Alias, FunctionType, TupleType, TypeVar, BuiltinType, std::monostate>;

        Variant val;

        TypeVariant(Variant&& v) : val(std::move(v)) {}
        TypeVariant& operator=(Variant&& v) {
            this->val = std::move(v);
            return *this;
        }

        TypeVariant(TypeVariant&&) noexcept = default;
        TypeVariant& operator=(TypeVariant&&) noexcept = default;

    };


    TypeRef TypeRef::unknownRef = {};
    TypeRef TypeRef::emptyRef = TypeRef(TypeVariant(std::monostate()));

    TypeRef::TypeRef(TypeVariant &&t) : _type(std::make_shared<TypeVariant>(std::move(t))) {}

    TypeRef::TypeRef(const std::weak_ptr<TypeVariant> &ptr) : _type(ptr) {}

    TypeRef &TypeRef::operator=(TypeRef &&) noexcept = default;

    TypeRef::TypeRef(TypeRef &&) noexcept = default;

    TypeRef &TypeRef::operator=(const TypeRef &) = default;

    TypeRef::TypeRef(const TypeRef &) = default;

    TypeRef::~TypeRef() = default;

    void TypeRef::populate(ModuleRegistry &registry) {
        std::visit(util::overload{
                [&](Struct &x) { x.populate(registry); },
                [&](Interface &x) { x.populate(registry); },
                [&](Enum &x) { x.populate(registry); },
                [&](Alias &x) { x.populate(registry); },
                [](auto &) { COMPILER_ASSERT(false, "cannot populate this type"); }
        }, _type->val);
    }

    TypeRef TypeRef::concretize(ModuleRegistry &registry, const std::vector<TypeRef> &args) const {
        COMPILER_ASSERT(!args.empty(), "no generic arguments given");
        if (!_type) {
            return unknown();
        }
        return std::visit(util::overload{
                [&](Struct &x) { return x.concretize(registry, args); },
                [&](Interface &x) { return x.concretize(registry, args); },
                [&](Enum &x) { return x.concretize(registry, args); },
                [&](Alias &x) { return x.concretize(registry, args); },
                [](auto &) -> TypeRef { COMPILER_ASSERT(false, "cannot concretize this type"); }
        }, _type->val);
    }

    TypeRef TypeRef::substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics) const {
        if (!_type) {
            return unknown();
        }
        return std::visit(util::overload{
                [&](Struct &x) { return x.substituteGenerics(registry, generics).first; },
//                [&](Interface &x) { return x.substituteGenerics(registry, generics).first; },
                [&](Enum &x) { return x.substituteGenerics(registry, generics).first; },
                [&](Alias &x) { return x.substituteGenerics(registry, generics).first; },
                [&](FunctionType &x) { return x.substituteGenerics(registry, generics).first; },
//                [&](TupleType &x) { return x.substituteGenerics(registry, generics).first; },
                [&](TypeVar &x) {
                    auto it = generics.find(*this);
                    if (it != generics.end()) {
                        return it->second;
                    } else {
                        return *this;
                    }
                },
                [&](BuiltinType &x) { return *this; },
                [&](auto &) -> TypeRef {
                    COMPILER_ASSERT(false, "cannot substitute generics on this type");
                }
        }, _type->val);
    }

    bool TypeRef::isPublic() const {
        if (!_type) {
            return true; // unknown type is always public
        }
        return std::visit(util::overload{
                [&](Struct &x) { return x.isPublic; },
                [&](Interface &x) { return x.isPublic; },
                [&](Enum &x) { return x.isPublic; },
                [&](Alias &x) { return x.isPublic; },
//                [&](TupleType &x) { return x.isPublic(); },
//                [&](FunctionType &x) { return x.isPublic(); },
                [](auto &) -> bool { return true; }
        }, _type->val);
    }

    std::string_view TypeRef::declModulePath() const {
        if (!_type) {
            COMPILER_ASSERT(false, "type does not have a module");
        }
        return std::visit(util::overload{
                [&](Struct &x) { return x.modulePath; },
                [&](Interface &x) { return x.modulePath; },
                [&](Enum &x) { return x.modulePath; },
                [&](Alias &x) { return x.modulePath; },
                [](auto &) -> std::string_view { COMPILER_ASSERT(false, "this type does not have a module"); }
        }, _type->val);
    }

    uint64_t TypeRef::declStart() const {
        if (!_type) {
            COMPILER_ASSERT(false, "type does not have a declaration");
        }
        return std::visit(util::overload{
                [&](Struct &x) { return x.declaration->start(); },
                [&](Interface &x) { return x.declaration->start(); },
                [&](Enum &x) { return x.declaration->start(); },
                [&](Alias &x) { return x.declaration->start(); },
                [](auto &) -> uint64_t { COMPILER_ASSERT(false, "type does not have a declaration"); }
        }, _type->val);
    }

    TypeRef TypeRef::unknown() {
        return unknownRef;
    }

    TypeRef TypeRef::empty() {
        return emptyRef;
    }

    TypeRef TypeRef::var(std::string name) {
        TypeRef t { std::make_shared<TypeVariant>(std::monostate()) };
        auto& v = t._type->val.emplace<TypeVar>(name);
        v._typeref = t._type;
        return t;
    }

    TypeRef TypeRef::makeBuiltin(std::string name, size_t size) {
        TypeRef t { std::make_shared<TypeVariant>(std::monostate()) };
        auto& v = t._type->val.emplace<BuiltinType>(name, size);
        v._typeref = t._type;
        return t;
    }

    std::pair<TypeRef, std::shared_ptr<Alias>> TypeRef::makeAlias(std::string name, std::string_view module, int arity, ast::AliasDeclaration *decl, std::shared_ptr<sourcemap::Source> source,
                               std::shared_ptr<ast::UseMap> useMap) {
        TypeRef t { std::make_shared<TypeVariant>(std::monostate()) };
        auto& v = t._type->val.emplace<Alias>(name, module, arity, decl, source, useMap);
        v._typeref = t._type;
        return {t, std::shared_ptr<Alias>(t._type, &v)};
    }

    std::pair<TypeRef, std::shared_ptr<Enum>> TypeRef::makeEnum(std::string name, std::string_view module, int arity, ast::EnumDeclaration *decl, std::shared_ptr<sourcemap::Source> source,
                              std::shared_ptr<ast::UseMap> useMap) {
        TypeRef t { std::make_shared<TypeVariant>(std::monostate()) };
        auto& v = t._type->val.emplace<Enum>(name, module, arity, decl, source, useMap);
        v._typeref = t._type;
        return {t, std::shared_ptr<Enum>(t._type, &v)};
    }

    std::pair<TypeRef, std::shared_ptr<Struct>> TypeRef::makeStruct(std::basic_string<char> name, std::string_view module, unsigned long arity, ast::StructDeclaration *decl,
                                const std::shared_ptr<sourcemap::Source> &source, const std::shared_ptr<ast::UseMap> &useMap) {
        TypeRef t { std::make_shared<TypeVariant>(std::monostate()) };
        auto& v = t._type->val.emplace<Struct>(name, module, arity, decl, source, useMap);
        v._typeref = t._type;
        return {t, std::shared_ptr<Struct>(t._type, &v)};
    }

    std::pair<TypeRef, std::shared_ptr<Interface>> TypeRef::makeInterface(std::basic_string<char> name, std::string_view module, unsigned long arity, ast::InterfaceDeclaration *decl,
                                   const std::shared_ptr<sourcemap::Source> &source, const std::shared_ptr<ast::UseMap> &useMap) {
        TypeRef t { std::make_shared<TypeVariant>(std::monostate()) };
        auto& v = t._type->val.emplace<Interface>(name, module, arity, decl, source, useMap);
        v._typeref = t._type;
        return {t, std::shared_ptr<Interface>(t._type, &v)};
    }

    std::pair<TypeRef, std::shared_ptr<FunctionType>> TypeRef::makeFunction(std::vector<std::pair<ParameterMode, TypeRef>> params, TypeRef returnType, bool returnMut) {
        TypeRef t { std::make_shared<TypeVariant>(std::monostate()) };
        auto& v = t._type->val.emplace<FunctionType>(params, returnType, returnMut);
        v._typeref = t._type;
        return {t, std::shared_ptr<FunctionType>(t._type, &v)};
    }

    std::pair<TypeRef, std::shared_ptr<TupleType>> TypeRef::makeTuple(const std::vector<TypeRef> &types) {
        TypeRef t { std::make_shared<TypeVariant>(std::monostate()) };
        auto& v = t._type->val.emplace<TupleType>(types);
        v._typeref = t._type;
        return {t, std::shared_ptr<TupleType>(t._type, &v)};
    }

    bool TypeRef::operator==(const TypeRef &other) const {
        return _type == other._type;
    }

    bool TypeRef::operator!=(const TypeRef &other) const {
        return _type != other._type;
    }

    bool TypeRef::operator<(const TypeRef &other) const {
        return _type < other._type;
    }

    bool TypeRef::isUnknown() const {
        return _type == nullptr;
    }

    TypeRef::operator bool() const {
        return _type != nullptr;
    }

    bool TypeRef::isBuiltin() const {
        return _type && std::holds_alternative<BuiltinType>(_type->val);
    }

    bool TypeRef::isVar() const {
        return _type && std::holds_alternative<TypeVar>(_type->val);
    }

    TypeRef::TypeRef() : _type(nullptr) {}

}
