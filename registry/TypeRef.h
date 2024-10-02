#pragma once


#include <memory>
#include <variant>

class Struct;
class Interface;
class Enum;
class Alias;
class FunctionType;
class TupleType;
class TypeVar;
class BuiltinType;

class TypeRef;
using TypeVariant = std::variant<Struct, Interface, Enum, Alias, FunctionType, TupleType, TypeVar, BuiltinType>;
using WeakTypeRef = std::weak_ptr<TypeVariant>;
