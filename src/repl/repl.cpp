#include "repl.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>

#ifdef __unix__
#include <unistd.h>
#include <termios.h>
#endif

namespace sapphire {

REPL::REPL() : interpreter(std::make_shared<Interpreter>()), running(true) {}

void REPL::run() {
    printWelcome();
    
    while (running) {
        try {
            // Check watched files for changes
            checkWatchedFiles();
            
            std::string line = readLine(getPrompt());
            
            if (line.empty()) {
                continue;
            }
            
            // Add to history
            history.push_back(line);
            
            // Check for commands
            if (handleCommand(line)) {
                continue;
            }
            
            // Execute code
            executeCode(line);
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    std::cout << "\nGoodbye! 💎\n";
}

void REPL::processLine(const std::string& line) {
    if (line.empty()) return;
    
    if (handleCommand(line)) {
        return;
    }
    
    executeCode(line);
}

bool REPL::handleCommand(const std::string& line) {
    if (line.empty() || line[0] != ':') {
        return false;
    }
    
    std::string cmd = line.substr(1);
    
    if (cmd == "help" || cmd == "h") {
        printHelp();
        return true;
    }
    
    if (cmd == "quit" || cmd == "exit" || cmd == "q") {
        running = false;
        return true;
    }
    
    if (cmd == "clear" || cmd == "cls") {
        clearScreen();
        return true;
    }
    
    if (cmd == "vars" || cmd == "variables") {
        showVariables();
        return true;
    }
    
    if (cmd == "funcs" || cmd == "functions") {
        showFunctions();
        return true;
    }
    
    if (cmd == "reset") {
        resetEnvironment();
        return true;
    }
    
    // Check for :reload command with filename
    if (cmd.substr(0, 6) == "reload") {
        std::string filename = cmd.substr(6);
        // Trim whitespace
        filename.erase(0, filename.find_first_not_of(" \t"));
        filename.erase(filename.find_last_not_of(" \t") + 1);
        
        if (filename.empty()) {
            std::cerr << "Usage: :reload <filename>" << std::endl;
        } else {
            reloadFile(filename);
        }
        return true;
    }
    
    // Check for :watch command with filename
    if (cmd.substr(0, 5) == "watch") {
        std::string filename = cmd.substr(5);
        // Trim whitespace
        filename.erase(0, filename.find_first_not_of(" \t"));
        filename.erase(filename.find_last_not_of(" \t") + 1);
        
        if (filename.empty()) {
            showWatchedFiles();
        } else {
            watchFile(filename);
        }
        return true;
    }
    
    // Check for :unwatch command with filename
    if (cmd.substr(0, 7) == "unwatch") {
        std::string filename = cmd.substr(7);
        // Trim whitespace
        filename.erase(0, filename.find_first_not_of(" \t"));
        filename.erase(filename.find_last_not_of(" \t") + 1);
        
        if (filename.empty()) {
            std::cerr << "Usage: :unwatch <filename>" << std::endl;
        } else {
            unwatchFile(filename);
        }
        return true;
    }
    
    std::cerr << "Unknown command: " << line << std::endl;
    std::cerr << "Type :help for available commands" << std::endl;
    return true;
}

void REPL::executeCode(const std::string& code) {
    try {
        // Lexer
        Lexer lexer(code);
        auto tokens = lexer.tokenize();
        
        // Parser
        Parser parser(tokens);
        auto statements = parser.parse();
        
        // Execute
        interpreter->interpret(statements);
        
        // Print last value if it's an expression (not nullptr)
        auto last_val = interpreter->getLastValue();
        if (!std::holds_alternative<std::nullptr_t>(last_val)) {
            std::string val_str = interpreter->valueToString(last_val);
            if (!val_str.empty() && val_str != "none") {
                std::cout << "=> " << val_str << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void REPL::printWelcome() {
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║         Sapphire REPL v1.0.0-beta.2                          ║\n";
    std::cout << "║         Interactive Programming Environment                   ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    std::cout << "Type :help for available commands\n";
    std::cout << "Type :quit to exit\n";
    std::cout << "\n";
}

void REPL::printHelp() {
    std::cout << "\nAvailable Commands:\n";
    std::cout << "  :help, :h          - Show this help message\n";
    std::cout << "  :quit, :exit, :q   - Exit the REPL\n";
    std::cout << "  :clear, :cls       - Clear the screen\n";
    std::cout << "  :vars              - Show defined variables\n";
    std::cout << "  :funcs             - Show defined functions\n";
    std::cout << "  :reset             - Reset the environment\n";
    std::cout << "  :reload <file>     - Reload a Sapphire file\n";
    std::cout << "  :watch <file>      - Watch a file for changes (auto-reload)\n";
    std::cout << "  :watch             - Show watched files\n";
    std::cout << "  :unwatch <file>    - Stop watching a file\n";
    std::cout << "\nTips:\n";
    std::cout << "  - Variables and functions persist across inputs\n";
    std::cout << "  - Expressions are automatically printed\n";
    std::cout << "  - Use :reload to update code without restarting\n";
    std::cout << "  - Use :watch for automatic hot reloading\n";
    std::cout << "  - Use Ctrl+C to interrupt execution\n";
    std::cout << "\n";
}

void REPL::showVariables() {
    std::cout << "\nDefined Variables:\n";
    auto env = interpreter->getEnvironment();
    auto vars = env->getAllVariables();
    
    if (vars.empty()) {
        std::cout << "  (none)\n";
    } else {
        for (const auto& [name, value] : vars) {
            // Skip built-in functions
            if (name.find("__builtin") != std::string::npos) {
                continue;
            }
            std::cout << "  " << name << " = " << interpreter->valueToString(value) << "\n";
        }
    }
    std::cout << "\n";
}

void REPL::showFunctions() {
    std::cout << "\nDefined Functions:\n";
    auto env = interpreter->getEnvironment();
    auto vars = env->getAllVariables();
    
    bool found = false;
    for (const auto& [name, value] : vars) {
        if (std::holds_alternative<std::shared_ptr<Function>>(value)) {
            auto func = std::get<std::shared_ptr<Function>>(value);
            std::cout << "  fn " << name << "(";
            for (size_t i = 0; i < func->params.size(); i++) {
                std::cout << func->params[i];
                if (i < func->params.size() - 1) std::cout << ", ";
            }
            std::cout << ")\n";
            found = true;
        }
    }
    
    if (!found) {
        std::cout << "  (none)\n";
    }
    std::cout << "\n";
}

void REPL::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void REPL::resetEnvironment() {
    interpreter = std::make_shared<Interpreter>();
    std::cout << "Environment reset.\n";
}

void REPL::reloadFile(const std::string& filename) {
    try {
        // Read the file
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
            return;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string code = buffer.str();
        file.close();
        
        std::cout << "Reloading '" << filename << "'..." << std::endl;
        
        // Lexer
        Lexer lexer(code);
        auto tokens = lexer.tokenize();
        
        // Parser
        Parser parser(tokens);
        auto statements = parser.parse();
        
        // Execute in current environment (preserves state)
        interpreter->interpret(statements);
        
        std::cout << "✓ File reloaded successfully" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error reloading file: " << e.what() << std::endl;
    }
}

std::string REPL::getPrompt() const {
    return "sapphire> ";
}

std::string REPL::readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    
    // Check for EOF (Ctrl+D)
    if (std::cin.eof()) {
        running = false;
        return "";
    }
    
    return line;
}

void REPL::watchFile(const std::string& filename) {
    try {
        // Check if file exists
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
            return;
        }
        file.close();
        
        // Get initial modification time
        auto mod_time = getFileModTime(filename);
        
        // Add to watched files
        watched_files[filename] = mod_time;
        
        std::cout << "✓ Now watching '" << filename << "' for changes" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error watching file: " << e.what() << std::endl;
    }
}

void REPL::unwatchFile(const std::string& filename) {
    auto it = watched_files.find(filename);
    if (it != watched_files.end()) {
        watched_files.erase(it);
        std::cout << "✓ Stopped watching '" << filename << "'" << std::endl;
    } else {
        std::cerr << "File '" << filename << "' is not being watched" << std::endl;
    }
}

void REPL::showWatchedFiles() {
    std::cout << "\nWatched Files:\n";
    if (watched_files.empty()) {
        std::cout << "  (none)\n";
    } else {
        for (const auto& [filename, _] : watched_files) {
            std::cout << "  " << filename << "\n";
        }
    }
    std::cout << "\n";
}

void REPL::checkWatchedFiles() {
    for (auto& [filename, last_mod_time] : watched_files) {
        try {
            auto current_mod_time = getFileModTime(filename);
            
            // Check if file was modified
            if (current_mod_time > last_mod_time) {
                std::cout << "\n🔄 File '" << filename << "' changed, reloading...\n" << std::endl;
                reloadFile(filename);
                last_mod_time = current_mod_time;
            }
            
        } catch (const std::exception& e) {
            // File might have been deleted, remove from watch list
            std::cerr << "Warning: Could not check '" << filename << "': " << e.what() << std::endl;
            watched_files.erase(filename);
            break;
        }
    }
}

std::chrono::system_clock::time_point REPL::getFileModTime(const std::string& filename) {
    struct stat file_stat;
    if (stat(filename.c_str(), &file_stat) != 0) {
        throw std::runtime_error("Could not stat file");
    }
    
    return std::chrono::system_clock::from_time_t(file_stat.st_mtime);
}

} // namespace sapphire
