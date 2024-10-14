#pragma once

#include "predeclare.h"

#include <memory>
#include <variant>
#include <vector>
#include <map>


class racc::registry::TypeRef {
    template<typename T>
    friend
    class TypeBase;
private:
    std::shared_ptr<TypeVariant> _type;

    static TypeRef unknownRef;
    static TypeRef emptyRef;

    TypeRef();

    explicit TypeRef(TypeVariant &&t);

public:

    explicit TypeRef(const std::weak_ptr<TypeVariant> &ptr);

    TypeRef(const TypeRef &);
    TypeRef &operator=(const TypeRef &);
    TypeRef(TypeRef &&) noexcept;
    TypeRef &operator=(TypeRef &&) noexcept;
    ~TypeRef();

    [[nodiscard]] static TypeRef var(Id name);

    [[nodiscard]] static TypeRef makeBuiltin(Id name, size_t size);

    [[nodiscard]] static TypeRef unknown();

    [[nodiscard]] static TypeRef empty();

    void populate(ModuleRegistry &registry);

    [[nodiscard]] TypeRef concretize(ModuleRegistry &registry, const std::vector<TypeRef> &args) const;

    [[nodiscard]] TypeRef substituteGenerics(ModuleRegistry &registry, const std::map<TypeRef, TypeRef> &generics) const;

    bool operator==(const TypeRef &other) const;

    bool operator!=(const TypeRef &other) const;

    bool operator<(const TypeRef &other) const;

    [[nodiscard]] bool isUnknown() const;

    [[nodiscard]] bool isInterface() const;

    [[nodiscard]] const Interface& asInterface() const;
    [[nodiscard]] Interface& asInterface();

    explicit operator bool() const;

    [[nodiscard]] Id declModulePath() const;

    [[nodiscard]] bool isPublic() const;

    [[nodiscard]] uint64_t declStart() const;
    static std::pair<TypeRef, std::shared_ptr<Alias>> makeAlias(Id name, Id module, int arity, ast::AliasDeclaration *decl, std::shared_ptr<sourcemap::Source> source,
                             std::shared_ptr<ast::UseMap> useMap);
    static std::pair<TypeRef, std::shared_ptr<Enum>> makeEnum(Id name, Id module, int arity, ast::EnumDeclaration *decl, std::shared_ptr<sourcemap::Source> source,
                            std::shared_ptr<ast::UseMap> useMap);
    static std::pair<TypeRef, std::shared_ptr<FunctionType>> makeFunction(std::vector<std::pair<ParameterMode, TypeRef>> params, TypeRef returnType, bool returnMut);
    static std::pair<TypeRef, std::shared_ptr<TupleType>> makeTuple(const std::vector<TypeRef> &types);
    static std::pair<TypeRef, std::shared_ptr<Struct>> makeStruct(Id name, Id module, unsigned long arity, ast::StructDeclaration *decl,
                              const std::shared_ptr<sourcemap::Source> &source, const std::shared_ptr<ast::UseMap> &useMap);
    static std::pair<TypeRef, std::shared_ptr<Interface>> makeInterface(Id name, Id module, unsigned long arity, ast::InterfaceDeclaration *decl,
                                 const std::shared_ptr<sourcemap::Source> &source, const std::shared_ptr<ast::UseMap> &useMap);
    [[nodiscard]] bool isVar() const;
    [[nodiscard]] bool isBuiltin() const;
};