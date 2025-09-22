#include "../../../include/backends/vm/VirtualMachine.hpp"
#include "../../../include/backends/vm/BytecodeCompiler.hpp"
#include <iostream>

static const std::unordered_map<TokenType, TypedValue::Type> typeTable = {
    {TokenType::INT, TypedValue::Type::INT},
    {TokenType::FLOAT, TypedValue::Type::FLOAT},
    {TokenType::DOUBLE, TypedValue::Type::DOUBLE},
    {TokenType::BOOL, TypedValue::Type::BOOL},
    {TokenType::TRUE, TypedValue::Type::BOOL},
    {TokenType::FALSE, TypedValue::Type::BOOL},
    {TokenType::STR, TypedValue::Type::STRING},
    {TokenType::STRING, TypedValue::Type::STRING}
};

static const std::unordered_map<TokenType,
      std::unordered_map<TypedValue::Type, OPCode>> binaryOpcodeTable = {
    {TokenType::PLUS, {
        {TypedValue::Type::INT, OPCode::ADD_INT},
        {TypedValue::Type::FLOAT, OPCode::ADD_FLOAT},
        {TypedValue::Type::DOUBLE, OPCode::ADD_DOUBLE},
        {TypedValue::Type::STRING, OPCode::ADD_STRING}
    }},
    {TokenType::MINUS, {
        {TypedValue::Type::INT, OPCode::SUB_INT},
        {TypedValue::Type::FLOAT, OPCode::SUB_FLOAT},
        {TypedValue::Type::DOUBLE, OPCode::SUB_DOUBLE}
    }},
    {TokenType::MULT, {
        {TypedValue::Type::INT, OPCode::MULT_INT},
        {TypedValue::Type::FLOAT, OPCode::MULT_FLOAT},
        {TypedValue::Type::DOUBLE, OPCode::MULT_DOUBLE}
    }},
    {TokenType::DIV, {
        {TypedValue::Type::INT, OPCode::DIV_INT},
        {TypedValue::Type::FLOAT, OPCode::DIV_FLOAT},
        {TypedValue::Type::DOUBLE, OPCode::DIV_DOUBLE}
    }},
    {TokenType::EQUAL_EQUAL, {
        {TypedValue::Type::INT, OPCode::EQ_INT},
        {TypedValue::Type::DOUBLE, OPCode::EQ_DOUBLE}
    }},
    {TokenType::LESS, {
        {TypedValue::Type::INT, OPCode::LT_INT},
        {TypedValue::Type::DOUBLE, OPCode::LT_DOUBLE}
    }},
    {TokenType::GREATER, {
        {TypedValue::Type::INT, OPCode::GT_INT},
        {TypedValue::Type::DOUBLE, OPCode::GT_DOUBLE}
    }}
};

static const std::unordered_map<TokenType,
      std::unordered_map<TypedValue::Type, OPCode>> unaryOpcodeTable = {
    {TokenType::MINUS, {
        {TypedValue::Type::INT, OPCode::NEG_INT},
        {TypedValue::Type::FLOAT, OPCode::NEG_FLOAT},
        {TypedValue::Type::DOUBLE, OPCode::NEG_DOUBLE}
    }},
    {TokenType::NOT, {
        {TypedValue::Type::BOOL, OPCode::NOT_BOOL}
    }}
};

TypedValue BytecodeCompiler::inferType(const Token& token){
    
    if (token.getType() == TokenType::NUMBER) {
        std::string value = token.getValue();
        TypedValue val;

        if (value.find('.') != std::string::npos) {
            if (value.back() == 'f') {
                // Float suffix found
                val.type = TypedValue::Type::FLOAT;
                val.value.f = std::stof(value.substr(0, value.length()-1)); // Remove 'f'
            } else {
                // Default to double for decimal numbers  
                val.type = TypedValue::Type::DOUBLE;
                val.value.d = std::stod(value);
            }
        } else {
            val.type = TypedValue::Type::INT;
            val.value.i = std::stoi(value);
        }
        return val;
    }

    auto it = typeTable.find(token.getType());
    if (it != typeTable.end()) {
        TypedValue::Type type = it->second;
        switch (type) {
            case TypedValue::Type::INT: {
                TypedValue val;
                val.type = type;
                val.value.i = std::stoi(token.getValue());
                return val;
            }
            case TypedValue::Type::FLOAT: {
                TypedValue val;
                val.type = type;
                val.value.f = std::stof(token.getValue());
                return val;
            }
            case TypedValue::Type::DOUBLE: {
                TypedValue val;
                val.type = type;
                val.value.d = std::stod(token.getValue());
                return val;
            }
            case TypedValue::Type::BOOL: {
                TypedValue val;
                val.type = type;
                val.value.b = token.getValue() == "true" ? true : false;
                return val;
            }
            case TypedValue::Type::STRING: {
                tempStringPool.push_back(token.getValue());
                TypedValue val;
                val.type = type;
                val.value.string_id = tempStringPool.size() - 1;
                return val;
            }
        };
    }
    
    throw RuntimeException("Could not infer type for " + token.getValue() +
        "at line " + std::to_string(token.getLine()) + ":" + std::to_string(token.getColumn()));
}


BytecodeCompiler::CompiledProgram BytecodeCompiler::compile(std::unique_ptr<Statement> ast) {
    instructions.clear();
    constants.clear();
    tempStringPool.clear();
    nextSlot = 0;

    ast->accept(*this);
    emit(OPCode::HALT, -1);
    return CompiledProgram {
        .instructions = instructions,
        .constants = constants,
        .strings  = tempStringPool
    };
}

int BytecodeCompiler::addConstant(TypedValue value) {
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

TypedValue::Type BytecodeCompiler::compileExpression(const Expression& node) {
    node.accept(*this);
    return currentExpressionType;
}

TypedValue::Type BytecodeCompiler::compileLiteral(const LiteralExpression& node) {
    TypedValue value = inferType(node.value);

    switch (value.type) {
        case TypedValue::Type::INT: {
            int constantIndex = addConstant(value);
            emit(OPCode::LOAD_INT, constantIndex);
            break;
        }
        case TypedValue::Type::FLOAT: {
            int constantIndex = addConstant(value);
            emit(OPCode::LOAD_FLOAT, constantIndex);
            break;
        }
        case TypedValue::Type::DOUBLE: {
            int constantIndex = addConstant(value);
            emit(OPCode::LOAD_DOUBLE, constantIndex);
            break;
        }
        case TypedValue::Type::BOOL: {
            int constantIndex = addConstant(value);
            emit(OPCode::LOAD_BOOL, constantIndex);
            break;
        }
        case TypedValue::Type::STRING:
            // Strings use string pool directly, not constants pool
            emit(OPCode::LOAD_STRING, value.value.string_id);
            break;
    }
    currentExpressionType = value.type;
    return value.type;
}

TypedValue::Type BytecodeCompiler::compileBinary(const BinaryExpression& node) {
    TypedValue::Type leftType = compileExpression(*(node.left));
    TypedValue::Type rightType = compileExpression(*(node.right));

    if (leftType != rightType) {
        throw RuntimeException("Type mismatch in binary operation");
    }

    OPCode opcode = getOpCode(node.operator_.getType(), leftType);
    emit(opcode);

    currentExpressionType = leftType;
    return leftType; 
}

TypedValue::Type BytecodeCompiler::compileUnary(const UnaryExpression& node) {
    TypedValue::Type operandType = compileExpression(*(node.operand));

    OPCode opcode = getOpCode(node.operator_.getType(), operandType, true);
    emit(opcode);

    currentExpressionType = operandType;
    return operandType;
}

TypedValue::Type BytecodeCompiler::compileIdentifier(const IdentifierExpression& node) {
    auto variable = node.name;
    auto it = symbolTable.find(variable);

    if (it == symbolTable.end()) {
        throw RuntimeException("Undefined variable: " + node.name);
    }

    emit(OPCode::LOAD_LOCAL, it->second.slot);
    return it->second.type;
}   

OPCode BytecodeCompiler::getOpCode(TokenType op, TypedValue::Type type, bool isUnary) {
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

TypedValue::Type BytecodeCompiler::compileFunctionCall(const FunctionCall& /* node */) {
    throw RuntimeException("Function calls not yet implemented");
}

void BytecodeCompiler::compileVariableDeclaration(const VariableDeclaration& node) {
    auto variable = node.variable.getValue();

    TypedValue::Type varType = typeTable.find(node.type.getType())->second;

    VariableInfo info = {nextSlot++, varType};
    symbolTable[variable] = info;

    node.expr->accept(*this);
    emit(OPCode::STORE_LOCAL, info.slot);
}

void BytecodeCompiler::compileAssignment(const Assignment& node) {
    auto variable = node.variable.getValue();

    auto it = symbolTable.find(variable);
    if (it == symbolTable.end()) {
        throw RuntimeException("Tried to assign value to un-initialized variable at " +
            std::to_string(node.variable.getLine()) + ":" + std::to_string(node.variable.getColumn()));
    }

    node.expr->accept(*this);
    emit(OPCode::STORE_LOCAL, it->second.slot);
}

void BytecodeCompiler::compileIfStatement(const IfStatement& node) { 
    compileExpression(*node.condition);

    int jumpIfFalsePos = instructions.size(); 
    emit(OPCode::JUMP_IF_FALSE, 0);

    for (const auto& stmt : node.body) {
        stmt->accept(*this);
    }

    if (!node.elseBody.empty()) {
        int jumpToEndPos = instructions.size();
        emit(OPCode::JUMP, 0);

        instructions[jumpIfFalsePos].operand = instructions.size();
        for (const auto& stmt : node.elseBody) {
            stmt->accept(*this);
        }
        instructions[jumpToEndPos].operand = instructions.size();
    } else {
        instructions[jumpIfFalsePos].operand = instructions.size();
    }
    
}