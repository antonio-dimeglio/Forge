#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include "../include/CLI11.hpp"
#include "lexer/Tokenizer.hpp"
#include "parser/Parser.hpp"
#include "backends/vm/BytecodeCompiler.hpp"
#include "backends/vm/OPCode.hpp"
#include "backends/vm/VirtualMachine.hpp"

int main(int argc, char** argv) {
    CLI::App app{"Forge Programming Language Compiler", "forge"};
    app.set_version_flag("--version,-v", "0.1.0 (Development)");

    // Input file
    std::string input_file;
    app.add_option("file", input_file, "Input file to compile")
        ->check(CLI::ExistingFile);

    // Output options
    std::string output_file;
    app.add_option("-o,--output", output_file, "Output file name");

    // Compilation stages
    bool lexer_only = false;
    bool parser_only = false;
    bool compiler_only = false;
    bool no_execute = false;

    app.add_flag("--lex", lexer_only, "Stop after lexical analysis");
    app.add_flag("--parse", parser_only, "Stop after parsing (AST generation)");
    app.add_flag("--compile", compiler_only, "Stop after compilation (bytecode generation)");
    app.add_flag("--no-run", no_execute, "Don't execute the program");

    // Debug options
    bool verbose = false;
    bool dump_tokens = false;
    bool dump_ast = false;
    bool dump_bytecode = false;
    bool dump_vm_state = false;

    app.add_flag("--verbose", verbose, "Enable verbose output");
    app.add_flag("--dump-tokens", dump_tokens, "Dump lexer tokens");
    app.add_flag("--dump-ast", dump_ast, "Dump parser AST");
    app.add_flag("--dump-bytecode", dump_bytecode, "Dump compiler bytecode");
    app.add_flag("--dump-vm", dump_vm_state, "Dump VM state during execution");

    // Performance options
    bool optimize = false;
    app.add_flag("--optimize,-O", optimize, "Enable optimizations");

    // Interactive mode
    bool interactive = false;
    app.add_flag("--interactive,-i", interactive, "Start interactive REPL mode");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    // Show header
    if (verbose) {
        std::cout << "Forge Programming Language Compiler v0.1.0\n";
        std::cout << "===================================\n\n";
    }

    // Handle interactive mode
    if (interactive) {
        std::cout << "Forge Interactive REPL (Not yet implemented)\n";
        std::cout << "Type 'exit' to quit\n";
        return 0;
    }

    // Require input file for non-interactive mode
    if (input_file.empty()) {
        std::cout << "Error: No input file specified\n";
        std::cout << "Use --help for usage information\n";
        return 1;
    }

    if (verbose) {
        std::cout << "Input file: " << input_file << "\n";
        if (!output_file.empty()) {
            std::cout << "Output file: " << output_file << "\n";
        }
        std::cout << "\n";
    }

    // Print compilation stages that will be executed
    if (verbose) {
        std::cout << "Compilation pipeline:\n";
        std::cout << "1. Lexical Analysis";
        if (lexer_only) std::cout << " (STOP)";
        std::cout << "\n";

        if (!lexer_only) {
            std::cout << "2. Parsing (AST Generation)";
            if (parser_only) std::cout << " (STOP)";
            std::cout << "\n";
        }

        if (!lexer_only && !parser_only) {
            std::cout << "3. Compilation (Bytecode Generation)";
            if (compiler_only) std::cout << " (STOP)";
            std::cout << "\n";
        }

        if (!lexer_only && !parser_only && !compiler_only && !no_execute) {
            std::cout << "4. Execution (Virtual Machine)\n";
        }
        std::cout << "\n";
    }

    std::cout << "Starting Forge compilation...\n";
    std::cout << "Processing: " << input_file << "\n";

    std::string source;
    std::ifstream reader(input_file);
    std::getline(reader, source, (char)std::char_traits<char>::eof());

    Tokenizer tokenizer(source);
    std::vector<Token> tokens = tokenizer.tokenize();

    if (dump_tokens) {
        std::cout << "\n=== TOKENS ===\n";
        for (const auto& tok: tokens) {
            std::cout << tok.toString() << "\n";
        }
    }

    Parser parser(tokens);
    std::unique_ptr<Expression> ast = parser.generateAST();

    if (dump_ast) {
        std::cout << "\n=== AST ===\n";
        std::cout << ast->toString() << "\n";
    }

    // Compile AST to bytecode
    BytecodeCompiler compiler;
    auto program = compiler.compile(std::move(ast));

    if (dump_bytecode) {
        std::cout << "\n=== BYTECODE ===\n";
        std::cout << "Instructions:\n";
        for (size_t i = 0; i < program.instructions.size(); ++i) {
            const auto& inst = program.instructions[i];
            std::cout << std::setw(4) << i << ": "
                      << std::setw(12) << std::left << OpCodeToString(inst.opcode)
                      << " " << inst.operand << "\n";
        }

        if (!program.constants.empty()) {
            std::cout << "\nConstants Pool:\n";
            for (size_t i = 0; i < program.constants.size(); ++i) {
                const auto& constant = program.constants[i];
                std::cout << std::setw(4) << i << ": ";
                switch (constant.type) {
                    case TypedValue::Type::INT:
                        std::cout << "INT     " << constant.value.i;
                        break;
                    case TypedValue::Type::FLOAT:
                        std::cout << "FLOAT   " << constant.value.f;
                        break;
                    case TypedValue::Type::DOUBLE:
                        std::cout << "DOUBLE  " << constant.value.d;
                        break;
                    case TypedValue::Type::BOOL:
                        std::cout << "BOOL    " << (constant.value.b ? "true" : "false");
                        break;
                    case TypedValue::Type::STRING:
                        std::cout << "STRING  (id:" << constant.value.string_id << ")";
                        break;
                }
                std::cout << "\n";
            }
        }

        if (!program.strings.empty()) {
            std::cout << "\nString Pool:\n";
            for (size_t i = 0; i < program.strings.size(); ++i) {
                std::cout << std::setw(4) << i << ": \"" << program.strings[i] << "\"\n";
            }
        }
    }

    VirtualMachine vm;
    vm.loadProgram(program.instructions, program.constants, program.strings);
    vm.run();

    if (dump_vm_state) {
        std::cout << "\n=== VM STATE ===\n";
        vm.dumpStack();    
    }


    return 0;
}