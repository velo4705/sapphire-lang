#ifndef SAPPHIRE_LSP_SERVER_H
#define SAPPHIRE_LSP_SERVER_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include "symbol_table.h"
#include "json_helper.h"
#include "../../src/lexer/lexer.h"
#include "../../src/parser/parser.h"
#include "../../src/parser/stmt.h"

namespace sapphire {
namespace lsp {

/**
 * LSP Server implementation
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
    void handleCompletion(const std::string& id) {
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
        items += "]";
        
        sendResponse(id, items);
    }
    
    // Handle go to definition
    void handleDefinition(const std::string& id, const std::string& uri, int line, int character) {
        // For now, return empty
        // TODO: Use symbol table to find definition
        sendResponse(id, "null");
    }
    
    // Handle find references
    void handleReferences(const std::string& id, const std::string& uri, int line, int character) {
        // For now, return empty
        // TODO: Use symbol table to find references
        sendResponse(id, "[]");
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
            extractSymbols(uri, ast);
            
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
    void extractSymbols(const std::string& uri, std::vector<std::unique_ptr<Stmt>>& statements) {
        for (auto& stmt : statements) {
            if (auto* func_decl = dynamic_cast<FunctionDecl*>(stmt.get())) {
                // Add function definition
                Symbol symbol(
                    func_decl->name,
                    "function",
                    Range(Position(0, 0), Position(0, 0)),  // TODO: Get actual position
                    uri
                );
                symbol_table_.addDefinition(symbol);
            }
            else if (auto* class_decl = dynamic_cast<ClassDecl*>(stmt.get())) {
                // Add class definition
                Symbol symbol(
                    class_decl->name,
                    "class",
                    Range(Position(0, 0), Position(0, 0)),  // TODO: Get actual position
                    uri
                );
                symbol_table_.addDefinition(symbol);
            }
        }
    }
    
public:
    LSPServer() : running_(false) {}
    
    void run();
    void handleMessage(const std::string& message);
};

} // namespace lsp
} // namespace sapphire

#endif // SAPPHIRE_LSP_SERVER_H
