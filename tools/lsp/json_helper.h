#ifndef SAPPHIRE_LSP_JSON_HELPER_H
#define SAPPHIRE_LSP_JSON_HELPER_H

#include <string>
#include <sstream>

namespace sapphire {
namespace lsp {

/**
 * Simple JSON helper functions
 * 
 * Note: This is a minimal implementation. For production,
 * use a proper JSON library like nlohmann/json or rapidjson.
 */
class JsonHelper {
public:
    // Extract string value from JSON
    static std::string extractString(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":\"";
        size_t pos = json.find(search);
        if (pos == std::string::npos) {
            return "";
        }
        
        pos += search.length();
        size_t end = json.find("\"", pos);
        if (end == std::string::npos) {
            return "";
        }
        
        return json.substr(pos, end - pos);
    }
    
    // Extract integer value from JSON
    static int extractInt(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        size_t pos = json.find(search);
        if (pos == std::string::npos) {
            return 0;
        }
        
        pos += search.length();
        size_t end = json.find_first_of(",}", pos);
        if (end == std::string::npos) {
            return 0;
        }
        
        std::string value = json.substr(pos, end - pos);
        try {
            return std::stoi(value);
        } catch (...) {
            return 0;
        }
    }
    
    // Create JSON string
    static std::string createString(const std::string& key, const std::string& value) {
        return "\"" + key + "\":\"" + value + "\"";
    }
    
    // Create JSON number
    static std::string createNumber(const std::string& key, int value) {
        return "\"" + key + "\":" + std::to_string(value);
    }
    
    // Create JSON object
    static std::string createObject(const std::string& content) {
        return "{" + content + "}";
    }
    
    // Create JSON array
    static std::string createArray(const std::string& content) {
        return "[" + content + "]";
    }
    
    // Escape string for JSON
    static std::string escape(const std::string& str) {
        std::string result;
        for (char c : str) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c; break;
            }
        }
        return result;
    }
};

} // namespace lsp
} // namespace sapphire

#endif // SAPPHIRE_LSP_JSON_HELPER_H
