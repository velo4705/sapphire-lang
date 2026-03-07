#ifndef SAPPHIRE_REPL_H
#define SAPPHIRE_REPL_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <chrono>
#include "../interpreter/interpreter.h"

namespace sapphire {

class REPL {
public:
    REPL();
    ~REPL() = default;
    
    // Start the REPL
    void run();
    
private:
    std::shared_ptr<Interpreter> interpreter;
    std::vector<std::string> history;
    bool running;
    
    // File watching
    std::map<std::string, std::chrono::system_clock::time_point> watched_files;
    
    // Process a line of input
    void processLine(const std::string& line);
    
    // Handle REPL commands
    bool handleCommand(const std::string& line);
    
    // Execute Sapphire code
    void executeCode(const std::string& code);
    
    // Print welcome message
    void printWelcome();
    
    // Print help
    void printHelp();
    
    // Show defined variables
    void showVariables();
    
    // Show defined functions
    void showFunctions();
    
    // Clear screen
    void clearScreen();
    
    // Reset environment
    void resetEnvironment();
    
    // Reload a file
    void reloadFile(const std::string& filename);
    
    // Watch a file for changes
    void watchFile(const std::string& filename);
    
    // Unwatch a file
    void unwatchFile(const std::string& filename);
    
    // Show watched files
    void showWatchedFiles();
    
    // Check watched files for changes
    void checkWatchedFiles();
    
    // Get file modification time
    std::chrono::system_clock::time_point getFileModTime(const std::string& filename);
    
    // Get prompt string
    std::string getPrompt() const;
    
    // Read line with history support
    std::string readLine(const std::string& prompt);
};

} // namespace sapphire

#endif // SAPPHIRE_REPL_H
