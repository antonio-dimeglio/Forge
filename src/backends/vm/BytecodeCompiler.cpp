#include "../../../include/backends/vm/VirtualMachine.hpp"
#include "../../../include/backends/vm/BytecodeCompiler.hpp"
#include <iostream>

BytecodeCompiler::BytecodeCompiler(VirtualMachine& vm) : vm(vm) {}

static const std::unordered_map<TokenType, std::unordered_map<ValueType, OPCode>> binaryOpcodeTable = {
    {TokenType::PLUS, {
        {ValueType::INT, OPCode::ADD_INT},
        {ValueType::FLOAT, OPCode::ADD_FLOAT},
        {ValueType::DOUBLE, OPCode::ADD_DOUBLE},
        {ValueType::OBJECT, OPCode::ADD_STRING}  // For string concatenation
    }},
    {TokenType::MINUS, {
        {ValueType::INT, OPCode::SUB_INT},
        {ValueType::FLOAT, OPCode::SUB_FLOAT},
        {ValueType::DOUBLE, OPCode::SUB_DOUBLE}
    }},
    {TokenType::MULT, {
        {ValueType::INT, OPCode::MULT_INT},
        {ValueType::FLOAT, OPCode::MULT_FLOAT},
        {ValueType::DOUBLE, OPCode::MULT_DOUBLE}
    }},
    {TokenType::DIV, {
        {ValueType::INT, OPCode::DIV_INT},
        {ValueType::FLOAT, OPCode::DIV_FLOAT},
        {ValueType::DOUBLE, OPCode::DIV_DOUBLE}
    }},
    {TokenType::EQUAL_EQUAL, {
        {ValueType::INT, OPCode::EQ_INT},
        {ValueType::DOUBLE, OPCode::EQ_DOUBLE},
        {ValueType::FLOAT, OPCode::EQ_FLOAT},
        {ValueType::BOOL, OPCode::EQ_BOOL},
        {ValueType::OBJECT, OPCode::EQ_STRING}  // For string comparison
    }},
    {TokenType::GREATER, {
        {ValueType::INT, OPCode::GT_INT},
        {ValueType::DOUBLE, OPCode::GT_DOUBLE},
        {ValueType::FLOAT, OPCode::GT_FLOAT},
    }},
    {TokenType::LESS, {
        {ValueType::INT, OPCode::LT_INT},
        {ValueType::DOUBLE, OPCode::LT_DOUBLE},
        {ValueType::FLOAT, OPCode::LT_FLOAT},
    }},
    {TokenType::GEQ, {
        {ValueType::INT, OPCode::GEQ_INT},
        {ValueType::DOUBLE, OPCode::GEQ_DOUBLE},
        {ValueType::FLOAT, OPCode::GEQ_FLOAT},
    }},
    {TokenType::LEQ, {
        {ValueType::INT, OPCode::LEQ_INT},
        {ValueType::DOUBLE, OPCode::LEQ_DOUBLE},
        {ValueType::FLOAT, OPCode::LEQ_FLOAT},
    }},
    {TokenType::BITWISE_AND, {
        {ValueType::INT, OPCode::BITWISE_AND_INT},
        {ValueType::BOOL, OPCode::BITWISE_AND_BOOL},
    }},
    {TokenType::BITWISE_OR, {
        {ValueType::INT, OPCode::BITWISE_OR_INT},
        {ValueType::BOOL, OPCode::BITWISE_OR_BOOL}
    }},
    {TokenType::BITWISE_XOR, {
        {ValueType::INT, OPCode::BITWISE_XOR_INT},
        {ValueType::BOOL, OPCode::BITWISE_XOR_BOOL}
    }}
};

static const std::unordered_map<TokenType, ValueType> typeTable = {
    {TokenType::INT, ValueType::INT},
    {TokenType::FLOAT, ValueType::FLOAT},
    {TokenType::DOUBLE, ValueType::DOUBLE},
    {TokenType::BOOL, ValueType::BOOL},
    {TokenType::STR, ValueType::OBJECT},
    {TokenType::STRING, ValueType::OBJECT},
    {TokenType::ARRAY, ValueType::OBJECT}  // Arrays are objects
};


static const std::unordered_map<TokenType, std::unordered_map<ValueType, OPCode>> unaryOpcodeTable = {
    {TokenType::MINUS, {
        {ValueType::INT, OPCode::NEG_INT},
        {ValueType::FLOAT, OPCode::NEG_FLOAT},
        {ValueType::DOUBLE, OPCode::NEG_DOUBLE}
    }},
    {TokenType::NOT, {
        {ValueType::BOOL, OPCode::NOT_BOOL}
    }}
};

Value BytecodeCompiler::inferType(const Token& token){
    if (token.getType() == TokenType::NUMBER) {
        std::string value = token.getValue();

        if (value.find('.') != std::string::npos) {
            if (value.back() == 'f') {
                // Float suffix found
                return createFloat(std::stof(value.substr(0, value.length()-1)));
            } else {
                // Default to double for decimal numbers
                return createDouble(std::stod(value));
            }
        } else {
            return createInt(std::stoi(value));
        }
    }

    // Check typeTable for explicit type tokens (TRUE, FALSE, etc.)
    auto it = typeTable.find(token.getType());
    if (it != typeTable.end()) {
        ValueType type = it->second;
        switch (type) {
            case ValueType::INT:
                return createInt(std::stoi(token.getValue()));
            case ValueType::FLOAT:
                return createFloat(std::stof(token.getValue()));
            case ValueType::DOUBLE:
                return createDouble(std::stod(token.getValue()));
            case ValueType::BOOL:
                return createBool(token.getValue() == "true");
            case ValueType::OBJECT: {
                // Handle string literals
                StringObject* stringObj = vm.getHeap().allocateString(token.getValue());
                return createObject(stringObj);
            }
        }
    }

    // Fallback for explicit cases not in typeTable
    switch (token.getType()){
        case TokenType::TRUE:
            return createBool(true);
        case TokenType::FALSE:
            return createBool(false);
        case TokenType::STRING: {
            StringObject* stringObj = vm.getHeap().allocateString(token.getValue());
            return createObject(stringObj);
        }
        default:
            throw RuntimeException("Could not infer type for " + token.getValue() +
                " at line " + std::to_string(token.getLine()) + ":" + std::to_string(token.getColumn()));
    }
}


BytecodeCompiler::CompiledProgram BytecodeCompiler::compile(std::unique_ptr<Statement> ast) {
    instructions.clear();
    constants.clear();
    nextSlot = 0;
    globalSlot = 0;
    scopeStack.clear();
    scopeStack.push_back({});

    ast->accept(*this);
    emit(OPCode::HALT, -1);
    return CompiledProgram {
        .instructions = instructions,
        .constants = constants
    };
}

int BytecodeCompiler::addConstant(Value value) {
    constants.push_back(value);
    return constants.size() - 1;
}

void BytecodeCompiler::emit(OPCode opcode, int operand) {
    instructions.push_back(
        Instruction {
            .opcode = opcode,
            .operand = operand
        }
    );
}

ValueType BytecodeCompiler::compileExpression(const Expression& node) {
    node.accept(*this);
    return currentExpressionType;
}

ValueType BytecodeCompiler::compileLiteral(const LiteralExpression& node) {
    Value value = inferType(node.value);

    switch (value.type) {
        case ValueType::INT: {
            int constantIndex = addConstant(value);
            emit(OPCode::LOAD_INT, constantIndex);
            break;
        }
        case ValueType::FLOAT: {
            int constantIndex = addConstant(value);
            emit(OPCode::LOAD_FLOAT, constantIndex);
            break;
        }
        case ValueType::DOUBLE: {
            int constantIndex = addConstant(value);
            emit(OPCode::LOAD_DOUBLE, constantIndex);
            break;
        }
        case ValueType::BOOL: {
            int constantIndex = addConstant(value);
            emit(OPCode::LOAD_BOOL, constantIndex);
            break;
        }
        case ValueType::OBJECT: {
            int constantIndex = addConstant(value);
            Object* obj = asObject(value);
            switch (obj->type) {
                case STRING:
                    emit(OPCode::LOAD_STRING, constantIndex);
                    break;
                default:
                    throw RuntimeException("Unknown object type in literal");
            }
            break;
        }
    }
    currentExpressionType = value.type;
    return value.type;
}

ValueType BytecodeCompiler::compileBinary(const BinaryExpression& node) {
    ValueType leftType = compileExpression(*(node.left));
    ValueType rightType = compileExpression(*(node.right));

    if (leftType != rightType) {
        throw RuntimeException("Type mismatch in binary operation");
    }

    OPCode opcode = getOpCode(node.operator_.getType(), leftType);
    emit(opcode, -1);

    currentExpressionType = leftType;
    return leftType; 
}

ValueType BytecodeCompiler::compileUnary(const UnaryExpression& node) {
    ValueType operandType = compileExpression(*(node.operand));

    OPCode opcode = getOpCode(node.operator_.getType(), operandType, true);
    emit(opcode, -1);

    currentExpressionType = operandType;
    return operandType;
}

ValueType BytecodeCompiler::compileIdentifier(const IdentifierExpression& node) {
    auto variable = node.name;
    auto it = lookupVariable(variable);

    if (it == nullptr) {
        throw RuntimeException("Undefined variable: " + node.name);
    }
    if (scopeStack.size() == 1) {
         
        emit(OPCode::LOAD_GLOBAL, it->slot);
    } else {

        auto globalIt = scopeStack[0].find(variable);
        if (globalIt != scopeStack[0].end()) {
            emit(OPCode::LOAD_GLOBAL, it->slot);
        } else {
            emit(OPCode::LOAD_LOCAL, it->slot);
        }
    }

    currentExpressionType = it->type;
    return it->type;
}   

OPCode BytecodeCompiler::getOpCode(TokenType op, ValueType type, bool isUnary) {
    const auto& table = isUnary ? unaryOpcodeTable : binaryOpcodeTable;

    auto opIt = table.find(op);
    if (opIt == table.end()) {
        throw RuntimeException("Unsupported operator");
    }

    auto typeIt = opIt->second.find(type);
    if (typeIt == opIt->second.end()) {
        throw RuntimeException("Unsupported type for operator");
    }

    return typeIt->second;
}

ValueType BytecodeCompiler::compileFunctionCall(const FunctionCall& node) {
    for (const auto& arg : node.arguments) {
        compileExpression(*arg);
    }

    auto it = functionTable.find(node.functionName);
    if (it == functionTable.end()) {
        throw RuntimeException("Unknown function: " + node.functionName);
    }

    int functionIndex = it->second;
    emit(OPCode::CALL, functionIndex);

    return ValueType::INT;
}

void BytecodeCompiler::compileVariableDeclaration(const VariableDeclaration& node) {
    auto variable = node.variable.getValue();

    // Look up the variable type safely
    auto typeIt = typeTable.find(node.type.getType());
    if (typeIt == typeTable.end()) {
        // Handle special case of Array[T] types (parsed as IDENTIFIER with value "Array")
        if (node.type.getType() == TokenType::IDENTIFIER && node.type.getValue() == "Array") {
            // Array[T] types are objects
            typeIt = typeTable.find(TokenType::ARRAY);
        } else {
            throw RuntimeException("Unknown type: " + node.type.getValue() + " at line " + std::to_string(node.type.getLine()));
        }
    }
    ValueType varType = typeIt->second;
    // Check if global scope or not
    if (scopeStack.size() == 1) { 
        VariableInfo info = {static_cast<int>(globalSlot++), varType};
        declareVariable(variable, info);
        node.expr->accept(*this);
        emit(OPCode::STORE_GLOBAL, info.slot);
    } else {
        VariableInfo info = {nextSlot++, varType};
        declareVariable(variable, info);
        node.expr->accept(*this);
        emit(OPCode::STORE_LOCAL, info.slot);
    }
}

void BytecodeCompiler::compileAssignment(const Assignment& node) {
    auto variable = node.variable.getValue();
    auto it = lookupVariable(variable);

    if (it == nullptr) {
        throw RuntimeException("Tried to assign value to un-initialized variable at " +
            std::to_string(node.variable.getLine()) + ":" + std::to_string(node.variable.getColumn()));
    }


    node.expr->accept(*this);

    // If we are in global scope directly reassign it
    if (scopeStack.size() == 1) {
        emit(OPCode::STORE_GLOBAL, it->slot);
    } else {
        // If we are not in the global scope but the variable in question is
        // update it
        auto globalIt = scopeStack[0].find(variable);
        if (globalIt != scopeStack[0].end()) {
            emit(OPCode::STORE_GLOBAL, it->slot);
        } else {
            emit(OPCode::STORE_LOCAL, it->slot);
        }
    }
}

void BytecodeCompiler::compileIndexAssignment(const IndexAssignment& node) {
    // For arr[index] = value, we need to:
    // 1. Compile the rvalue (value)
    // 2. Extract array and index from lvalue
    // 3. Emit ARRAY_SET

    // The lvalue should be an IndexAccessExpression (arr[index])
    auto indexExpr = dynamic_cast<const IndexAccessExpression*>(node.lvalue.get());
    if (!indexExpr) {
        throw RuntimeException("Invalid lvalue in index assignment");
    }

    // VM pops in order: value, index, array
    // So push in reverse order: array, index, value

    // 1. Compile the array (first to push)
    compileExpression(*indexExpr->array);
    // 2. Compile the index (second to push)
    compileExpression(*indexExpr->index);
    // 3. Compile the value to assign (last to push, first to pop)
    compileExpression(*node.rvalue);

    // Emit ARRAY_SET (expects stack: [array, index, value] where value is TOS)
    emit(OPCode::ARRAY_SET);
}

void BytecodeCompiler::compileIfStatement(const IfStatement& node) {
    compileExpression(*node.condition);

    int jumpIfFalsePos = instructions.size();
    emit(OPCode::JUMP_IF_FALSE, 0);

    node.thenBlock->accept(*this);

    if (node.elseBlock != nullptr) {
        int jumpToEndPos = instructions.size();
        emit(OPCode::JUMP, 0);

        instructions[jumpIfFalsePos].operand = instructions.size();
        node.elseBlock->accept(*this);
        instructions[jumpToEndPos].operand = instructions.size();
    } else {
        instructions[jumpIfFalsePos].operand = instructions.size();
    }
}

void BytecodeCompiler::compileWhileStatement(const WhileStatement& node) {
    int loopStart = instructions.size(); 
    compileExpression(*node.condition);

    int jumpIfFalseCondition = instructions.size();
    emit(OPCode::JUMP_IF_FALSE, 0);

    node.body->accept(*this);
    emit(OPCode::JUMP, loopStart);

    instructions[jumpIfFalseCondition].operand = instructions.size();
}

void BytecodeCompiler::enterScope() {
    scopeStack.push_back({}); 
}

void BytecodeCompiler::exitScope() {
    scopeStack.pop_back(); 
}

void BytecodeCompiler::compileBlockStatement(const BlockStatement& node) {
    enterScope();
    for (const auto& stmt : node.statements) {
        stmt->accept(*this);
    }
    exitScope();
}

void BytecodeCompiler::compileFunctionDefinition(const FunctionDefinition& node) {
    auto savedInstructions = std::move(instructions);
    auto savedNextSlot = nextSlot;

    // Create function object early for recursive calls
    FunctionObject* functionObj = vm.getHeap().allocateFunction(
        node.functionName.getValue(),
        static_cast<int>(node.parameters.size())
    );

    Value functionValue = createObject(functionObj);

    instructions.clear();
    
    nextSlot = 0;

    int functionConstantIndex = addConstant(functionValue);
    functionTable[node.functionName.getValue()] = functionConstantIndex;

    enterScope();

    for (const auto& param : node.parameters) {
        VariableInfo paramInfo;

        // Convert type token to ValueType
        switch (param.type.getType()) {
            case TokenType::INT:
                paramInfo.type = ValueType::INT;
                break;
            case TokenType::FLOAT:
                paramInfo.type = ValueType::FLOAT;
                break;
            case TokenType::DOUBLE:
                paramInfo.type = ValueType::DOUBLE;
                break;
            case TokenType::BOOL:
                paramInfo.type = ValueType::BOOL;
                break;
            case TokenType::STR:
                paramInfo.type = ValueType::OBJECT;
                break;
            case TokenType::ARRAY:
                paramInfo.type = ValueType::OBJECT;
                break;
            default:
                throw RuntimeException("Unknown parameter type");
        }

        paramInfo.slot = nextSlot++;
        declareVariable(param.name.getValue(), paramInfo);
    }

    node.body->accept(*this);

    exitScope();

    functionObj->instructions = instructions;
    functionObj->constants = constants;

    instructions = std::move(savedInstructions);
    nextSlot = savedNextSlot;
    int mainFunctionIndex = addConstant(functionValue);
    functionTable[node.functionName.getValue()] = mainFunctionIndex;  
    nextSlot = savedNextSlot;
}

void BytecodeCompiler::compileReturnStatement(const ReturnStatement& node) {
    compileExpression(*node.returnValue);

    emit(OPCode::RETURN, -1);
}


BytecodeCompiler::VariableInfo* BytecodeCompiler::lookupVariable(const std::string& name) {
    for (int i = scopeStack.size() - 1; i >= 0; i--) {
        auto it = scopeStack[i].find(name);
        if (it != scopeStack[i].end()) {
            return &it->second;
        }
    }
    return nullptr; 
}

void BytecodeCompiler::declareVariable(const std::string& name, const VariableInfo& info) {
    scopeStack.back()[name] = info;
}

ValueType BytecodeCompiler::compileArrayLiteral(const ArrayLiteralExpression& node) {
    // Create new array
    emit(OPCode::ARRAY_NEW);

    // For array literals with elements like [1, 2, 3]
    // Use ARRAY_PUSH to add each element
    for (const auto& element : node.arrayValues) {
        // Compile the element value
        compileExpression(*element);

        // ARRAY_PUSH expects: [array, value] on stack
        // But we need [array] to stay on top for the next iteration
        // So ARRAY_PUSH should return the array

        emit(OPCode::ARRAY_PUSH);
    }

    currentExpressionType = ValueType::OBJECT; // Arrays are objects
    return currentExpressionType;
}

ValueType BytecodeCompiler::compileIndexAccess(const IndexAccessExpression& node) {
    // Compile the array expression first
    compileExpression(*node.array);
    // Compile the index expression
    compileExpression(*node.index);

    // Emit ARRAY_GET instruction
    emit(OPCode::ARRAY_GET);

    // TODO: In a full type system, we'd return the array's element type
    // For now, assume INT (most array tests use int arrays)
    currentExpressionType = ValueType::INT;
    return currentExpressionType;
}

ValueType BytecodeCompiler::compileMemberAccess(const MemberAccessExpression& node) {
    // Check if this is a static method call on Array type
    auto objectIdent = dynamic_cast<const IdentifierExpression*>(node.object.get());
    if (objectIdent && objectIdent->name == "Array") {
        // Handle Array static method calls
        if (node.memberName == "new" && node.isMethodCall) {
            // Array.new() - create new array
            // Compile arguments (if any)
            for (const auto& arg : node.arguments) {
                compileExpression(*arg);
            }
            // Emit array creation instruction
            emit(OPCode::ARRAY_NEW);
            currentExpressionType = ValueType::OBJECT; // Arrays are objects
            return currentExpressionType;
        }
    }

    // Handle instance method calls and property access
    compileExpression(*node.object);

    if (node.isMethodCall) {
        // Compile method arguments
        for (const auto& arg : node.arguments) {
            compileExpression(*arg);
        }

        // Handle specific array methods
        if (node.memberName == "push") {
            // arr.push(value) - emit ARRAY_PUSH
            emit(OPCode::ARRAY_PUSH);
            currentExpressionType = ValueType::OBJECT;
        } else if (node.memberName == "pop") {
            // arr.pop() - emit ARRAY_POP
            emit(OPCode::ARRAY_POP);
            currentExpressionType = ValueType::INT; // Returns the popped element
        } else if (node.memberName == "length") {
            // arr.length() - get array length
            emit(OPCode::ARRAY_LENGTH);
            currentExpressionType = ValueType::INT;
        } else {
            // Generic method call - placeholder
            currentExpressionType = ValueType::INT;
        }
    } else {
        // Property access like arr.length
        if (node.memberName == "length") {
            emit(OPCode::ARRAY_LENGTH);
            currentExpressionType = ValueType::INT;
        } else {
            currentExpressionType = ValueType::INT; // Placeholder
        }
    }

    return currentExpressionType;
}