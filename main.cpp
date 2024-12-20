#include <ast/ImplBlock.h>
#include "ast/AliasDeclaration.h"
#include "ast/EnumDeclaration.h"
#include "ast/InterfaceDeclaration.h"
#include "ast/StructDeclaration.h"
#include "ast/ModuleDeclaration.h"
#include "errors/ConsoleErrorHandler.h"
#include "errors/ErrorHandler.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "registry/ModuleRegistry.h"
#include "registry/Module.h"
#include "sourceMap/SourceMap.h"

#include <iostream>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>


int main() {
    auto llvmCtx = std::make_unique<llvm::LLVMContext>();
    auto llvmBuilder = std::make_unique<llvm::IRBuilder<>>(*llvmCtx);
    auto llvmModule = std::make_unique<llvm::Module>("main", *llvmCtx);
    auto *treeType = llvm::StructType::create(*llvmCtx, "Tree");
    treeType->setBody({llvm::Type::getInt32Ty(*llvmCtx), treeType->getPointerTo(), treeType->getPointerTo()});

    auto directory = "demo";


    racc::sourcemap::SourceMap sources{};

    const auto source = sources.addEntry("demo/test.rc");

    racc::lexer::Lexer lexer(source);
    lexer.tokenize();


    std::cout << source->tokenTree->toString(sources, 0);

    racc::parser::Parser parser(source);
    auto modules = parser.parse();

    /*for (auto &module: modules) {

        std::cout << std::string(5, '\n');
        std::cout << std::string(10, '#');
        std::cout << std::string(2, '\n');

        std::cout << module.toString(sources, 1, true);
        std::cout << std::endl << std::endl;
    }*/

    racc::registry::ModuleRegistry moduleRegistry{};
    for (auto &moduleDecl: modules) {
        auto modulePath = moduleDecl.buildPathString();
        auto &module = moduleRegistry.addModule(modulePath);

        //TODO: what to do with declarations that have the same generic parameter multiple times?

        for (auto &structDeclaration: moduleDecl.structDeclarations) {
            if (!structDeclaration.name)
                continue;

            module.addStruct(
                source,
                structDeclaration,
                moduleDecl.uses);
        }

        for (auto &enumDeclaration: moduleDecl.enumDeclarations) {
            if (!enumDeclaration.name)
                continue;

            module.addEnum(
                source,
                enumDeclaration,
                moduleDecl.uses);
        }

        for (auto &interfaceDeclaration: moduleDecl.interfaceDeclarations) {
            if (!interfaceDeclaration.name)
                continue;

            module.addInterface(
                source,
                interfaceDeclaration,
                moduleDecl.uses);
        }

        for (auto &aliasDeclaration: moduleDecl.aliasDeclarations) {
            if (!aliasDeclaration.name)
                continue;

            module.addAlias(
                source,
                aliasDeclaration,
                moduleDecl.uses);
        }
    }

    moduleRegistry.populate();

    for(auto &moduleDecl: modules) {
        auto modulePath = moduleDecl.buildPathString();
        auto &module = moduleRegistry.getModule(modulePath);

        for (auto &implBlock: moduleDecl.implBlocks) {
            if(!implBlock.structName)
                continue;

            /*module.addImplBlock(
                source,
                implBlock,
                moduleDecl.uses);*/
        }
    }

    const std::unique_ptr<racc::errors::ErrorHandler> errorHandler = std::make_unique<racc::errors::ConsoleErrorHandler>();
    for (const auto &error: source->errors) {
        errorHandler->handleError(error, sources);
    }

    return 0;
}
