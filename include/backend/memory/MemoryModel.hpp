#pragma once

#include "../../ast/Statement.hpp"
#include "../../ast/Expression.hpp"
#include "../errors/ErrorReporter.hpp"
#include "../errors/ErrorTypes.hpp"
#include "../errors/Result.hpp"
#include "../types/Type.hpp"
#include <unordered_map>
#include <vector>
#include <memory>

// Forward declarations
namespace forge::memory {
    class LifetimeAnalyzer;
}



namespace forge::memory {

    // Type aliases for clarity
    using VariableId = size_t;

    // Forward declare BorrowInfo for use in MemoryModel
    struct BorrowInfo;

    class MemoryModel {
        public:
            enum class Ownership {
                Owned,          // Variable owns the value exclusively
                Borrowed,       // Temporary immutable access
                MutBorrowed,    // Temporary mutable access
                Moved           // Value has been transferred
            };

            enum class Lifetime {
                Static,         // 'static lifetime
                Function,       // Lives for function duration
                Block,          // Lives for block duration
                Expression      // Lives for expression duration
            };

            explicit MemoryModel(forge::errors::ErrorReporter& errorReporter);

            // Ownership Tracking
            forge::errors::Result<void, forge::errors::BorrowError>
            registerVariable(VariableId id, const forge::types::Type& type, Lifetime lifetime);

            forge::errors::Result<void, forge::errors::BorrowError>
            registerBorrow(VariableId target, bool isMutable, forge::errors::SourceLocation location);

            forge::errors::Result<void, forge::errors::BorrowError>
            registerMove(VariableId source, forge::errors::SourceLocation location);

            forge::errors::Result<void, forge::errors::BorrowError>
            endBorrow(VariableId target, forge::errors::SourceLocation location);

            // Lifetime Analysis
            forge::errors::Result<Lifetime, forge::errors::BorrowError>
            computeLifetime(const forge::ast::Expression& expr);

            forge::errors::Result<void, forge::errors::BorrowError>
            validateLifetimes(const forge::ast::Statement& stmt);

            // Query Interface
            Ownership getOwnership(VariableId id) const;
            Lifetime getLifetime(VariableId id) const;
            bool hasActiveBorrows(VariableId id) const;
            std::vector<BorrowInfo> getActiveBorrows(VariableId id) const;

        private:
            forge::errors::ErrorReporter& errorReporter_;

            struct VariableInfo {
                std::unique_ptr<forge::types::Type> type;  // Use pointer to abstract type
                Ownership ownership;
                Lifetime lifetime;
                std::vector<BorrowInfo> activeBorrows;
                forge::errors::SourceLocation declaration;
            };

            std::unordered_map<VariableId, VariableInfo> variables_;
            std::unique_ptr<LifetimeAnalyzer> lifetimeAnalyzer_;
    };

    struct BorrowInfo {
        VariableId borrower;
        bool isMutable;
        forge::errors::SourceLocation location;
        MemoryModel::Lifetime expectedLifetime;
    };

}