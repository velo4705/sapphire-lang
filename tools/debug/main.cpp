#include "dap_server.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace sapphire {
namespace debug {

DAPServer::DAPServer() 
    : running_(false), next_breakpoint_id_(1), next_seq_(1), state_(DebugState::STOPPED) {}

void DAPServer::sendResponse(const std::string& request_seq, const std::string& command, 
                             const std::string& body) {
    std::string response = R"({"type":"response","request_seq":)" + request_seq + 
                          R"(,"success":true,"command":")" + command + R"(","seq":)" + 
                          std::to_string(next_seq_++) + R"(,"body":)" + body + "}";
    
    std::cout << "Content-Length: " << response.length() << "\r\n\r\n";
    std::cout << response << std::flush;
}

void DAPServer::sendEvent(const std::string& event, const std::string& body) {
    std::string message = R"({"type":"event","event":")" + event + 
                         R"(","seq":)" + std::to_string(next_seq_++) + 
                         R"(,"body":)" + body + "}";
    
    std::cout << "Content-Length: " << message.length() << "\r\n\r\n";
    std::cout << message << std::flush;
}

void DAPServer::handleInitialize(const std::string& seq) {
    std::string capabilities = R"({
        "supportsConfigurationDoneRequest": true,
        "supportsEvaluateForHovers": false,
        "supportsStepBack": false,
        "supportsSetVariable": false,
        "supportsRestartFrame": false,
        "supportsGotoTargetsRequest": false,
        "supportsStepInTargetsRequest": false,
        "supportsCompletionsRequest": false,
        "supportsModulesRequest": false,
        "supportsRestartRequest": false,
        "supportsExceptionOptions": false,
        "supportsValueFormattingOptions": false,
        "supportsExceptionInfoRequest": false,
        "supportTerminateDebuggee": false,
        "supportsDelayedStackTraceLoading": false,
        "supportsLoadedSourcesRequest": false,
        "supportsLogPoints": false,
        "supportsTerminateThreadsRequest": false,
        "supportsSetExpression": false,
        "supportsTerminateRequest": false,
        "supportsDataBreakpoints": false,
        "supportsReadMemoryRequest": false,
        "supportsDisassembleRequest": false,
        "supportsCancelRequest": false,
        "supportsBreakpointLocationsRequest": false,
        "supportsClipboardContext": false
    })";
    
    sendResponse(seq, "initialize", capabilities);
    sendEvent("initialized", "{}");
}

void DAPServer::handleLaunch(const std::string& seq, const std::string& program) {
    std::cerr << "Launching program: " << program << "\n";
    state_ = DebugState::RUNNING;
    sendResponse(seq, "launch", "{}");
}

void DAPServer::handleSetBreakpoints(const std::string& seq, const std::string& file, 
                                    const std::vector<int>& lines) {
    std::cerr << "Setting breakpoints in " << file << "\n";
    
    // Clear existing breakpoints for this file
    if (breakpoints_by_file_.count(file)) {
        for (int bp_id : breakpoints_by_file_[file]) {
            breakpoints_.erase(bp_id);
        }
        breakpoints_by_file_[file].clear();
    }
    
    // Add new breakpoints
    std::string breakpoints_json = "[";
    for (size_t i = 0; i < lines.size(); i++) {
        int bp_id = next_breakpoint_id_++;
        Breakpoint bp(file, lines[i], bp_id);
        bp.verified = true;
        
        breakpoints_[bp_id] = bp;
        breakpoints_by_file_[file].insert(bp_id);
        
        breakpoints_json += R"({"id":)" + std::to_string(bp_id) + 
                           R"(,"verified":true,"line":)" + std::to_string(lines[i]) + "}";
        if (i < lines.size() - 1) {
            breakpoints_json += ",";
        }
    }
    breakpoints_json += "]";
    
    std::string body = R"({"breakpoints":)" + breakpoints_json + "}";
    sendResponse(seq, "setBreakpoints", body);
}

void DAPServer::handleConfigurationDone(const std::string& seq) {
    std::cerr << "Configuration done\n";
    sendResponse(seq, "configurationDone", "{}");
}

void DAPServer::handleThreads(const std::string& seq) {
    std::string body = R"({"threads":[{"id":1,"name":"Main Thread"}]})";
    sendResponse(seq, "threads", body);
}

void DAPServer::handleStackTrace(const std::string& seq, int threadId) {
    // For now, return empty stack trace
    // This will be populated by the interpreter
    std::string body = R"({"stackFrames":[],"totalFrames":0})";
    sendResponse(seq, "stackTrace", body);
}

void DAPServer::handleScopes(const std::string& seq, int frameId) {
    // Return local and global scopes
    std::string body = R"({
        "scopes":[
            {"name":"Local","variablesReference":1,"expensive":false},
            {"name":"Global","variablesReference":2,"expensive":false}
        ]
    })";
    sendResponse(seq, "scopes", body);
}

void DAPServer::handleVariables(const std::string& seq, int variablesReference) {
    // For now, return empty variables
    // This will be populated by the interpreter
    std::string body = R"({"variables":[]})";
    sendResponse(seq, "variables", body);
}

void DAPServer::handleContinue(const std::string& seq) {
    std::cerr << "Continue execution\n";
    state_ = DebugState::RUNNING;
    sendResponse(seq, "continue", R"({"allThreadsContinued":true})");
}

void DAPServer::handleNext(const std::string& seq) {
    std::cerr << "Step over\n";
    state_ = DebugState::STEP_OVER;
    sendResponse(seq, "next", "{}");
}

void DAPServer::handleStepIn(const std::string& seq) {
    std::cerr << "Step in\n";
    state_ = DebugState::STEP_IN;
    sendResponse(seq, "stepIn", "{}");
}

void DAPServer::handleStepOut(const std::string& seq) {
    std::cerr << "Step out\n";
    state_ = DebugState::STEP_OUT;
    sendResponse(seq, "stepOut", "{}");
}

void DAPServer::handlePause(const std::string& seq) {
    std::cerr << "Pause execution\n";
    state_ = DebugState::PAUSED;
    sendResponse(seq, "pause", "{}");
}

void DAPServer::handleDisconnect(const std::string& seq) {
    std::cerr << "Disconnect\n";
    sendResponse(seq, "disconnect", "{}");
    running_ = false;
}

void DAPServer::run() {
    running_ = true;
    std::cerr << "Sapphire Debug Adapter starting...\n";
    
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
        
        // Handle message
        handleMessage(message);
    }
}

void DAPServer::handleMessage(const std::string& message) {
    std::cerr << "Received: " << message.substr(0, 100) << "...\n";
    
    // Extract command
    size_t command_pos = message.find("\"command\":\"");
    if (command_pos == std::string::npos) {
        return;
    }
    
    command_pos += 11;
    size_t command_end = message.find("\"", command_pos);
    std::string command = message.substr(command_pos, command_end - command_pos);
    
    // Extract seq
    std::string seq = "1";
    size_t seq_pos = message.find("\"seq\":");
    if (seq_pos != std::string::npos) {
        seq_pos += 6;
        size_t seq_end = message.find_first_of(",}", seq_pos);
        seq = message.substr(seq_pos, seq_end - seq_pos);
    }
    
    std::cerr << "Command: " << command << ", Seq: " << seq << "\n";
    
    // Handle different commands
    if (command == "initialize") {
        handleInitialize(seq);
    } else if (command == "launch") {
        // Extract program path
        size_t program_pos = message.find("\"program\":\"");
        std::string program;
        if (program_pos != std::string::npos) {
            program_pos += 11;
            size_t program_end = message.find("\"", program_pos);
            program = message.substr(program_pos, program_end - program_pos);
        }
        handleLaunch(seq, program);
    } else if (command == "setBreakpoints") {
        // Extract file path
        size_t file_pos = message.find("\"path\":\"");
        std::string file;
        if (file_pos != std::string::npos) {
            file_pos += 8;
            size_t file_end = message.find("\"", file_pos);
            file = message.substr(file_pos, file_end - file_pos);
        }
        
        // Extract line numbers (simplified parsing)
        std::vector<int> lines;
        size_t lines_pos = message.find("\"breakpoints\":[");
        if (lines_pos != std::string::npos) {
            lines_pos += 15;
            size_t lines_end = message.find("]", lines_pos);
            std::string lines_str = message.substr(lines_pos, lines_end - lines_pos);
            
            // Parse line numbers
            std::istringstream iss(lines_str);
            std::string token;
            while (std::getline(iss, token, ',')) {
                size_t line_pos = token.find("\"line\":");
                if (line_pos != std::string::npos) {
                    line_pos += 7;
                    size_t line_end = token.find_first_of(",}", line_pos);
                    int line = std::stoi(token.substr(line_pos, line_end - line_pos));
                    lines.push_back(line);
                }
            }
        }
        
        handleSetBreakpoints(seq, file, lines);
    } else if (command == "configurationDone") {
        handleConfigurationDone(seq);
    } else if (command == "threads") {
        handleThreads(seq);
    } else if (command == "stackTrace") {
        handleStackTrace(seq, 1);
    } else if (command == "scopes") {
        handleScopes(seq, 1);
    } else if (command == "variables") {
        handleVariables(seq, 1);
    } else if (command == "continue") {
        handleContinue(seq);
    } else if (command == "next") {
        handleNext(seq);
    } else if (command == "stepIn") {
        handleStepIn(seq);
    } else if (command == "stepOut") {
        handleStepOut(seq);
    } else if (command == "pause") {
        handlePause(seq);
    } else if (command == "disconnect") {
        handleDisconnect(seq);
    }
}

bool DAPServer::hasBreakpoint(const std::string& file, int line) const {
    auto it = breakpoints_by_file_.find(file);
    if (it == breakpoints_by_file_.end()) {
        return false;
    }
    
    for (int bp_id : it->second) {
        const auto& bp = breakpoints_.at(bp_id);
        if (bp.line == line) {
            return true;
        }
    }
    
    return false;
}

void DAPServer::notifyBreakpoint(const std::string& file, int line, const std::string& reason) {
    std::string body = R"({
        "reason":")" + reason + R"(",
        "threadId":1,
        "allThreadsStopped":true
    })";
    sendEvent("stopped", body);
}

} // namespace debug
} // namespace sapphire

int main() {
    sapphire::debug::DAPServer server;
    server.run();
    return 0;
}
