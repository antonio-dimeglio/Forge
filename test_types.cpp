#include "include/backend/types/PrimitiveType.hpp"
#include <llvm/IR/LLVMContext.h>
#include <iostream>

int main() {
    // Test your PrimitiveType implementation
    llvm::LLVMContext context;

    forge::types::PrimitiveType intType(TokenType::INT);
    forge::types::PrimitiveType floatType(TokenType::FLOAT);
    forge::types::PrimitiveType stringType(TokenType::STRING);

    std::cout << "Int type: " << intType.toString() << std::endl;
    std::cout << "Float type: " << floatType.toString() << std::endl;
    std::cout << "String type: " << stringType.toString() << std::endl;

    // Test LLVM type conversion
    llvm::Type* intLLVM = intType.toLLVMType(context);
    llvm::Type* floatLLVM = floatType.toLLVMType(context);
    llvm::Type* stringLLVM = stringType.toLLVMType(context);

    std::cout << "LLVM types created successfully!" << std::endl;

    // Test type compatibility
    std::cout << "Int assignable from float: " << intType.isAssignableFrom(floatType) << std::endl;
    std::cout << "Float assignable from int: " << floatType.isAssignableFrom(intType) << std::endl;

    return 0;
}