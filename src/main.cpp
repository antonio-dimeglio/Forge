#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <memory>

#include "../include/CLI11.hpp"
#include "../include/llvm/LLVMCompiler.hpp"
#include "lexer/Tokenizer.hpp"
#include "parser/Parser.hpp"

struct CompilerOptions {
    std::string input_file;
    std::string output_file;

    // Compilation stages
    bool lexer_only = false;
    bool parser_only = false;
    bool compile_only = false;

    // Debug options
    bool verbose = false;
    bool dump_tokens = false;
    bool dump_ast = false;
    bool dump_llvm = false;

    // Other options
    bool optimize = false;
    bool interactive = false;
};

class ForgeCompiler {
public:
    ForgeCompiler(const CompilerOptions& opts) : options(opts) {}

    int compile() {
        if (options.interactive) {
            return runInteractiveMode();
        }

        if (options.input_file.empty()) {
            std::cerr << "Error: No input file specified\n";
            return 1;
        }

        return compileFile();
    }

private:
    const CompilerOptions& options;

    int runInteractiveMode() {
        std::cout << "Forge Interactive REPL (Not yet implemented)\n";
        std::cout << "Type 'exit' to quit\n";
        return 0;
    }

    int compileFile() {
        if (options.verbose) {
            printHeader();
        }

        // Read source file
        std::string source = readFile(options.input_file);

        // Lexical analysis
        auto tokens = runLexer(source);
        if (options.lexer_only) return 0;

        // Parsing
        auto ast = runParser(tokens);
        if (options.parser_only) return 0;

        auto compiler = LLVMCompiler();
        auto module = compiler.compile(*ast);

        if (options.dump_llvm) {
            std::cout << "\n=== LLVM IR ===\n";
            module->print(llvm::outs(), nullptr);
            std::cout << "\n";
        }

        if (options.verbose) {
            
        }

        return 0;
    }

    void printHeader() {
        std::cout << "Forge Programming Language Compiler v0.1.0\n";
        std::cout << "===========================================\n";
        std::cout << "Input: " << options.input_file << "\n\n";
    }

    std::string readFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        std::string content;
        std::getline(file, content, '\0');
        return content;
    }

    std::vector<Token> runLexer(const std::string& source) {
        if (options.verbose) {
            std::cout << "Running lexical analysis...\n";
        }

        Tokenizer tokenizer(source);
        auto tokens = tokenizer.tokenize();

        if (options.dump_tokens) {
            std::cout << "\n=== TOKENS ===\n";
            for (const auto& token : tokens) {
                std::cout << token.toString() << "\n";
            }
            std::cout << "\n";
        }

        return tokens;
    }

    std::unique_ptr<Statement> runParser(const std::vector<Token>& tokens) {
        if (options.verbose) {
            std::cout << "Running parser...\n";
        }

        Parser parser(tokens);
        auto ast = parser.parseProgram();

        if (options.dump_ast) {
            std::cout << "\n=== AST ===\n";
            std::cout << ast->toString() << "\n\n";
        }

        return ast;
    }
};

CompilerOptions parseCommandLine(int argc, char** argv) {
    CLI::App app{"Forge Programming Language Compiler", "forge"};
    app.set_version_flag("--version,-v", "0.1.0 (Development)");

    CompilerOptions options;

    // Input/Output
    app.add_option("file", options.input_file, "Input file to compile")
        ->check(CLI::ExistingFile);
    app.add_option("-o,--output", options.output_file, "Output file name");

    // Compilation stages
    app.add_flag("--lex", options.lexer_only, "Stop after lexical analysis");
    app.add_flag("--parse", options.parser_only, "Stop after parsing");
    app.add_flag("--compile", options.compile_only, "Stop after compilation");

    // Debug options
    app.add_flag("--verbose", options.verbose, "Enable verbose output");
    app.add_flag("--dump-tokens", options.dump_tokens, "Dump lexer tokens");
    app.add_flag("--dump-ast", options.dump_ast, "Dump parser AST");
    app.add_flag("--dump-llvm", options.dump_llvm, "Dump LLVM IR");

    // Other options
    app.add_flag("--optimize,-O", options.optimize, "Enable optimizations");
    app.add_flag("--interactive,-i", options.interactive, "Interactive REPL mode");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& error) {
        std::exit(app.exit(error));
    }

    return options;
}

int main(int argc, char** argv) {
    try {
        auto options = parseCommandLine(argc, argv);
        ForgeCompiler compiler(options);
        int result = compiler.compile();
        return result; 
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return 1;
    }
}