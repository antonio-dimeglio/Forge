#include "../../../include/backends/vm/VirtualMachine.hpp"
#include "../../../include/backends/vm/RuntimeException.hpp"
#include <iostream>

size_t VirtualMachine::internString(const std::string& str) {
    auto it = stringCache.find(str);
    if (it != stringCache.end()) {
        return it->second;
    }

    size_t id = stringTable.size();
    stringTable.push_back(str);
    stringCache[str] = id;
    return id;
}

std::string VirtualMachine::getString(size_t id) {
    if (id >= stringTable.size()) {
        throw RuntimeException("Invalid string ID: " + std::to_string(id));
    }

    return stringTable[id];
}

void VirtualMachine::pushInt(int value) {
    TypedValue tv;
    tv.type = TypedValue::INT;
    tv.value.i = value;
    stack.push_back(tv);
}

void VirtualMachine::pushFloat(float value) {
    TypedValue tv;
    tv.type = TypedValue::FLOAT;
    tv.value.f = value;
    stack.push_back(tv);
}

void VirtualMachine::pushDouble(double value) {
    TypedValue tv;
    tv.type = TypedValue::DOUBLE;
    tv.value.d = value;
    stack.push_back(tv);
}

void VirtualMachine::pushStringId(size_t stringId) {
    TypedValue tv;
    tv.type = TypedValue::STRING;
    tv.value.string_id = stringId;
    stack.push_back(tv);
}

void VirtualMachine::pushBool(bool value) {
    TypedValue tv;
    tv.type = TypedValue::BOOL;
    tv.value.b = value;
    stack.push_back(tv);
}

int VirtualMachine::popInt() {
    if (stack.empty()) {
        throw RuntimeException("Stack underflow");
    }
    TypedValue tv = stack.back();
    if (tv.type != TypedValue::INT) {
        throw RuntimeException("Expected INT on stack, got " + std::string(TypeToString(tv.type)));
    }
    stack.pop_back();
    return tv.value.i;
}

float VirtualMachine::popFloat() {
    if (stack.empty()) {
        throw RuntimeException("Stack underflow");
    }
    TypedValue tv = stack.back();
    if (tv.type != TypedValue::FLOAT) {
        throw RuntimeException("Expected FLOAT on stack, got " + std::string(TypeToString(tv.type)));
    }
    stack.pop_back();
    return tv.value.f;
}


double VirtualMachine::popDouble() {
    if (stack.empty()) {
        throw RuntimeException("Stack underflow");
    }
    TypedValue tv = stack.back();
    if (tv.type != TypedValue::DOUBLE) {
        throw RuntimeException("Expected DOUBLE on stack, got " + std::string(TypeToString(tv.type)));
    }
    stack.pop_back();
    return tv.value.d;
}

size_t VirtualMachine::popStringId() {
    if (stack.empty()) {
        throw RuntimeException("Stack underflow");
    }
    TypedValue tv = stack.back();
    if (tv.type != TypedValue::STRING) {
        throw RuntimeException("Expected STRING on stack, got " + std::string(TypeToString(tv.type)));
    }
    stack.pop_back();
    return tv.value.string_id;
}

bool VirtualMachine::popBool() {
    if (stack.empty()) {
        throw RuntimeException("Stack underflow");
    }
    TypedValue tv = stack.back();
    if (tv.type != TypedValue::BOOL) {
        throw RuntimeException("Expected BOOL on stack, got " + std::string(TypeToString(tv.type)));
    }
    stack.pop_back();
    return tv.value.b;
}

void VirtualMachine::run() {
    while (ip < instructions.size()) {
        Instruction inst = instructions[ip];

        switch (inst.opcode) {
            case OPCode::LOAD_INT:
                pushInt(constants[inst.operand].value.i);
                break;
            case OPCode::LOAD_FLOAT:
                pushFloat(constants[inst.operand].value.f);
                break;
            case OPCode::LOAD_DOUBLE:
                pushDouble(constants[inst.operand].value.d);
                break;
            case OPCode::LOAD_STRING:
                pushStringId(constants[inst.operand].value.string_id);
                break;
            case OPCode::LOAD_BOOL:
                pushBool(constants[inst.operand].value.b);
                break;
            case OPCode::STORE_LOCAL:
                if (inst.operand >= (int)locals.size()) {
                    locals.resize(inst.operand + 1);
                }
                locals[inst.operand] = stack.back();
                stack.pop_back();
                break;
            case OPCode::LOAD_LOCAL: {
                    if (inst.operand >= (int)locals.size()) {
                        throw RuntimeException("Invalid local variable slot");
                    }
                    stack.push_back(locals[inst.operand]);
                }
                break;

            case OPCode::ADD_INT: {
                int b = popInt();
                int a = popInt();
                pushInt(a + b);
                break;
            }
            case OPCode::SUB_INT: {
                int b = popInt();
                int a = popInt();
                pushInt(a - b);
                break;
            }
            case OPCode::MULT_INT: {
                int b = popInt();
                int a = popInt();
                pushInt(a * b);
                break;
            }
            case OPCode::DIV_INT: {
                int b = popInt();
                int a = popInt();
                pushInt(a / b);
                break;
            }

            case OPCode::ADD_FLOAT: {
                float b = popFloat();
                float a = popFloat();
                pushFloat(a + b);
                break;
            }
            case OPCode::SUB_FLOAT: {
                float b = popFloat();
                float a = popFloat();
                pushFloat(a - b);
                break;
            }
            case OPCode::MULT_FLOAT: {
                float b = popFloat();
                float a = popFloat();
                pushFloat(a * b);
                break;
            }
            case OPCode::DIV_FLOAT: {
                float b = popFloat();
                float a = popFloat();
                pushFloat(a / b);
                break;
            }

            case OPCode::ADD_DOUBLE: {
                double b = popDouble();
                double a = popDouble();
                pushDouble(a + b);
                break;
            }
            case OPCode::SUB_DOUBLE: {
                double b = popDouble();
                double a = popDouble();
                pushDouble(a - b);
                break;
            }
            case OPCode::MULT_DOUBLE: {
                double b = popDouble();
                double a = popDouble();
                pushDouble(a * b);
                break;
            }
            case OPCode::DIV_DOUBLE: {
                double b = popDouble();
                double a = popDouble();
                pushDouble(a / b);
                break;
            }
            case OPCode::BITWISE_OR_INT: {
                int b = popInt();
                int a = popInt();
                pushInt(a | b);
                break;
            }
            case OPCode::BITWISE_OR_BOOL: {
                bool b = popBool();
                bool a = popBool();
                pushBool(a | b);
                break;
            }
            case OPCode::BITWISE_AND_INT: {
                int b = popInt();
                int a = popInt();
                pushInt(a & b);
                break;
            }
            case OPCode::BITWISE_AND_BOOL: {
                bool b = popBool();
                bool a = popBool();
                pushBool(a & b);
                break;
            }
            case OPCode::BITWISE_XOR_INT: {
                int b = popInt();
                int a = popInt();
                pushInt(a ^ b);
                break;
            }
            case OPCode::BITWISE_XOR_BOOL: {
                bool b = popBool();
                bool a = popBool();
                pushBool(a ^ b);
                break;
            }

            case OPCode::INT_TO_DOUBLE: {
                int a = popInt();
                pushDouble(static_cast<double>(a));
                break;
            }
            case OPCode::FLOAT_TO_DOUBLE: {
                float a = popFloat();
                pushDouble(static_cast<double>(a));
                break;
            }

            case OPCode::EQ_INT: {
                int b = popInt();
                int a = popInt();
                pushBool(a == b);
                break;
            }
            case OPCode::LT_INT: {
                int b = popInt();
                int a = popInt();
                pushBool(a < b);
                break;
            }
            case OPCode::GT_INT: {
                int b = popInt();
                int a = popInt();
                pushBool(a > b);
                break;
            }

            case OPCode::EQ_DOUBLE: {
                double b = popDouble();
                double a = popDouble();
                pushBool(a == b);
                break;
            }
            case OPCode::LT_DOUBLE: {
                double b = popDouble();
                double a = popDouble();
                pushBool(a < b);
                break;
            }
            case OPCode::GT_DOUBLE: {
                double b = popDouble();
                double a = popDouble();
                pushBool(a > b);
                break;
            }

            // Unary operations
            case OPCode::NEG_INT: {
                int a = popInt();
                pushInt(-a);
                break;
            }
            case OPCode::NEG_FLOAT: {
                float a = popFloat();
                pushFloat(-a);
                break;
            }
            case OPCode::NEG_DOUBLE: {
                double a = popDouble();
                pushDouble(-a);
                break;
            }
            case OPCode::NOT_BOOL: {
                bool a = popBool();
                pushBool(!a);
                break;
            }
            case OPCode::JUMP: {
                ip = inst.operand - 1;
                break;
            }
            case OPCode::JUMP_IF_FALSE: {
                bool a = popBool();
                if (!a) {
                    ip = inst.operand - 1;
                }
                break;
            }
            case OPCode::HALT:
                return;

            default:
                throw RuntimeException("Unknown opcode: " +
std::to_string(static_cast<int>(inst.opcode)));
        }
        ip++;
    }
}

void VirtualMachine::loadProgram(const std::vector<Instruction>& instructions,
                                const std::vector<TypedValue>& constants) {
    loadProgram(instructions, constants, {});
}

void VirtualMachine::loadProgram(const std::vector<Instruction>& instructions,
                                const std::vector<TypedValue>& constants,
                                const std::vector<std::string>& strings) {
    this->instructions = instructions;
    this->constants = constants;
    ip = 0;
    stack.clear();

    // Only clear string table if we have strings to intern (new BytecodeCompiler path)
    // Legacy tests pre-intern strings and expect them to remain
    if (!strings.empty()) {
        stringTable.clear();
        stringCache.clear();

        // Intern all strings from the compiler
        for (const auto& str : strings) {
            internString(str);
        }
    }
}

void VirtualMachine::dumpStack() {
    std::cout << "Stack (top to bottom):\n";
    if (stack.empty()) {
        std::cout << "  [empty]\n";
        return;
    }

    for (int i = stack.size() - 1; i >= 0; i--) {
        std::cout << "  [" << i << "] " << valueToString(stack[i]) << "\n";
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

        case OPCode::INT_TO_DOUBLE: return "INT_TO_DOUBLE";
        case OPCode::FLOAT_TO_DOUBLE: return "FLOAT_TO_DOUBLE";

        case OPCode::EQ_INT: return "EQ_INT";
        case OPCode::LT_INT: return "LT_INT";
        case OPCode::GT_INT: return "GT_INT";
        case OPCode::EQ_DOUBLE: return "EQ_DOUBLE";
        case OPCode::LT_DOUBLE: return "LT_DOUBLE";
        case OPCode::GT_DOUBLE: return "GT_DOUBLE";

        case OPCode::JUMP: return "JUMP";
        case OPCode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";

        case OPCode::HALT: return "HALT";
        default: return "UNKNOWN";
    }
}

void VirtualMachine::printInstructions() {
    std::cout << "Instructions:\n";
    for (size_t i = 0; i < instructions.size(); i++) {
        std::cout << "  [" << i << "] " << OpCodeToString(instructions[i].opcode)
                << " " << instructions[i].operand << "\n";
    }
}

std::string VirtualMachine::valueToString(TypedValue tv) {
    std::string result = "Type: " + std::string(TypeToString(tv.type)) + " Value: ";

    switch (tv.type) {
        case TypedValue::INT:
            return result + std::to_string(tv.value.i);
        case TypedValue::FLOAT:
            return result + std::to_string(tv.value.f);
        case TypedValue::DOUBLE:
            return result + std::to_string(tv.value.d);
        case TypedValue::STRING:
            return result + "\"" + getString(tv.value.string_id) + "\"";
        case TypedValue::BOOL:
            return result + (tv.value.b ? "true" : "false");
        default:
            return result + "unknown";
    }
}

TypedValue VirtualMachine::getLocal(size_t idx) {
    if (idx < locals.size()) {
        return locals[idx];
    }

    throw RuntimeException("Tried to get value from locals at invalid index.");
}

void VirtualMachine::reset() {
    // Clear all VM state
    stack.clear();
    constants.clear();
    instructions.clear();
    stringTable.clear();
    stringCache.clear();
    locals.clear();
    ip = 0;
}