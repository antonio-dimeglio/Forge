#pragma once 
#include "OPCode.hpp"
#include "Instruction.hpp"
#include <vector>
#include <memory>
#include <unordered_map>
#include "../../parser/Expression.hpp"

class VirtualMachine {
    private:
        std::vector<TypedValue> stack;
        std::vector<TypedValue> constants;
        std::vector<Instruction> instructions;
        std::vector<std::string> stringTable; 
        std::unordered_map<std::string, size_t> stringCache; 
        size_t ip = 0;
        std::vector<TypedValue> locals;
        
        void pushInt(int value);
        void pushFloat(float value);
        void pushDouble(double value);
        void pushStringId(size_t stringId);
        void pushBool(bool value);

        int peekInt();
        float peekFloat();
        double peekDouble();
        std::string peekString();
        bool peekBool(bool value);


        void runtimeError(const std::string& message);

    public:
        void loadProgram(const std::vector<Instruction>& instructions,
                       const std::vector<TypedValue>& constants);
        void run();
        void dumpStack();
        void printInstructions();
        size_t internString(const std::string& str);    
        std::string getString(size_t id);               
        std::string valueToString(TypedValue tv);

        int popInt();
        float popFloat();
        double popDouble();
        size_t popStringId();
        bool popBool();

        TypedValue getLocal(size_t idx);
        void reset();
};