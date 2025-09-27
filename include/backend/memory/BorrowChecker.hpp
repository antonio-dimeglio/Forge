#pragma once
#include "MemoryModel.hpp"
#include "../errors/Result.hpp"

using namespace forge::errors;

namespace forge::memory {

    class BorrowChecker {
        public:
            explicit BorrowChecker(MemoryModel& memoryModel, errors::ErrorReporter& errorReporter);

            // Main Analysis Interface
            Result<void, errors::BorrowError>
            analyzeProgram(const ast::Program& program);

            Result<void, errors::BorrowError>
            analyzeStatement(const ast::Statement& stmt);

            Result<void, errors::BorrowError>
            analyzeExpression(const ast::Expression& expr);

        private:
            MemoryModel& memoryModel_;
            errors::ErrorReporter& errorReporter_;

            // Core Borrow Checking Rules
            Result<void, errors::BorrowError>
            checkBorrowRules(VariableId target, bool isMutable, SourceLocation location);

            Result<void, errors::BorrowError>
            checkMoveRules(VariableId source, SourceLocation location);

            Result<void, errors::BorrowError>
            checkLifetimeRules(const ast::Expression& expr);

            // Statement-specific analysis
            Result<void, errors::BorrowError>
            analyzeVariableDeclaration(const ast::VariableDeclaration& decl);

            Result<void, errors::BorrowError>
            analyzeAssignment(const ast::Assignment& assignment);

            Result<void, errors::BorrowError>
            analyzeFunctionCall(const ast::FunctionCall& call);

            // Expression-specific analysis
            Result<void, errors::BorrowError>
            analyzeAddressOf(const ast::UnaryExpression& expr);

            Result<void, errors::BorrowError>
            analyzeDereference(const ast::UnaryExpression& expr);

            Result<void, errors::BorrowError>
            analyzeMove(const ast::MoveExpression& expr);
    };

}