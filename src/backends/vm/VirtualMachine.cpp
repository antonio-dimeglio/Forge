#include "../../../include/backends/vm/VirtualMachine.hpp"
#include "../../../include/backends/vm/RuntimeException.hpp"
#include "../../../include/backends/vm/Value.hpp"
#include "../../../include/backends/vm/Object.hpp"
#include <iostream>

// New Value-based stack operations
void VirtualMachine::pushValue(Value value) {
    stack.push_back(value);
}

Value VirtualMachine::popValue() {
    if (stack.empty()) {
        throw RuntimeException("Stack underflow");
    }
    Value value = stack.back();
    stack.pop_back();
    return value;
}

Value VirtualMachine::peekValue() {
    if (stack.empty()) {
        throw RuntimeException("Stack underflow");
    }
    return stack.back();
}

Value VirtualMachine::getLocal(size_t idx) {
    if (idx >= locals.size()) {
        throw RuntimeException("Invalid local variable slot: " + std::to_string(idx));
    }
    return locals[idx];
}

void VirtualMachine::reset() {
    stack.clear();
    constants.clear();
    instructions.clear();
    locals.clear();
    while (!callStack.empty()) {
        callStack.pop();
    }
    ip = 0;
}

void VirtualMachine::loadProgram(const std::vector<Instruction>& instructions,
                               const std::vector<Value>& constants) {
    this->instructions = instructions;
    this->constants = constants;
    ip = 0;
    stack.clear();
    locals.clear();
    while (!callStack.empty()) {
        callStack.pop();
    }
}

void VirtualMachine::run() {
    while (ip < instructions.size()) {
        Instruction inst = instructions[ip];

        switch (inst.opcode) {
            case OPCode::LOAD_INT: {
                Value value = constants[inst.operand];
                pushValue(value);
                break;
            }
            case OPCode::LOAD_FLOAT: {
                Value value = constants[inst.operand];
                pushValue(value);
                break;
            }
            case OPCode::LOAD_DOUBLE: {
                Value value = constants[inst.operand];
                pushValue(value);
                break;
            }
            case OPCode::LOAD_BOOL: {
                Value value = constants[inst.operand];
                pushValue(value);
                break;
            }
            case OPCode::LOAD_STRING: {
                Value value = constants[inst.operand];
                pushValue(value);
                break;
            }

            // Arithmetic operations - INT
            case OPCode::ADD_INT: {
                Value right = popValue();
                Value left = popValue();
                Value result = createInt(asInt(left) + asInt(right));
                pushValue(result);
                break;
            }
            case OPCode::SUB_INT: {
                Value right = popValue();
                Value left = popValue();
                Value result = createInt(asInt(left) - asInt(right));
                pushValue(result);
                break;
            }
            case OPCode::MULT_INT: {
                Value right = popValue();
                Value left = popValue();
                Value result = createInt(asInt(left) * asInt(right));
                pushValue(result);
                break;
            }
            case OPCode::DIV_INT: {
                Value right = popValue();
                Value left = popValue();
                Value result = createInt(asInt(left) / asInt(right));
                pushValue(result);
                break;
            }

            // Arithmetic operations - FLOAT
            case OPCode::ADD_FLOAT: {
                Value right = popValue();
                Value left = popValue();
                Value result = createFloat(asFloat(left) + asFloat(right));
                pushValue(result);
                break;
            }
            case OPCode::SUB_FLOAT: {
                Value right = popValue();
                Value left = popValue();
                Value result = createFloat(asFloat(left) - asFloat(right));
                pushValue(result);
                break;
            }
            case OPCode::MULT_FLOAT: {
                Value right = popValue();
                Value left = popValue();
                Value result = createFloat(asFloat(left) * asFloat(right));
                pushValue(result);
                break;
            }
            case OPCode::DIV_FLOAT: {
                Value right = popValue();
                Value left = popValue();
                Value result = createFloat(asFloat(left) / asFloat(right));
                pushValue(result);
                break;
            }

            // Arithmetic operations - DOUBLE
            case OPCode::ADD_DOUBLE: {
                Value right = popValue();
                Value left = popValue();
                Value result = createDouble(asDouble(left) + asDouble(right));
                pushValue(result);
                break;
            }
            case OPCode::SUB_DOUBLE: {
                Value right = popValue();
                Value left = popValue();
                Value result = createDouble(asDouble(left) - asDouble(right));
                pushValue(result);
                break;
            }
            case OPCode::MULT_DOUBLE: {
                Value right = popValue();
                Value left = popValue();
                Value result = createDouble(asDouble(left) * asDouble(right));
                pushValue(result);
                break;
            }
            case OPCode::DIV_DOUBLE: {
                Value right = popValue();
                Value left = popValue();
                Value result = createDouble(asDouble(left) / asDouble(right));
                pushValue(result);
                break;
            }

            // String operations
            case OPCode::ADD_STRING: {
                Value right = popValue();
                Value left = popValue();
                StringObject* leftStr = asString(left);
                StringObject* rightStr = asString(right);
                std::string leftString(leftStr->chars, leftStr->length);
                std::string rightString(rightStr->chars, rightStr->length);
                std::string concatenated = leftString + rightString;
                StringObject* resultStr = heap.allocateString(concatenated);
                Value result = createObject(resultStr);
                pushValue(result);
                break;
            }

            // Comparison operations - INT
            case OPCode::EQ_INT: {
                Value right = popValue();
                Value left = popValue();
                Value result = createBool(asInt(left) == asInt(right));
                pushValue(result);
                break;
            }
            case OPCode::LT_INT: {
                Value right = popValue();
                Value left = popValue();
                Value result = createBool(asInt(left) < asInt(right));
                pushValue(result);
                break;
            }
            case OPCode::GT_INT: {
                Value right = popValue();
                Value left = popValue();
                Value result = createBool(asInt(left) > asInt(right));
                pushValue(result);
                break;
            }
            case OPCode::LEQ_INT: {
                Value right = popValue();
                Value left = popValue();
                Value result = createBool(asInt(left) <= asInt(right));
                pushValue(result);
                break;
            }
            case OPCode::GEQ_INT: {
                Value right = popValue();
                Value left = popValue();
                Value result = createBool(asInt(left) >= asInt(right));
                pushValue(result);
                break;
            }

            // Comparison operations - DOUBLE
            case OPCode::EQ_DOUBLE: {
                Value right = popValue();
                Value left = popValue();
                Value result = createBool(asDouble(left) == asDouble(right));
                pushValue(result);
                break;
            }
            case OPCode::LT_DOUBLE: {
                Value right = popValue();
                Value left = popValue();
                Value result = createBool(asDouble(left) < asDouble(right));
                pushValue(result);
                break;
            }
            case OPCode::GT_DOUBLE: {
                Value right = popValue();
                Value left = popValue();
                Value result = createBool(asDouble(left) > asDouble(right));
                pushValue(result);
                break;
            }
            case OPCode::LEQ_DOUBLE: {
                Value right = popValue();
                Value left = popValue();
                Value result = createBool(asDouble(left) <= asDouble(right));
                pushValue(result);
                break;
            }
            case OPCode::GEQ_DOUBLE: {
                Value right = popValue();
                Value left = popValue();
                Value result = createBool(asDouble(left) >= asDouble(right));
                pushValue(result);
                break;
            }

            // Comparison operations - BOOL and STRING
            case OPCode::EQ_BOOL: {
                Value right = popValue();
                Value left = popValue();
                Value result = createBool(asBool(left) == asBool(right));
                pushValue(result);
                break;
            }
            case OPCode::EQ_STRING: {
                Value right = popValue();
                Value left = popValue();
                StringObject* leftStr = asString(left);
                StringObject* rightStr = asString(right);
                std::string leftString(leftStr->chars, leftStr->length);
                std::string rightString(rightStr->chars, rightStr->length);
                Value result = createBool(leftString == rightString);
                pushValue(result);
                break;
            }

            // Local variable operations
            case OPCode::STORE_LOCAL: {
                Value value = popValue();
                size_t slot = static_cast<size_t>(inst.operand);
                if (slot >= locals.size()) {
                    locals.resize(slot + 1);
                }
                locals[slot] = value;
                break;
            }
            case OPCode::LOAD_LOCAL: {
                size_t slot = static_cast<size_t>(inst.operand);
                if (slot >= locals.size()) {
                    throw RuntimeException("Invalid local variable slot: " + std::to_string(slot));
                }
                Value value = locals[slot];
                pushValue(value);
                break;
            }
            case OPCode::STORE_GLOBAL: {
                Value value = popValue();
                size_t slot = static_cast<size_t>(inst.operand);
                if (slot >= globals.size()) {
                    globals.resize(slot + 1);
                }
                globals[slot] = value;
                break;
            }
            case OPCode::LOAD_GLOBAL: {
                size_t slot = static_cast<size_t>(inst.operand);
                if (slot >= globals.size()) {
                    throw RuntimeException("Invalid global variable slot: " + std::to_string(slot));
                }
                Value value = globals[slot];
                pushValue(value);
                break;
            }
            
            // Control flow operations
            case OPCode::JUMP: {
                ip = static_cast<size_t>(inst.operand);
                continue; // Skip ip++ at end of loop
            }
            case OPCode::JUMP_IF_FALSE: {
                Value condition = popValue();
                if (!asBool(condition)) {
                    ip = static_cast<size_t>(inst.operand);
                    continue; // Skip ip++ at end of loop
                }
                break;
            }

            // Function call operations
            case OPCode::CALL: {
                // Validate operand bounds
                if (inst.operand < 0 || static_cast<size_t>(inst.operand) >= constants.size()) {
                    throw RuntimeException("Invalid function constant index: " + std::to_string(inst.operand));
                }

                Value functionValue = constants[inst.operand];
                FunctionObject* function = asFunction(functionValue);

                // Validate function pointer
                if (function == nullptr) {
                    throw RuntimeException("Invalid function object - null pointer");
                }

                // CRITICAL: Validate parameter count before proceeding
                if (stack.size() < static_cast<size_t>(function->parameterCount)) {
                    throw RuntimeException("Not enough arguments for function call. Expected: " +
                                         std::to_string(function->parameterCount) +
                                         ", Got: " + std::to_string(stack.size()));
                }

                // Create new call frame
                CallFrame frame;
                frame.instructions = instructions;
                frame.constants = constants;
                frame.locals = locals;
                frame.returnAddress = ip + 1;
                frame.stackBase = stack.size() - function->parameterCount; // Now safe

                // Push current frame onto call stack
                callStack.push(frame);

                // Set up new execution context
                instructions = function->instructions;
                constants = function->constants;
                locals.clear();
                locals.resize(function->parameterCount);

                // Move parameters from stack to locals
                for (int i = function->parameterCount - 1; i >= 0; i--) {
                    locals[i] = popValue();
                }

                // Start executing function
                ip = 0;
                continue; // Skip ip++ at end of loop
            }

            case OPCode::RETURN: {
                if (callStack.empty()) {
                    // Top-level return, halt execution
                    return;
                }

                // Get return value from stack (if any)
                Value returnValue;
                bool hasReturnValue = !stack.empty();
                if (hasReturnValue) {
                    returnValue = popValue();
                }

                // Restore previous frame
                CallFrame frame = callStack.top();
                callStack.pop();

                instructions = frame.instructions;
                constants = frame.constants;
                locals = frame.locals;
                ip = frame.returnAddress;

                // Restore stack to frame base and push return value
                stack.resize(frame.stackBase);
                if (hasReturnValue) {
                    pushValue(returnValue);
                }

                continue; // Skip ip++ at end of loop
            }

            // Type conversion operations
            case OPCode::INT_TO_DOUBLE: {
                Value intValue = popValue();
                Value doubleValue = createDouble(static_cast<double>(asInt(intValue)));
                pushValue(doubleValue);
                break;
            }
            case OPCode::FLOAT_TO_DOUBLE: {
                Value floatValue = popValue();
                Value doubleValue = createDouble(static_cast<double>(asFloat(floatValue)));
                pushValue(doubleValue);
                break;
            }

            case OPCode::HALT:
                return;
            default:
                throw RuntimeException("Unimplemented opcode: " + std::string(OpCodeToString(inst.opcode)));
        }

        ip++;
    }
}


void VirtualMachine::dumpStack() {
    for (auto& v : stack) {
        std::cout << valueToString(v) << "\n";
    }
}

const char* OpCodeToString(OPCode op) {
    switch (op) {
        case OPCode::LOAD_INT: return "LOAD_INT";
        case OPCode::LOAD_FLOAT: return "LOAD_FLOAT";
        case OPCode::LOAD_DOUBLE: return "LOAD_DOUBLE";
        case OPCode::LOAD_STRING: return "LOAD_STRING";
        case OPCode::LOAD_BOOL: return "LOAD_BOOL";
        case OPCode::STORE_LOCAL: return "STORE_LOCAL";
        case OPCode::LOAD_LOCAL: return "LOAD_LOCAL";
        case OPCode::STORE_GLOBAL: return "STORE_GLOBAL";
        case OPCode::LOAD_GLOBAL: return "LOAD_GLOBAL";

        case OPCode::ADD_INT: return "ADD_INT";
        case OPCode::SUB_INT: return "SUB_INT";
        case OPCode::MULT_INT: return "MULT_INT";
        case OPCode::DIV_INT: return "DIV_INT";

        case OPCode::ADD_FLOAT: return "ADD_FLOAT";
        case OPCode::SUB_FLOAT: return "SUB_FLOAT";
        case OPCode::MULT_FLOAT: return "MULT_FLOAT";
        case OPCode::DIV_FLOAT: return "DIV_FLOAT";

        case OPCode::ADD_DOUBLE: return "ADD_DOUBLE";
        case OPCode::SUB_DOUBLE: return "SUB_DOUBLE";
        case OPCode::MULT_DOUBLE: return "MULT_DOUBLE";
        case OPCode::DIV_DOUBLE: return "DIV_DOUBLE";
        
        case OPCode::ADD_STRING: return "ADD_STRING";
        case OPCode::INT_TO_DOUBLE: return "INT_TO_DOUBLE";
        case OPCode::FLOAT_TO_DOUBLE: return "FLOAT_TO_DOUBLE";

        case OPCode::EQ_INT: return "EQ_INT";
        case OPCode::LT_INT: return "LT_INT";
        case OPCode::GT_INT: return "GT_INT";
        case OPCode::GEQ_INT: return "GEQ_INT";
        case OPCode::LEQ_INT: return "LEQ_INT";
        case OPCode::EQ_DOUBLE: return "EQ_DOUBLE";
        case OPCode::LT_DOUBLE: return "LT_DOUBLE";
        case OPCode::GT_DOUBLE: return "GT_DOUBLE";
        case OPCode::LEQ_DOUBLE: return "LEQ_DOUBLE";
        case OPCode::GEQ_DOUBLE: return "GEQ_DOUBLE";
        case OPCode::EQ_BOOL: return "EQ_BOOL";
        case OPCode::EQ_STRING: return "EQ_STRING";

        case OPCode::JUMP: return "JUMP";
        case OPCode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";

        case OPCode::CALL: return "CALL";
        case OPCode::RETURN: return "RETURN";

        case OPCode::HALT: return "HALT";
        default: return "UNKNOWN";
    }
}

void VirtualMachine::printInstructions() {
    std::cout << "Instruction printing temporarily disabled\n";    
}
