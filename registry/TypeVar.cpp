#include "TypeVar.h"

TypeVar::TypeVar(std::string name) : name(std::move(name)) {}

TypeVar &TypeVar::operator=(TypeVar &&) noexcept = default;

TypeVar::TypeVar(TypeVar &&) noexcept = default;

TypeVar::~TypeVar() = default;
