#include "../../../include/backend/codegen/SymbolTable.hpp"
using namespace forge::errors;

namespace forge::codegen {

void SymbolTable::Scope::declare(std::string name, Symbol symbol) {
    // Empty implementation
}

SymbolTable::Symbol* SymbolTable::Scope::lookup(const std::string& name) {
    return nullptr;
}

const SymbolTable::Symbol* SymbolTable::Scope::lookup(const std::string& name) const {
    return nullptr;
}

std::vector<SymbolTable::Symbol*> SymbolTable::Scope::getAllSymbols() {
    return {};
}

void SymbolTable::enterScope() {
    // Empty implementation
}

void SymbolTable::exitScope() {
    // Empty implementation
}

size_t SymbolTable::getCurrentScopeDepth() const {
    return 0;
}

forge::errors::Result<void, forge::errors::TypeError> SymbolTable::declare(std::string name, Symbol symbol) {
    // Empty implementation
    return Result<void, forge::errors::TypeError>::Ok();
}

std::optional<SymbolTable::Symbol*> SymbolTable::lookup(const std::string& name) const {
    return std::nullopt;
}

std::vector<SymbolTable::Symbol*> SymbolTable::getSymbolsInCurrentScope() {
    return {};
}

std::vector<SymbolTable::Symbol*> SymbolTable::getAllVisibleSymbols() {
    return {};
}

SymbolTable::Scope& SymbolTable::getCurrentScope() {
    static Scope dummy;
    return dummy;
}

const SymbolTable::Scope& SymbolTable::getCurrentScope() const {
    static Scope dummy;
    return dummy;
}

} // namespace forge::codegen