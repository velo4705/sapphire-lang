#include "formatter.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <vector>

namespace sapphire {

Formatter::Formatter(const FormatOptions& options) 
    : options_(options), current_indent_(0) {}

void Formatter::add_indent() {
    for (int i = 0; i < current_indent_ * options_.indent_size; i++) {
        result_ += ' ';
    }
}

void Formatter::add_line(const std::string& line) {
    if (!line.empty()) {
        add_indent();
        result_ += line;
    }
    result_ += '\n';
}

void Formatter::increase_indent() {
    current_indent_++;
}

void Formatter::decrease_indent() {
    if (current_indent_ > 0) {
        current_indent_--;
    }
}

std::string Formatter::format(const std::string& code) {
    result_.clear();
    
    // First pass: parse into lines and determine indentation
    std::istringstream stream(code);
    std::string line;
    std::vector<std::pair<std::string, int>> lines_with_indent;
    int indent_level = 0;
    
    while (std::getline(stream, line)) {
        // Trim whitespace
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) {
            // Empty line
            lines_with_indent.push_back({"", indent_level});
            continue;
        }
        
        std::string trimmed = line.substr(start);
        
        // Dedent before top-level keywords
        if (trimmed.find("fn ") == 0 || trimmed.find("class ") == 0) {
            indent_level = 0;
        }
        
        // Store line with current indent
        lines_with_indent.push_back({trimmed, indent_level});
        
        // Increase indent after lines ending with ':'
        if (!trimmed.empty() && trimmed.back() == ':') {
            indent_level++;
        }
    }
    
    // Second pass: format each line
    current_indent_ = 0;
    bool in_string = false;
    
    for (const auto& [line, indent] : lines_with_indent) {
        current_indent_ = indent;
        
        if (line.empty()) {
            result_ += '\n';
            continue;
        }
        
        // Handle comments
        if (line[0] == '#') {
            add_line(line);
            continue;
        }
        
        // Format the line
        std::string formatted;
        for (size_t i = 0; i < line.length(); i++) {
            char c = line[i];
            
            // Handle strings
            if (c == '"' && (i == 0 || line[i-1] != '\\')) {
                in_string = !in_string;
                formatted += c;
                continue;
            }
            
            if (in_string) {
                formatted += c;
                continue;
            }
            
            // Add spaces around operators
            if (options_.space_around_operators) {
                // Check for -> operator (don't split it)
                if (c == '-' && i + 1 < line.length() && line[i+1] == '>') {
                    if (!formatted.empty() && formatted.back() != ' ') {
                        formatted += ' ';
                    }
                    formatted += "->";
                    i++; // Skip the '>'
                    if (i + 1 < line.length() && line[i+1] != ' ' && line[i+1] != ':') {
                        formatted += ' ';
                    }
                    continue;
                }
                
                if (c == '=' || c == '+' || c == '-' || c == '*' || c == '/' ||
                    c == '<' || c == '>' || c == '!') {
                    if (!formatted.empty() && formatted.back() != ' ') {
                        formatted += ' ';
                    }
                    formatted += c;
                    if (i + 1 < line.length() && line[i+1] != ' ' && line[i+1] != '=') {
                        formatted += ' ';
                    }
                    continue;
                }
            }
            
            // Add space after comma
            if (options_.space_after_comma && c == ',') {
                formatted += c;
                if (i + 1 < line.length() && line[i+1] != ' ') {
                    formatted += ' ';
                }
                continue;
            }
            
            formatted += c;
        }
        
        if (!formatted.empty()) {
            add_line(formatted);
        }
    }
    
    return result_;
}

std::string Formatter::format_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return format(buffer.str());
}

bool Formatter::format_file_inplace(const std::string& filename) {
    std::string formatted = format_file(filename);
    if (formatted.empty()) {
        return false;
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << formatted;
    return true;
}

} // namespace sapphire
