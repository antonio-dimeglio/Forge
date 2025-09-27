#include "../../include/llvm/MemoryManager.hpp"

MemoryManager::MemoryManager(llvm::LLVMContext& context,
                           llvm::IRBuilder<>& builder,
                           llvm::Module& _module,
                           ScopeManager& scopeManager,
                           RuntimeFunctionRegistry& rfRegistry)
    : context(context), builder(builder), _module(_module),
      scopeManager(scopeManager), rfRegistry(rfRegistry) {
    // Initialize with global scope
    smartPointerScopes.push_back({});
}

void MemoryManager::createSmartPointerVariable(const VariableDeclaration& node, llvm::Value* expressionValue) {
    auto varName = node.variable.getValue();
    auto smartPtrType = LLVMTypeSystem::getLLVMType(context, node.type);

    // 1. Allocate the smart pointer struct on the stack
    llvm::AllocaInst* smartPtr = builder.CreateAlloca(smartPtrType, nullptr, varName);

    // 2. Use the provided expression value
    auto value = expressionValue;

    // 3. Allocate memory on heap for the actual value
    llvm::Function* mallocFunc = rfRegistry.getFunction(LLVMLabels::SMART_PTR_MALLOC); 
    llvm::Value* heapPtr = builder.CreateCall(mallocFunc, {builder.getInt32(4)}); // sizeof(int)

    // 4. Store the value in heap memory
    builder.CreateStore(value, heapPtr);

    // 5. Store heap pointer in smart pointer struct
    if (node.type.smartPointerType == SmartPointerType::Unique) {
        // For unique_ptr: data is field 0
        llvm::Value* dataFieldPtr = builder.CreateGEP(smartPtrType, smartPtr, {builder.getInt32(0), builder.getInt32(0)});
        builder.CreateStore(heapPtr, dataFieldPtr);

    } else if (node.type.smartPointerType == SmartPointerType::Shared) {
        // For shared_ptr: refcount is field 0, data is field 1
        llvm::Value* refcountPtr = builder.CreateGEP(smartPtrType, smartPtr, {builder.getInt32(0), builder.getInt32(0)});
        builder.CreateStore(builder.getInt32(1), refcountPtr);

        llvm::Value* dataFieldPtr = builder.CreateGEP(smartPtrType, smartPtr, {builder.getInt32(0), builder.getInt32(1)});
        builder.CreateStore(heapPtr, dataFieldPtr);
    }

    // 6. Register the variable in scope
    scopeManager.declare(varName, smartPtr, node.type);
    smartPointerScopes.back().push_back({smartPtr, node.type.smartPointerType});
}

void MemoryManager::generateSmartPointerCleanup() {
    if (smartPointerScopes.empty()) return;

    auto& currentScope = smartPointerScopes.back();
    for (auto& [smartPtr, type] : currentScope) {
        if (type == SmartPointerType::Unique) {
            llvm::Function* releaseFunc = rfRegistry.getFunction(LLVMLabels::UNIQUE_PTR_RELEASE);
            builder.CreateCall(releaseFunc, {smartPtr});
        } else if (type == SmartPointerType::Shared) {
            llvm::Function* releaseFunc =  rfRegistry.getFunction(LLVMLabels::SHARED_PTR_RELEASE);
            builder.CreateCall(releaseFunc, {smartPtr});
        } else if (type == SmartPointerType::Weak) {
            llvm::Function* releaseFunc = rfRegistry.getFunction(LLVMLabels::WEAK_PTR_RELEASE);
            builder.CreateCall(releaseFunc, {smartPtr});
        }
    }
    currentScope.clear();
}

void MemoryManager::trackSmartPointer(llvm::Value* smartPtr, SmartPointerType type) {
    smartPointerScopes.back().push_back({smartPtr, type});
}

void MemoryManager::addDeferredCall(llvm::Function* func, std::vector<llvm::Value*> args) {
    deferredCalls.push_back({func, std::move(args)});
}

void MemoryManager::emitDeferredCalls() {
    // Execute deferred calls in reverse order (LIFO - Last In, First Out)
    for (auto it = deferredCalls.rbegin(); it != deferredCalls.rend(); ++it) {
        builder.CreateCall(it->function, it->args);
    }
    deferredCalls.clear();
}

void MemoryManager::enterScope() {
    smartPointerScopes.push_back({});
}

void MemoryManager::exitScope() {
    generateSmartPointerCleanup();
    if (smartPointerScopes.size() > 1) {
        smartPointerScopes.pop_back();
    }
}

void MemoryManager::copySharedPointerVariable(const VariableDeclaration& node, llvm::Value* sourceSharedPtr) {
    // TODO: Your implementation here
    // 1. Create stack allocation for new shared pointer
    // 2. Copy fields from source to destination
    // 3. Call shared_ptr_retain() to increment ref_count
    // 4. Register in scope
    std::string varName = node.variable.getValue();
    auto smartPtrType = LLVMTypeSystem::getLLVMType(context, node.type);

    llvm::AllocaInst* sharedPtr = builder.CreateAlloca(smartPtrType, nullptr, varName);
    llvm::Value* sourceRefCountPtr = builder.CreateGEP(smartPtrType, sourceSharedPtr, {builder.getInt32(0), builder.getInt32(0)});
    llvm::Value* sourceRefCount = builder.CreateLoad(llvm::Type::getInt32Ty(context), sourceRefCountPtr);

    llvm::Value* sourceDataPtr = builder.CreateGEP(smartPtrType, sourceSharedPtr, {builder.getInt32(0), builder.getInt32(1)});
    llvm::Value* sourceData = builder.CreateLoad(llvm::PointerType::getUnqual(context), sourceDataPtr);

    // Set ref_count in destination (field 0)
    llvm::Value* destRefCountPtr = builder.CreateGEP(smartPtrType, sharedPtr, {builder.getInt32(0), builder.getInt32(0)});
    builder.CreateStore(sourceRefCount, destRefCountPtr);

    // Set data pointer in destination (field 1)
    llvm::Value* destDataPtr = builder.CreateGEP(smartPtrType, sharedPtr, {builder.getInt32(0), builder.getInt32(1)});
    builder.CreateStore(sourceData, destDataPtr);

    llvm::Function* retainFunc = rfRegistry.getFunction(LLVMLabels::SHARED_PTR_RETAIN);
    builder.CreateCall(retainFunc, {sharedPtr});

    scopeManager.declare(varName, sharedPtr, node.type);
    smartPointerScopes.back().push_back({sharedPtr, node.type.smartPointerType});
}