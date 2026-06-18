#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>

namespace Sapphire {
namespace HTTP {

// Forward declarations
class HTTPRequest;
class HTTPResponse;
class HTTPServer;
class HTTPClient;

// HTTP Methods
enum class Method {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS
};

// HTTP Status Codes
enum class Status {
    OK = 200,
    CREATED = 201,
    NO_CONTENT = 204,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    SERVICE_UNAVAILABLE = 503
};

// HTTP Headers
using Headers = std::unordered_map<std::string, std::string>;

// HTTP Request
class HTTPRequest {
public:
    HTTPRequest();
    ~HTTPRequest();

    // Getters
    Method get_method() const { return method_; }
    const std::string& get_path() const { return path_; }
    const std::string& get_query() const { return query_; }
    const std::string& get_body() const { return body_; }
    const Headers& get_headers() const { return headers_; }
    
    // Header operations
    std::string get_header(const std::string& name) const;
    bool has_header(const std::string& name) const;
    
    // Query parameters
    std::string get_param(const std::string& name) const;
    bool has_param(const std::string& name) const;
    
    // Setters (for building requests)
    void set_method(Method method) { method_ = method; }
    void set_path(const std::string& path) { path_ = path; }
    void set_query(const std::string& query) { query_ = query; }
    void set_body(const std::string& body) { body_ = body; }
    void set_header(const std::string& name, const std::string& value);
    
    // Parsing
    bool parse(const std::string& raw_request);
    std::string to_string() const;

private:
    Method method_;
    std::string path_;
    std::string query_;
    std::string body_;
    Headers headers_;
    std::unordered_map<std::string, std::string> params_;
    
    void parse_query_params();
    std::string method_to_string(Method method) const;
    Method string_to_method(const std::string& method) const;
};

// HTTP Response
class HTTPResponse {
public:
    HTTPResponse();
    ~HTTPResponse();

    // Getters
    Status get_status() const { return status_; }
    const std::string& get_body() const { return body_; }
    const Headers& get_headers() const { return headers_; }
    
    // Header operations
    std::string get_header(const std::string& name) const;
    bool has_header(const std::string& name) const;
    
    // Setters
    void set_status(Status status) { status_ = status; }
    void set_body(const std::string& body) { body_ = body; }
    void set_header(const std::string& name, const std::string& value);
    
    // Convenience methods
    void set_json(const std::string& json);
    void set_html(const std::string& html);
    void set_text(const std::string& text);
    
    // Parsing and serialization
    bool parse(const std::string& raw_response);
    std::string to_string() const;

private:
    Status status_;
    std::string body_;
    Headers headers_;
    
    std::string status_to_string(Status status) const;
    Status string_to_status(int code) const;
};

// Route Handler
using RouteHandler = std::function<void(const HTTPRequest&, HTTPResponse&)>;

// Middleware
using Middleware = std::function<bool(const HTTPRequest&, HTTPResponse&)>;

// HTTP Server
class HTTPServer {
public:
    HTTPServer(int port = 8080);
    ~HTTPServer();

    // Route registration
    void get(const std::string& path, RouteHandler handler);
    void post(const std::string& path, RouteHandler handler);
    void put(const std::string& path, RouteHandler handler);
    void delete_(const std::string& path, RouteHandler handler);
    void route(Method method, const std::string& path, RouteHandler handler);
    
    // Middleware
    void use(Middleware middleware);
    
    // Static file serving
    void serve_static(const std::string& path, const std::string& directory);
    
    // Server control
    bool start();
    void stop();
    bool is_running() const { return running_; }
    
    // Configuration
    void set_port(int port) { port_ = port; }
    void set_max_connections(int max_conn) { max_connections_ = max_conn; }
    void set_timeout(int timeout_ms) { timeout_ms_ = timeout_ms; }

private:
    struct Route {
        Method method;
        std::string path;
        RouteHandler handler;
    };
    
    int port_;
    int server_fd_;
    std::atomic<bool> running_;
    std::vector<std::thread> worker_threads_;
    std::vector<Route> routes_;
    std::vector<Middleware> middlewares_;
    std::unordered_map<std::string, std::string> static_routes_;
    
    // Configuration
    int max_connections_;
    int timeout_ms_;
    
    void server_loop();
    void handle_client(int client_fd);
    bool match_route(const HTTPRequest& request, RouteHandler& handler);
    bool run_middlewares(const HTTPRequest& request, HTTPResponse& response);
    void send_response(int client_fd, const HTTPResponse& response);
    std::string read_static_file(const std::string& filepath);
};

// HTTP Client
class HTTPClient {
public:
    HTTPClient();
    ~HTTPClient();

    // HTTP methods
    HTTPResponse get(const std::string& url);
    HTTPResponse post(const std::string& url, const std::string& body = "");
    HTTPResponse put(const std::string& url, const std::string& body = "");
    HTTPResponse delete_(const std::string& url);
    HTTPResponse request(Method method, const std::string& url, const std::string& body = "");
    
    // Configuration
    void set_timeout(int timeout_ms) { timeout_ms_ = timeout_ms; }
    void set_header(const std::string& name, const std::string& value);
    void set_user_agent(const std::string& user_agent);
    
    // SSL/TLS support
    void enable_ssl(bool enable = true) { ssl_enabled_ = enable; }

private:
    Headers default_headers_;
    int timeout_ms_;
    bool ssl_enabled_;
    
    HTTPResponse send_request(const HTTPRequest& request, const std::string& host, int port);
    bool parse_url(const std::string& url, std::string& host, int& port, std::string& path);
    int connect_to_host(const std::string& host, int port);
};

// Utility functions
std::string url_encode(const std::string& str);
std::string url_decode(const std::string& str);
std::string html_escape(const std::string& str);
std::string get_mime_type(const std::string& filename);

} // namespace HTTP
} // namespace Sapphire

// C API for compiler integration
extern "C" {
    // Server functions
    void* http_server_create(int port);
    void http_server_destroy(void* server);
    void http_server_get(void* server, const char* path, void* handler);
    void http_server_post(void* server, const char* path, void* handler);
    void http_server_put(void* server, const char* path, void* handler);
    void http_server_delete(void* server, const char* path, void* handler);
    int http_server_start(void* server);
    void http_server_stop(void* server);
    
    // Client functions
    void* http_client_create();
    void http_client_destroy(void* client);
    void* http_client_get(void* client, const char* url);
    void* http_client_post(void* client, const char* url, const char* body);
    
    // Response functions
    int http_response_get_status(void* response);
    const char* http_response_get_body(void* response);
    const char* http_response_get_header(void* response, const char* name);
    void http_response_destroy(void* response);
    
    // Utility functions
    char* http_url_encode(const char* str);
    char* http_url_decode(const char* str);
    char* http_html_escape(const char* str);
    void http_free_string(char* str);
}