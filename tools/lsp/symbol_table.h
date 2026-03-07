#ifndef SAPPHIRE_LSP_SYMBOL_TABLE_H
#define SAPPHIRE_LSP_SYMBOL_TABLE_H

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace sapphire {
namespace lsp {

/**
 * Position in a document
 */
struct Position {
    int line;
    int character;
    
    Position() : line(0), character(0) {}
    Position(int l, int c) : line(l), character(c) {}
};

/**
 * Range in a document
 */
struct Range {
    Position start;
    Position end;
    
    Range() {}
    Range(Position s, Position e) : start(s), end(e) {}
};

/**
 * Symbol information
 */
struct Symbol {
    std::string name;
    std::string type;  // "function", "class", "variable", etc.
    Range range;
    std::string uri;
    
    Symbol() {}
    Symbol(const std::string& n, const std::string& t, const Range& r, const std::string& u)
        : name(n), type(t), range(r), uri(u) {}
};

/**
 * Symbol table for tracking definitions and references
 */
class SymbolTable {
private:
    // Map from symbol name to list of definitions
    std::map<std::string, std::vector<Symbol>> definitions_;
    
    // Map from symbol name to list of references
    std::map<std::string, std::vector<Symbol>> references_;
    
public:
    SymbolTable() = default;
    
    // Add a definition
    void addDefinition(const Symbol& symbol) {
        definitions_[symbol.name].push_back(symbol);
    }
    
    // Add a reference
    void addReference(const Symbol& symbol) {
        references_[symbol.name].push_back(symbol);
    }
    
    // Find definition for a symbol
    std::vector<Symbol> findDefinitions(const std::string& name) const {
        auto it = definitions_.find(name);
        if (it != definitions_.end()) {
            return it->second;
        }
        return {};
    }
    
    // Find all references to a symbol
    std::vector<Symbol> findReferences(const std::string& name) const {
        auto it = references_.find(name);
        if (it != references_.end()) {
            return it->second;
        }
        return {};
    }
    
    // Clear all symbols for a document
    void clearDocument(const std::string& uri) {
        // Remove all symbols from this document
        for (auto& pair : definitions_) {
            auto& symbols = pair.second;
            symbols.erase(
                std::remove_if(symbols.begin(), symbols.end(),
                    [&uri](const Symbol& s) { return s.uri == uri; }),
                symbols.end()
            );
        }
        
        for (auto& pair : references_) {
            auto& symbols = pair.second;
            symbols.erase(
                std::remove_if(symbols.begin(), symbols.end(),
                    [&uri](const Symbol& s) { return s.uri == uri; }),
                symbols.end()
            );
        }
    }
    
    // Get all symbols in a document
    std::vector<Symbol> getDocumentSymbols(const std::string& uri) const {
        std::vector<Symbol> result;
        
        for (const auto& pair : definitions_) {
            for (const auto& symbol : pair.second) {
                if (symbol.uri == uri) {
                    result.push_back(symbol);
                }
            }
        }
        
        return result;
    }
};

} // namespace lsp
} // namespace sapphire

#endif // SAPPHIRE_LSP_SYMBOL_TABLE_H
