//
// Created by zoe on 07.09.24.
//


#pragma once
#include <iostream>
#include <stacktrace>


#define COMPILER_ASSERT(cond, msg) do { if (!(cond)) { raccoonlang_compiler_assert(msg, __FILE__, __LINE__); }} while(0)

[[noreturn]] static void raccoonlang_compiler_assert(const std::string& msg, const std::string& filename, int line) {
    std::cerr << "compiler error in " << filename << ":" << line << ": " << msg << std::endl << std::flush;
#ifndef NDEBUG
    std::cerr << std::stacktrace::current() << std::endl << std::flush;
#endif
    std::exit(1);
}

#ifndef NDEBUG
#define DEBUG_ASSERT(cond, msg) do { if (!(cond)) { raccoonlang_compiler_assert(msg, __FILE__, __LINE__); }} while(0)

[[noreturn]] static void raccoonlang_debug_assert(const std::string& msg, const std::string& filename, int line) {
    std::cerr << "debug error in " << filename << ":" << line << ": " << msg << std::endl << std::flush;
    std::exit(1);
}
#else
#define DEBUG_ASSERT(cond, msg)
#endif
