#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <memory>
#include <algorithm>
#include "symbol_table.h"
#include "json_helper.h"
#include "../../src/lexer/lexer.h"
#include "../../src/parser/parser.h"
#include "../../src/parser/stmt.h"

namespace sapphire {
namespace lsp {

/**
 * LSP Server implementation with symbol tracking
 */
class LSPServer {
private:
    bool running_;
    std::map<std::string, std::string> documents_;
    SymbolTable symbol_table_;
    
    // Send JSON-RPC response
    void sendResponse(const std::string& id, const std::string& result) {
        std::string response = R"({"jsonrpc":"2.0","id":)" + id + R"(,"result":)" + result + "}";
        std::cout << "Content-Length: " << response.length() << "\r\n\r\n";
        std::cout << response << std::flush;
    }
    
    // Send notification
    void sendNotification(const std::string& method, const std::string& params) {
        std::string notification = R"({"jsonrpc":"2.0","method":")" + method + R"(","params":)" + params + "}";
        std::cout << "Content-Length: " << notification.length() << "\r\n\r\n";
        std::cout << notification << std::flush;
    }
    
    // Handle initialize request
    void handleInitialize(const std::string& id) {
        std::string capabilities = R"({
            "capabilities": {
                "textDocumentSync": 1,
                "completionProvider": {
                    "triggerCharacters": [".", ":"]
                },
                "definitionProvider": true,
                "referencesProvider": true,
                "hoverProvider": true,
                "documentFormattingProvider": true,
                "documentSymbolProvider": true
            }
        })";
        sendResponse(id, capabilities);
    }
    
    // Handle completion request
    void handleCompletion(const std::string& id, const std::string& uri) {
        // Keywords
        std::vector<std::string> keywords = {
            "fn", "class", "if", "else", "elif", "for", "while",
            "return", "let", "var", "import", "from", "as",
            "try", "catch", "finally", "throw", "match",
            "async", "await", "trait", "impl", "chan", "go", "select",
            "Some", "None", "Ok", "Err", "true", "false"
        };
        
        // Build completion items
        std::string items = "[";
        for (size_t i = 0; i < keywords.size(); i++) {
            items += R"({"label":")" + keywords[i] + R"(","kind":14})";
            if (i < keywords.size() - 1) {
                items += ",";
            }
        }
        
        // Add symbols from current document
        auto symbols = symbol_table_.getDocumentSymbols(uri);
        for (const auto& symbol : symbols) {
            if (!items.empty() && items.back() != '[') {
                items += ",";
            }
            int kind = (symbol.type == "function") ? 3 : (symbol.type == "class") ? 5 : 6;
            items += R"({"label":")" + symbol.name + R"(","kind":)" + std::to_string(kind) + "}";
        }
        
        items += "]";
        sendResponse(id, items);
    }
    
    // Handle go to definition
    void handleDefinition(const std::string& id, const std::string& uri, int line, int character) {
        // Get word at position
        auto it = documents_.find(uri);
        if (it == documents_.end()) {
            sendResponse(id, "null");
            return;
        }
        
        std::string word = getWordAtPosition(it->second, line, character);
        if (word.empty()) {
            sendResponse(id, "null");
            return;
        }
        
        // Find definition
        auto definitions = symbol_table_.findDefinitions(word);
        if (definitions.empty()) {
            sendResponse(id, "null");
            return;
        }
        
        // Return first definition
        const auto& def = definitions[0];
        std::string result = R"({
            "uri":")" + def.uri + R"(",
            "range":{
                "start":{"line":)" + std::to_string(def.range.start.line) + R"(,"character":)" + std::to_string(def.range.start.character) + R"(},
                "end":{"line":)" + std::to_string(def.range.end.line) + R"(,"character":)" + std::to_string(def.range.end.character) + R"(}
            }
        })";
        sendResponse(id, result);
    }
    
    // Handle find references
    void handleReferences(const std::string& id, const std::string& uri, int line, int character) {
        // Get word at position
        auto it = documents_.find(uri);
        if (it == documents_.end()) {
            sendResponse(id, "[]");
            return;
        }
        
        std::string word = getWordAtPosition(it->second, line, character);
        if (word.empty()) {
            sendResponse(id, "[]");
            return;
        }
        
        // Find references
        auto references = symbol_table_.findReferences(word);
        if (references.empty()) {
            sendResponse(id, "[]");
            return;
        }
        
        // Build result array
        std::string result = "[";
        for (size_t i = 0; i < references.size(); i++) {
            const auto& ref = references[i];
            result += R"({
                "uri":")" + ref.uri + R"(",
                "range":{
                    "start":{"line":)" + std::to_string(ref.range.start.line) + R"(,"character":)" + std::to_string(ref.range.start.character) + R"(},
                    "end":{"line":)" + std::to_string(ref.range.end.line) + R"(,"character":)" + std::to_string(ref.range.end.character) + R"(}
                }
            })";
            if (i < references.size() - 1) {
                result += ",";
            }
        }
        result += "]";
        sendResponse(id, result);
    }
    
    // Get word at position in document
    std::string getWordAtPosition(const std::string& text, int line, int character) {
        // Split text into lines
        std::vector<std::string> lines;
        std::istringstream stream(text);
        std::string current_line;
        while (std::getline(stream, current_line)) {
            lines.push_back(current_line);
        }
        
        if (line < 0 || line >= static_cast<int>(lines.size())) {
            return "";
        }
        
        const std::string& target_line = lines[line];
        if (character < 0 || character >= static_cast<int>(target_line.length())) {
            return "";
        }
        
        // Find word boundaries
        int start = character;
        while (start > 0 && (std::isalnum(target_line[start - 1]) || target_line[start - 1] == '_')) {
            start--;
        }
        
        int end = character;
        while (end < static_cast<int>(target_line.length()) && (std::isalnum(target_line[end]) || target_line[end] == '_')) {
            end++;
        }
        
        return target_line.substr(start, end - start);
    }
    
    // Handle document open
    void handleDocumentOpen(const std::string& uri, const std::string& text) {
        documents_[uri] = text;
        symbol_table_.clearDocument(uri);
        runDiagnostics(uri, text);
    }
    
    // Handle document change
    void handleDocumentChange(const std::string& uri, const std::string& text) {
        documents_[uri] = text;
        symbol_table_.clearDocument(uri);
        runDiagnostics(uri, text);
    }
    
    // Run diagnostics on document
    void runDiagnostics(const std::string& uri, const std::string& text) {
        try {
            // Try to parse the document
            Lexer lexer(text);
            auto tokens = lexer.tokenize();
            
            Parser parser(tokens);
            auto ast = parser.parse();
            
            // Extract symbols from AST
            extractSymbols(uri, ast, tokens);
            
            // If parsing succeeds, no errors
            std::string diagnostics = R"({"uri":")" + uri + R"(","diagnostics":[]})";
            sendNotification("textDocument/publishDiagnostics", diagnostics);
            
        } catch (const std::exception& e) {
            // Parse error - send diagnostic
            std::string error_msg = JsonHelper::escape(e.what());
            std::string diagnostics = R"({
                "uri":")" + uri + R"(",
                "diagnostics":[{
                    "range":{"start":{"line":0,"character":0},"end":{"line":0,"character":1}},
                    "severity":1,
                    "message":")" + error_msg + R"("
                }]
            })";
            sendNotification("textDocument/publishDiagnostics", diagnostics);
        }
    }
    
    // Extract symbols from AST
    void extractSymbols(const std::string& uri, std::vector<std::unique_ptr<Stmt>>& statements, 
                       const std::vector<Token>& tokens) {
        for (auto& stmt : statements) {
            if (auto* func_decl = dynamic_cast<FunctionDecl*>(stmt.get())) {
                // Find token position for function name
                Position pos = findTokenPosition(tokens, func_decl->name);
                Range range(pos, Position(pos.line, pos.character + func_decl->name.length()));
                
                Symbol symbol(func_decl->name, "function", range, uri);
                symbol_table_.addDefinition(symbol);
            }
            else if (auto* class_decl = dynamic_cast<ClassDecl*>(stmt.get())) {
                // Find token position for class name
                Position pos = findTokenPosition(tokens, class_decl->name);
                Range range(pos, Position(pos.line, pos.character + class_decl->name.length()));
                
                Symbol symbol(class_decl->name, "class", range, uri);
                symbol_table_.addDefinition(symbol);
            }
            else if (auto* var_decl = dynamic_cast<VarDeclStmt*>(stmt.get())) {
                // Find token position for variable name
                Position pos = findTokenPosition(tokens, var_decl->name);
                Range range(pos, Position(pos.line, pos.character + var_decl->name.length()));
                
                Symbol symbol(var_decl->name, "variable", range, uri);
                symbol_table_.addDefinition(symbol);
            }
            else if (auto* trait_decl = dynamic_cast<TraitDecl*>(stmt.get())) {
                // Find token position for trait name
                Position pos = findTokenPosition(tokens, trait_decl->name);
                Range range(pos, Position(pos.line, pos.character + trait_decl->name.length()));
                
                Symbol symbol(trait_decl->name, "trait", range, uri);
                symbol_table_.addDefinition(symbol);
            }
        }
    }
    
    // Find token position by name
    Position findTokenPosition(const std::vector<Token>& tokens, const std::string& name) {
        for (const auto& token : tokens) {
            if (token.lexeme == name) {
                return Position(token.line, token.column);
            }
        }
        return Position(0, 0);
    }
    
public:
    LSPServer() : running_(false) {}
    
    void run() {
        running_ = true;
        
        std::cerr << "Sapphire LSP Server starting...\n";
        
        while (running_) {
            // Read Content-Length header
            std::string header;
            std::getline(std::cin, header);
            
            if (header.empty() || header == "\r") {
                continue;
            }
            
            // Parse content length
            size_t pos = header.find("Content-Length: ");
            if (pos == std::string::npos) {
                continue;
            }
            
            int content_length = std::stoi(header.substr(16));
            
            // Skip empty line
            std::getline(std::cin, header);
            
            // Read message body
            std::string message;
            message.resize(content_length);
            std::cin.read(&message[0], content_length);
            
            // Parse and handle message
            handleMessage(message);
        }
    }
    
    void handleMessage(const std::string& message) {
        std::cerr << "Received: " << message.substr(0, 100) << "...\n";
        
        // Extract method
        std::string method = JsonHelper::extractString(message, "method");
        if (method.empty()) {
            return;
        }
        
        // Extract id
        std::string id = "1";
        size_t id_pos = message.find("\"id\":");
        if (id_pos != std::string::npos) {
            id_pos += 5;
            size_t id_end = message.find_first_of(",}", id_pos);
            id = message.substr(id_pos, id_end - id_pos);
        }
        
        std::cerr << "Method: " << method << ", ID: " << id << "\n";
        
        // Handle different methods
        if (method == "initialize") {
            handleInitialize(id);
        } else if (method == "initialized") {
            // No response needed
        } else if (method == "shutdown") {
            sendResponse(id, "null");
            running_ = false;
        } else if (method == "exit") {
            running_ = false;
        } else if (method == "textDocument/didOpen") {
            std::string uri = JsonHelper::extractString(message, "uri");
            std::string text = JsonHelper::extractString(message, "text");
            if (!uri.empty() && !text.empty()) {
                handleDocumentOpen(uri, text);
            }
        } else if (method == "textDocument/didChange") {
            std::string uri = JsonHelper::extractString(message, "uri");
            std::string text = JsonHelper::extractString(message, "text");
            if (!uri.empty() && !text.empty()) {
                handleDocumentChange(uri, text);
            }
        } else if (method == "textDocument/completion") {
            std::string uri = JsonHelper::extractString(message, "uri");
            handleCompletion(id, uri);
        } else if (method == "textDocument/definition") {
            std::string uri = JsonHelper::extractString(message, "uri");
            int line = JsonHelper::extractInt(message, "line");
            int character = JsonHelper::extractInt(message, "character");
            handleDefinition(id, uri, line, character);
        } else if (method == "textDocument/references") {
            std::string uri = JsonHelper::extractString(message, "uri");
            int line = JsonHelper::extractInt(message, "line");
            int character = JsonHelper::extractInt(message, "character");
            handleReferences(id, uri, line, character);
        }
    }
};

} // namespace lsp
} // namespace sapphire

int main() {
    sapphire::lsp::LSPServer server;
    server.run();
    return 0;
}
