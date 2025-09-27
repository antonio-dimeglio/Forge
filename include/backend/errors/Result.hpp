#pragma once
#include <memory>

namespace forge::errors {
    template<typename T, typename E>
    class Result {
        private:
            union {
                T value_;
                E error_;
            };
            bool isOk_;

            Result(T value, bool) : value_(std::move(value)), isOk_(true) {}
            Result(E error, bool) : error_(std::move(error)), isOk_(false) {}
        
        public:
            static Result Ok(T value) { return (Result(std::move(value), true)); }
            static Result Err(E error) { return (Result(std::move(error), false)); }
            

            bool isOk() const { return isOk_; }
            bool isErr() const { return !isOk_; }

            T& unwrap() {
                    if (!isOk_) {
                        throw std::runtime_error("Called unwrap() on error Result");
                    }
                    return value_;
                }

            const T& unwrap() const {
                if (!isOk_) {
                    throw std::runtime_error("Called unwrap() on error Result");
                }
                return value_;
            }

            T unwrapOr(T defaultValue) {
                return isOk_ ? std::move(value_) : std::move(defaultValue);
            }

            E& error() {
                if (isOk_) {
                    throw std::runtime_error("Called error() on ok Result");
                }
                return error_;
            }

            const E& error() const {
                if (isOk_) {
                    throw std::runtime_error("Called error() on ok Result");
                }
                return error_;
            }

            template<typename F>
            auto map(F func) -> Result<decltype(func(std::declval<T>())), E> {
                using ReturnType = decltype(func(std::declval<T>()));
                if (isOk_) {
                    return Result<ReturnType, E>::Ok(func(value_));
                } else {
                    return Result<ReturnType, E>::Err(error_);
                }
            }

            template<typename F>
            auto andThen(F func) -> decltype(func(std::declval<T>())) {
                if (isOk_) {
                    return func(value_);
                } else {
                    using ReturnType = decltype(func(std::declval<T>()));
                    return ReturnType::Err(error_);
                }
            }
            
            ~Result() {
                if (isOk_) {
                    value_.~T();
                } else {
                    error_.~E();
                }
            }

            // Move constructor and assignment
            Result(Result&& other) noexcept : isOk_(other.isOk_) {
                if (isOk_) {
                    new(&value_) T(std::move(other.value_));
                } else {
                    new(&error_) E(std::move(other.error_));
                }
            }

            Result& operator=(Result&& other) noexcept {
                if (this != &other) {
                    this->~Result();
                    isOk_ = other.isOk_;
                    if (isOk_) {
                        new(&value_) T(std::move(other.value_));
                    } else {
                        new(&error_) E(std::move(other.error_));
                    }
                }
                return *this;
            }

            // Delete copy operations
            Result(const Result&) = delete;
            Result& operator=(const Result&) = delete;
    };
}