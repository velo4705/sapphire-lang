#ifndef SAPPHIRE_DAP_SERVER_H
#define SAPPHIRE_DAP_SERVER_H

#include <string>
#include <map>
#include <vector>
#include <set>
#include <memory>
#include <functional>

namespace sapphire {
namespace debug {

/**
 * Breakpoint information
 */
struct Breakpoint {
    std::string file;
    int line;
    bool verified;
    int id;
    
    Breakpoint() : line(0), verified(false), id(0) {}
    Breakpoint(const std::string& f, int l, int i)
        : file(f), line(l), verified(false), id(i) {}
};

/**
 * Stack frame information
 */
struct StackFrame {
    int id;
    std::string name;
    std::string file;
    int line;
    int column;
    
    StackFrame(int i, const std::string& n, const std::string& f, int l, int c)
        : id(i), name(n), file(f), line(l), column(c) {}
};

/**
 * Variable information
 */
struct Variable {
    std::string name;
    std::string value;
    std::string type;
    int variablesReference;
    
    Variable(const std::string& n, const std::string& v, const std::string& t, int ref = 0)
        : name(n), value(v), type(t), variablesReference(ref) {}
};

/**
 * Debug state
 */
enum class DebugState {
    STOPPED,
    RUNNING,
    PAUSED,
    STEP_IN,
    STEP_OVER,
    STEP_OUT
};

/**
 * Debug event callback
 */
using DebugEventCallback = std::function<void(const std::string& event, const std::string& data)>;

/**
 * Debug Adapter Protocol Server
 */
class DAPServer {
private:
    bool running_;
    int next_breakpoint_id_;
    int next_seq_;
    std::map<int, Breakpoint> breakpoints_;
    std::map<std::string, std::set<int>> breakpoints_by_file_;
    DebugState state_;
    DebugEventCallback event_callback_;
    
    // Send JSON-RPC response
    void sendResponse(const std::string& request_seq, const std::string& command, 
                     const std::string& body);
    
    // Send event
    void sendEvent(const std::string& event, const std::string& body);
    
    // Handle different DAP requests
    void handleInitialize(const std::string& seq);
    void handleLaunch(const std::string& seq, const std::string& program);
    void handleSetBreakpoints(const std::string& seq, const std::string& file, 
                             const std::vector<int>& lines);
    void handleConfigurationDone(const std::string& seq);
    void handleThreads(const std::string& seq);
    void handleStackTrace(const std::string& seq, int threadId);
    void handleScopes(const std::string& seq, int frameId);
    void handleVariables(const std::string& seq, int variablesReference);
    void handleContinue(const std::string& seq);
    void handleNext(const std::string& seq);
    void handleStepIn(const std::string& seq);
    void handleStepOut(const std::string& seq);
    void handlePause(const std::string& seq);
    void handleDisconnect(const std::string& seq);
    
public:
    DAPServer();
    
    // Run the DAP server (reads from stdin, writes to stdout)
    void run();
    
    // Handle a message from the client
    void handleMessage(const std::string& message);
    
    // Check if there's a breakpoint at the given location
    bool hasBreakpoint(const std::string& file, int line) const;
    
    // Get current debug state
    DebugState getState() const { return state_; }
    
    // Set debug state
    void setState(DebugState state) { state_ = state; }
    
    // Set event callback
    void setEventCallback(DebugEventCallback callback) { event_callback_ = callback; }
    
    // Notify stopped at breakpoint
    void notifyBreakpoint(const std::string& file, int line, const std::string& reason);
    
    // Get all breakpoints
    const std::map<int, Breakpoint>& getBreakpoints() const { return breakpoints_; }
};

} // namespace debug
} // namespace sapphire

#endif // SAPPHIRE_DAP_SERVER_H
