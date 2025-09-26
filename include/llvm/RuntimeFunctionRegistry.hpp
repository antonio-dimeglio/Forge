#pragma once

#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>

#include "LLVMLabels.hpp"
class RuntimeFunctionRegistry {
    private:
        llvm::LLVMContext& context;
        llvm::Module& _module;

        
        void declareFunction(const std::string& name, 
                            llvm::Type* returnType, 
                            const std::vector<llvm::Type*>& params,
                            bool variadic = false);

    public:
        RuntimeFunctionRegistry(llvm::LLVMContext& context, llvm::Module& _module) :
            context(context), _module(_module) {}

        void declareSmartPointerFunctions();

        llvm::Function* getFunction(const std::string& name) const;
};