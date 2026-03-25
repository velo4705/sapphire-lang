#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"
#include "semantic/type_checker.h"
#include "semantic/type_inference.h"
#include "codegen/llvm_codegen.h"
#include "codegen/llvm_codegen.h"
#include "repl/repl.h"
#include "error/exception.h"
#include "error/error_codes.h"
#include "error/error_reporter.h"

void printUsage(const char* program) {
    (void)program;
    std::cout << "Sapphire Programming Language 💎\n";
    std::cout << "A Fast and Powerful Language built with simplicity on top.\n\n";
    std::cout << "Usage:\n";
    std::cout << "  sapp                         # Start interactive REPL\n";
    std::cout << "  sapp <file.spp>              # Run a program (interpreter)\n";
    std::cout << "  sapp compile <file.spp>      # Compile and show LLVM IR\n";
    std::cout << "  sapp --version               # Show version\n";
    std::cout << "  sapp --update                # Update to latest version\n";
    std::cout << "  sapp --help                  # Show this help\n";
    std::cout << "\nNote: 'sapphire' is an alias for 'sapp'\n";
}

void checkForUpdates() {
    std::cout << "Checking for updates...\n";

    // Read current version from the binary's directory
    std::string current_version = "1.0-beta.7";
    char exe_path[4096] = {};
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len > 0) {
        exe_path[len] = '\0';
        std::string dir(exe_path);
        dir = dir.substr(0, dir.rfind('/'));
        std::ifstream vf(dir + "/VERSION");
        if (vf.is_open()) { std::getline(vf, current_version); vf.close(); }
    }
    // trim whitespace
    while (!current_version.empty() && (current_version.back() == '\n' || current_version.back() == '\r' || current_version.back() == ' '))
        current_version.pop_back();

    std::cout << "Current version: v" << current_version << "\n";

    // Fetch latest VERSION from GitHub
    std::string latest_version;
    FILE* pipe = popen("curl -fsSL --max-time 5 https://raw.githubusercontent.com/velo4705/sapphire-lang/main/VERSION 2>/dev/null", "r");
    if (pipe) {
        char buf[64] = {};
        if (fgets(buf, sizeof(buf), pipe)) {
            latest_version = buf;
            while (!latest_version.empty() && (latest_version.back() == '\n' || latest_version.back() == '\r' || latest_version.back() == ' '))
                latest_version.pop_back();
        }
        pclose(pipe);
    }

    if (latest_version.empty()) {
        std::cout << "Could not reach GitHub. Check your internet connection.\n";
        std::cout << "Latest release: https://github.com/velo4705/sapphire-lang/releases\n";
        return;
    }

    std::cout << "Latest version:  v" << latest_version << "\n\n";

    if (current_version == latest_version) {
        std::cout << "✓ You are already on the latest version.\n";
    } else {
        std::cout << "↑ Update available: v" << current_version << " → v" << latest_version << "\n";
        std::cout << "\nTo update, run:\n";
        std::cout << "  sapp --update\n";
        std::cout << "\nOr manually:\n";
        std::cout << "  curl -fsSL https://raw.githubusercontent.com/velo4705/sapphire-lang/main/install.sh | bash\n";
    }
}

void updateSapphire() {
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║         Updating Sapphire...                                  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";

    // Find the directory this binary lives in — that's the repo root
    // Try /proc/self/exe first (Linux), fall back to argv[0]
    char exe_path[4096] = {};
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    std::string repo_dir;
    if (len > 0) {
        exe_path[len] = '\0';
        std::string p(exe_path);
        repo_dir = p.substr(0, p.rfind('/'));
    }

    // Check if this looks like a git repo
    bool is_git = false;
    if (!repo_dir.empty()) {
        std::string check = "test -d \"" + repo_dir + "/.git\"";
        is_git = (system(check.c_str()) == 0);
    }

    if (is_git) {
        std::cout << "Found git repo at: " << repo_dir << "\n";
        std::cout << "Pulling latest changes...\n\n";

        std::string pull_cmd  = "git -C \"" + repo_dir + "\" pull origin main";
        std::string build_cmd = "make -C \"" + repo_dir + "\" quick";

        int pull_result = system(pull_cmd.c_str());
        if (pull_result != 0) {
            std::cout << "✗ git pull failed. Check your internet connection or repo state.\n";
            std::cout << "  Try: git -C " << repo_dir << " pull origin main\n";
            return;
        }

        std::cout << "\nRebuilding...\n\n";
        int build_result = system(build_cmd.c_str());
        if (build_result == 0) {
            std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
            std::cout << "║  ✓ Sapphire updated successfully!                            ║\n";
            std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
            std::cout << "\nRun: sapp --version\n";
        } else {
            std::cout << "✗ Build failed after pull. Try: make -C " << repo_dir << " quick\n";
        }
    } else {
        // Not a git repo — fall back to the curl installer
        std::cout << "Not a git installation. Downloading latest installer...\n\n";
        int result = system("curl -fsSL https://raw.githubusercontent.com/velo4705/sapphire-lang/main/install.sh | bash");
        if (result == 0) {
            std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
            std::cout << "║  ✓ Sapphire updated successfully!                            ║\n";
            std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
            std::cout << "\nRun: sapp --version\n";
        } else {
            std::cout << "✗ Update failed. Try manually:\n";
            std::cout << "  curl -fsSL https://raw.githubusercontent.com/velo4705/sapphire-lang/main/install.sh | bash\n";
        }
    }
}

int main(int argc, char* argv[]) {
    // No arguments - start REPL
    if (argc < 2) {
        sapphire::REPL repl;
        repl.run();
        return 0;
    }

    std::string arg = argv[1];
    
    if (arg == "--version") {
        std::cout << "Sapphire v1.0-beta.7\n";
        return 0;
    }
    
    if (arg == "--update") {
        updateSapphire();
        return 0;
    }
    
    if (arg == "--check-updates") {
        checkForUpdates();
        return 0;
    }
    
    if (arg == "--help") {
        printUsage(argv[0]);
        return 0;
    }

    // sapp --explain E201  →  print full error book entry
    if (arg == "--explain") {
        if (argc < 3) {
            std::cerr << "Usage: sapp --explain <error-code>  e.g. sapp --explain E201\n";
            return 1;
        }
        std::string code = argv[2];
        const sapphire::ErrorCode* ec = sapphire::lookup_error_code(code);
        if (!ec) {
            sapphire::ErrorFormatter::print_simple("",
                "unknown error code '" + code + "' — valid range: E1A0–E1A7, E2A0–E2A3, E3A0–E3A4, E4A0–E4A4, E5A0–E5A1, E6A0–E6A1, E7A0–E7A2");
            return 1;
        }
        // Print a formatted explanation using the error formatter
        sapphire::ErrorFormatter::print(
            code,
            std::string(ec->title),
            "", -1, -1, "",
            std::string(ec->note)
        );
        return 0;
    }

    // Check for compile command
    bool compile_mode = false;
    std::string filename;
    std::string output_file;
    bool emit_llvm = false;
    
    if (arg == "compile") {
        if (argc < 3) {
            std::cerr << "Error: compile command requires a filename\n";
            std::cerr << "Usage: sapp compile <file.spp> [-o output] [--emit-llvm]\n";
            return 1;
        }
        compile_mode = true;
        filename = argv[2];
        
        // Parse additional arguments
        for (int i = 3; i < argc; i++) {
            std::string opt = argv[i];
            if (opt == "-o" && i + 1 < argc) {
                output_file = argv[++i];
            } else if (opt == "--emit-llvm") {
                emit_llvm = true;
            }
        }
    } else {
        filename = argv[1];
    }
    
    // Read source file
    std::ifstream file(filename);
    if (!file.is_open()) {
        sapphire::ErrorFormatter::print("E6A0",
            "could not open file '" + filename + "'",
            filename, -1, -1, "");
        return 1;
    }

    std::string source((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();

    try {
        // Lexer - tokenize source
        sapphire::Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        // Parser - build AST
        sapphire::Parser parser(tokens, source, filename);
        auto statements = parser.parse();
        
        if (compile_mode) {
            // Code generation mode
            std::cerr << "=== Compiling " << filename << " ===\n\n";
            
            sapphire::LLVMCodeGen codegen(filename);
            codegen.generate(statements);
            
            // Apply optimizations if requested
            // Note: Optimization removes unused variables (dead code elimination)
            // codegen.optimize(2); // -O2 by default
            
            if (emit_llvm || output_file.empty()) {
                // Show LLVM IR
                std::cerr << "\n=== Generated LLVM IR ===\n\n";
                codegen.printIR();
            }
            
            if (!output_file.empty()) {
                // Generate executable
                std::cerr << "\n=== Generating executable ===\n\n";
                codegen.writeExecutable(output_file);
            }
            
            std::cerr << "\n=== Compilation successful! ===\n";
        } else {
            // Interpreter mode
            // Interpreter - execute
            sapphire::Interpreter interpreter;
            interpreter.interpret(statements);
        }
        
    } catch (const sapphire::SapphireException& e) {
        // Map exception type to error code
        std::string code;
        const std::string& t = e.getTypeName();
        if      (t == "IndexError")          code = "E4A0";
        else if (t == "DivisionByZeroError") code = "E4A1";
        else if (t == "FileNotFoundError")   code = "E6A0";
        else if (t == "PermissionError")     code = "E6A1";
        else if (t == "ValueError")          code = "E7A0";
        else if (t == "TypeError")           code = "E3A0";

        sapphire::ErrorFormatter::print(
            code,
            e.getMessage(),
            e.getFileName().empty() ? filename : e.getFileName(),
            e.getLineNumber(),
            e.getColumnNumber(),
            source
        );
        return 1;
    } catch (const std::exception& e) {
        sapphire::ErrorFormatter::print_simple("", e.what());
        return 1;
    }
    
    return 0;
}
