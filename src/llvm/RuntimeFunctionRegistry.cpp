#include "../../include/llvm/RuntimeFunctionRegistry.hpp"

void RuntimeFunctionRegistry::declareSmartPointerFunctions() {
    auto voidType = llvm::Type::getVoidTy(context);
    auto i32Type = llvm::Type::getInt32Ty(context);
    auto ptrType = llvm::Type::getInt8PtrTy(context);

    declareFunction(LLVMLabels::UNIQUE_PTR_RELEASE, voidType, {ptrType});
    declareFunction(LLVMLabels::SHARED_PTR_RETAIN, voidType, {ptrType});
    declareFunction(LLVMLabels::SHARED_PTR_RELEASE, voidType, {ptrType});
    declareFunction(LLVMLabels::SHARED_PTR_USE_COUNT, i32Type, {ptrType});
    declareFunction(LLVMLabels::SMART_PTR_MALLOC, ptrType, {i32Type});
    declareFunction(LLVMLabels::WEAK_PTR_RELEASE, voidType, {ptrType});
}

void RuntimeFunctionRegistry::declareFunction(const std::string& name, 
    llvm::Type* returnType, const std::vector<llvm::Type*>& params, bool variadic) {
    
    llvm::FunctionType* funcType = llvm::FunctionType::get(
        returnType,  
        params,         
        variadic
    );

    llvm::Function::Create(
        funcType, 
        llvm::Function::ExternalLinkage,
        name, 
        &_module);

}

llvm::Function* RuntimeFunctionRegistry::getFunction(const std::string& name) const {
    return _module.getFunction(name);
}