//
// Created by zoe on 19.09.24.
//


#pragma once
#include <string_view>
#include <vector>

class ModuleDeclaration;

class Module {
public:
    std::string_view name;
    std::vector<ModuleDeclaration> parts{};

    explicit Module(std::string_view name);

    ~Module();
    Module(Module&&) noexcept;
    Module& operator=(Module&&) noexcept;

    void addPart(ModuleDeclaration declaration);
};
