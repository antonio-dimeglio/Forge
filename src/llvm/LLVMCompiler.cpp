#include "../../include/llvm/LLVMCompiler.hpp"
#include "../../include/llvm/BinaryOperationHandler.hpp"
#include "../../include/llvm/LLVMTypeSystem.hpp"
#include "../../include/llvm/ErrorReporter.hpp"
#include "../../include/llvm/LLVMLabels.hpp"
#include <iostream>
#include <string>
#include <optional>

// LLVM target generation includes
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>

LLVMCompiler::LLVMCompiler() : context(), builder(context) {
    _module = std::make_unique<llvm::Module>(LLVMLabels::MAIN_LABEL, context);
    rfRegistry = std::make_unique<RuntimeFunctionRegistry>(context, *_module);
    memoryManager = std::make_unique<MemoryManager>(context, builder, *_module, scopeManager, *rfRegistry);
    expressionCodeGen = std::make_unique<ExpressionCodeGenerator>(context, builder, scopeManager, *rfRegistry, *_module);
    statementCodeGen = std::make_unique<StatementCodeGenerator>(context, builder, *_module, scopeManager, *rfRegistry, *expressionCodeGen, *memoryManager);
}

void LLVMCompiler::compile(const Statement& ast) {
    rfRegistry->declareSmartPointerFunctions();
    ast.accept(*this);
}

// Dispatch to StatementCodeGenerator
void LLVMCompiler::visit(const Statement& node) {
    statementCodeGen->generate(node);
}

// Dispatch to ExpressionCodeGenerator
llvm::Value* LLVMCompiler::visit(const Expression& node) {
    return expressionCodeGen->generate(node);
}

llvm::Module* LLVMCompiler::getModule() const {
    return _module.get();
}

void LLVMCompiler::printModule() const {
    _module->print(llvm::outs(), nullptr);
}

void LLVMCompiler::generateObjectFile(const std::string& filename) {
    // Initialize all target info
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    // Get target triple for current machine
    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    _module->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
    if (!Target) {
        throw std::runtime_error("Failed to lookup target: " + Error);
    }

    auto CPU = "generic";
    auto Features = "";
    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    _module->setDataLayout(TargetMachine->createDataLayout());

    // Open output file
    std::error_code EC;
    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
    if (EC) {
        throw std::runtime_error("Could not open file: " + EC.message());
    }

    // Create pass to emit object file
    llvm::legacy::PassManager pass;
    auto FileType = llvm::CGFT_ObjectFile;
    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        throw std::runtime_error("TargetMachine can't emit a file of this type");
    }

    // Run the pass
    pass.run(*_module);
    dest.flush();
}