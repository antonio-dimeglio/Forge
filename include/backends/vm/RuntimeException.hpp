#include <string>
#include <exception>

class RuntimeException : public std::exception {
private:
    std::string message;

public:
    RuntimeException(const std::string& msg) : message(msg) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};