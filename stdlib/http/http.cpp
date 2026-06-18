#include "http.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <regex>
#include <cstring>
#include <iomanip>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/select.h>

namespace Sapphire {
namespace HTTP {

// HTTPRequest Implementation
HTTPRequest::HTTPRequest() : method_(Method::GET) {}

HTTPRequest::~HTTPRequest() {}

std::string HTTPRequest::get_header(const std::string& name) const {
    auto it = headers_.find(name);
    return (it != headers_.end()) ? it->second : "";
}

bool HTTPRequest::has_header(const std::string& name) const {
    return headers_.find(name) != headers_.end();
}

std::string HTTPRequest::get_param(const std::string& name) const {
    auto it = params_.find(name);
    return (it != params_.end()) ? it->second : "";
}

bool HTTPRequest::has_param(const std::string& name) const {
    return params_.find(name) != params_.end();
}

void HTTPRequest::set_header(const std::string& name, const std::string& value) {
    headers_[name] = value;
}

bool HTTPRequest::parse(const std::string& raw_request) {
    std::istringstream stream(raw_request);
    std::string line;
    
    // Parse request line
    if (!std::getline(stream, line)) return false;
    
    std::istringstream request_line(line);
    std::string method_str, path_query, version;
    if (!(request_line >> method_str >> path_query >> version)) return false;
    
    method_ = string_to_method(method_str);
    
    // Split path and query
    size_t query_pos = path_query.find('?');
    if (query_pos != std::string::npos) {
        path_ = path_query.substr(0, query_pos);
        query_ = path_query.substr(query_pos + 1);
        parse_query_params();
    } else {
        path_ = path_query;
        query_.clear();
    }
    
    // Parse headers
    while (std::getline(stream, line) && !line.empty() && line != "\r") {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string name = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            
            // Trim whitespace
            name.erase(0, name.find_first_not_of(" \t"));
            name.erase(name.find_last_not_of(" \t\r") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r") + 1);
            
            headers_[name] = value;
        }
    }
    
    // Parse body
    std::string body_line;
    while (std::getline(stream, body_line)) {
        body_ += body_line + "\n";
    }
    if (!body_.empty()) {
        body_.pop_back(); // Remove last newline
    }
    
    return true;
}

std::string HTTPRequest::to_string() const {
    std::ostringstream oss;
    oss << method_to_string(method_) << " " << path_;
    if (!query_.empty()) {
        oss << "?" << query_;
    }
    oss << " HTTP/1.1\r\n";
    
    for (const auto& header : headers_) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    
    oss << "\r\n" << body_;
    return oss.str();
}

void HTTPRequest::parse_query_params() {
    params_.clear();
    if (query_.empty()) return;
    
    std::istringstream stream(query_);
    std::string param;
    
    while (std::getline(stream, param, '&')) {
        size_t eq_pos = param.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = url_decode(param.substr(0, eq_pos));
            std::string value = url_decode(param.substr(eq_pos + 1));
            params_[key] = value;
        }
    }
}

std::string HTTPRequest::method_to_string(Method method) const {
    switch (method) {
        case Method::GET: return "GET";
        case Method::POST: return "POST";
        case Method::PUT: return "PUT";
        case Method::DELETE: return "DELETE";
        case Method::PATCH: return "PATCH";
        case Method::HEAD: return "HEAD";
        case Method::OPTIONS: return "OPTIONS";
        default: return "GET";
    }
}

Method HTTPRequest::string_to_method(const std::string& method) const {
    if (method == "GET") return Method::GET;
    if (method == "POST") return Method::POST;
    if (method == "PUT") return Method::PUT;
    if (method == "DELETE") return Method::DELETE;
    if (method == "PATCH") return Method::PATCH;
    if (method == "HEAD") return Method::HEAD;
    if (method == "OPTIONS") return Method::OPTIONS;
    return Method::GET;
}

// HTTPResponse Implementation
HTTPResponse::HTTPResponse() : status_(Status::OK) {}

HTTPResponse::~HTTPResponse() {}

std::string HTTPResponse::get_header(const std::string& name) const {
    auto it = headers_.find(name);
    return (it != headers_.end()) ? it->second : "";
}

bool HTTPResponse::has_header(const std::string& name) const {
    return headers_.find(name) != headers_.end();
}

void HTTPResponse::set_header(const std::string& name, const std::string& value) {
    headers_[name] = value;
}

void HTTPResponse::set_json(const std::string& json) {
    set_body(json);
    set_header("Content-Type", "application/json");
}

void HTTPResponse::set_html(const std::string& html) {
    set_body(html);
    set_header("Content-Type", "text/html; charset=utf-8");
}

void HTTPResponse::set_text(const std::string& text) {
    set_body(text);
    set_header("Content-Type", "text/plain; charset=utf-8");
}

bool HTTPResponse::parse(const std::string& raw_response) {
    std::istringstream stream(raw_response);
    std::string line;
    
    // Parse status line
    if (!std::getline(stream, line)) return false;
    
    std::istringstream status_line(line);
    std::string version;
    int status_code;
    if (!(status_line >> version >> status_code)) return false;
    
    status_ = string_to_status(status_code);
    
    // Parse headers
    while (std::getline(stream, line) && !line.empty() && line != "\r") {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string name = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            
            // Trim whitespace
            name.erase(0, name.find_first_not_of(" \t"));
            name.erase(name.find_last_not_of(" \t\r") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r") + 1);
            
            headers_[name] = value;
        }
    }
    
    // Parse body
    std::string body_line;
    while (std::getline(stream, body_line)) {
        body_ += body_line + "\n";
    }
    if (!body_.empty()) {
        body_.pop_back(); // Remove last newline
    }
    
    return true;
}

std::string HTTPResponse::to_string() const {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << static_cast<int>(status_) << " " << status_to_string(status_) << "\r\n";
    
    // Add Content-Length if not present
    if (headers_.find("Content-Length") == headers_.end()) {
        oss << "Content-Length: " << body_.length() << "\r\n";
    }
    
    for (const auto& header : headers_) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    
    oss << "\r\n" << body_;
    return oss.str();
}

std::string HTTPResponse::status_to_string(Status status) const {
    switch (status) {
        case Status::OK: return "OK";
        case Status::CREATED: return "Created";
        case Status::NO_CONTENT: return "No Content";
        case Status::BAD_REQUEST: return "Bad Request";
        case Status::UNAUTHORIZED: return "Unauthorized";
        case Status::FORBIDDEN: return "Forbidden";
        case Status::NOT_FOUND: return "Not Found";
        case Status::METHOD_NOT_ALLOWED: return "Method Not Allowed";
        case Status::INTERNAL_SERVER_ERROR: return "Internal Server Error";
        case Status::NOT_IMPLEMENTED: return "Not Implemented";
        case Status::SERVICE_UNAVAILABLE: return "Service Unavailable";
        default: return "Unknown";
    }
}

Status HTTPResponse::string_to_status(int code) const {
    switch (code) {
        case 200: return Status::OK;
        case 201: return Status::CREATED;
        case 204: return Status::NO_CONTENT;
        case 400: return Status::BAD_REQUEST;
        case 401: return Status::UNAUTHORIZED;
        case 403: return Status::FORBIDDEN;
        case 404: return Status::NOT_FOUND;
        case 405: return Status::METHOD_NOT_ALLOWED;
        case 500: return Status::INTERNAL_SERVER_ERROR;
        case 501: return Status::NOT_IMPLEMENTED;
        case 503: return Status::SERVICE_UNAVAILABLE;
        default: return Status::OK;
    }
}

// HTTPServer Implementation
HTTPServer::HTTPServer(int port) 
    : port_(port), server_fd_(-1), running_(false), max_connections_(100), timeout_ms_(30000) {}

HTTPServer::~HTTPServer() {
    stop();
}

void HTTPServer::get(const std::string& path, RouteHandler handler) {
    route(Method::GET, path, handler);
}

void HTTPServer::post(const std::string& path, RouteHandler handler) {
    route(Method::POST, path, handler);
}

void HTTPServer::put(const std::string& path, RouteHandler handler) {
    route(Method::PUT, path, handler);
}

void HTTPServer::delete_(const std::string& path, RouteHandler handler) {
    route(Method::DELETE, path, handler);
}

void HTTPServer::route(Method method, const std::string& path, RouteHandler handler) {
    routes_.push_back({method, path, handler});
}

void HTTPServer::use(Middleware middleware) {
    middlewares_.push_back(middleware);
}

void HTTPServer::serve_static(const std::string& path, const std::string& directory) {
    static_routes_[path] = directory;
}

bool HTTPServer::start() {
    if (running_) return false;
    
    // Create socket
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set socket options" << std::endl;
        close(server_fd_);
        return false;
    }
    
    // Bind socket
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);
    
    if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Failed to bind socket to port " << port_ << std::endl;
        close(server_fd_);
        return false;
    }
    
    // Listen
    if (listen(server_fd_, max_connections_) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(server_fd_);
        return false;
    }
    
    running_ = true;
    
    // Start server thread
    worker_threads_.emplace_back(&HTTPServer::server_loop, this);
    
    std::cout << "HTTP Server started on port " << port_ << std::endl;
    return true;
}

void HTTPServer::stop() {
    if (!running_) return;
    
    running_ = false;
    
    if (server_fd_ >= 0) {
        close(server_fd_);
        server_fd_ = -1;
    }
    
    // Wait for threads to finish
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    worker_threads_.clear();
    
    std::cout << "HTTP Server stopped" << std::endl;
}

void HTTPServer::server_loop() {
    while (running_) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        // Set socket to non-blocking for accept
        int flags = fcntl(server_fd_, F_GETFL, 0);
        fcntl(server_fd_, F_SETFL, flags | O_NONBLOCK);
        
        int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No connection available, sleep briefly
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            } else if (running_) {
                std::cerr << "Failed to accept client connection" << std::endl;
            }
            continue;
        }
        
        // Reset to blocking for client handling
        fcntl(client_fd, F_SETFL, flags);
        
        // Handle client in separate thread
        std::thread client_thread(&HTTPServer::handle_client, this, client_fd);
        client_thread.detach();
    }
}

void HTTPServer::handle_client(int client_fd) {
    char buffer[8192];
    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }
    
    buffer[bytes_read] = '\0';
    std::string raw_request(buffer);
    
    HTTPRequest request;
    HTTPResponse response;
    
    if (!request.parse(raw_request)) {
        response.set_status(Status::BAD_REQUEST);
        response.set_text("Bad Request");
        send_response(client_fd, response);
        close(client_fd);
        return;
    }
    
    // Run middlewares
    if (!run_middlewares(request, response)) {
        send_response(client_fd, response);
        close(client_fd);
        return;
    }
    
    // Find matching route
    RouteHandler handler;
    if (match_route(request, handler)) {
        try {
            handler(request, response);
        } catch (const std::exception& e) {
            response.set_status(Status::INTERNAL_SERVER_ERROR);
            response.set_text("Internal Server Error");
        }
    } else {
        // Check static files
        bool served_static = false;
        for (const auto& static_route : static_routes_) {
            if (request.get_path().find(static_route.first) == 0) {
                std::string file_path = static_route.second + 
                    request.get_path().substr(static_route.first.length());
                std::string content = read_static_file(file_path);
                if (!content.empty()) {
                    response.set_body(content);
                    response.set_header("Content-Type", get_mime_type(file_path));
                    served_static = true;
                    break;
                }
            }
        }
        
        if (!served_static) {
            response.set_status(Status::NOT_FOUND);
            response.set_text("Not Found");
        }
    }
    
    send_response(client_fd, response);
    close(client_fd);
}

bool HTTPServer::match_route(const HTTPRequest& request, RouteHandler& handler) {
    for (const auto& route : routes_) {
        if (route.method == request.get_method() && route.path == request.get_path()) {
            handler = route.handler;
            return true;
        }
    }
    return false;
}

bool HTTPServer::run_middlewares(const HTTPRequest& request, HTTPResponse& response) {
    for (const auto& middleware : middlewares_) {
        if (!middleware(request, response)) {
            return false;
        }
    }
    return true;
}

void HTTPServer::send_response(int client_fd, const HTTPResponse& response) {
    std::string response_str = response.to_string();
    send(client_fd, response_str.c_str(), response_str.length(), 0);
}

std::string HTTPServer::read_static_file(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

// HTTPClient Implementation
HTTPClient::HTTPClient() : timeout_ms_(30000), ssl_enabled_(false) {
    set_user_agent("Sapphire-HTTP-Client/1.0");
}

HTTPClient::~HTTPClient() {}

HTTPResponse HTTPClient::get(const std::string& url) {
    return request(Method::GET, url);
}

HTTPResponse HTTPClient::post(const std::string& url, const std::string& body) {
    return request(Method::POST, url, body);
}

HTTPResponse HTTPClient::put(const std::string& url, const std::string& body) {
    return request(Method::PUT, url, body);
}

HTTPResponse HTTPClient::delete_(const std::string& url) {
    return request(Method::DELETE, url);
}

HTTPResponse HTTPClient::request(Method method, const std::string& url, const std::string& body) {
    HTTPResponse response;
    
    std::string host;
    int port;
    std::string path;
    
    if (!parse_url(url, host, port, path)) {
        response.set_status(Status::BAD_REQUEST);
        response.set_text("Invalid URL");
        return response;
    }
    
    HTTPRequest request;
    request.set_method(method);
    request.set_path(path);
    request.set_body(body);
    
    // Set default headers
    for (const auto& header : default_headers_) {
        request.set_header(header.first, header.second);
    }
    
    request.set_header("Host", host);
    if (!body.empty()) {
        request.set_header("Content-Length", std::to_string(body.length()));
    }
    
    return send_request(request, host, port);
}

void HTTPClient::set_header(const std::string& name, const std::string& value) {
    default_headers_[name] = value;
}

void HTTPClient::set_user_agent(const std::string& user_agent) {
    set_header("User-Agent", user_agent);
}

HTTPResponse HTTPClient::send_request(const HTTPRequest& request, const std::string& host, int port) {
    HTTPResponse response;
    
    int sock_fd = connect_to_host(host, port);
    if (sock_fd < 0) {
        response.set_status(Status::SERVICE_UNAVAILABLE);
        response.set_text("Connection failed");
        return response;
    }
    
    // Send request
    std::string request_str = request.to_string();
    if (send(sock_fd, request_str.c_str(), request_str.length(), 0) < 0) {
        response.set_status(Status::SERVICE_UNAVAILABLE);
        response.set_text("Send failed");
        close(sock_fd);
        return response;
    }
    
    // Receive response
    char buffer[8192];
    std::string raw_response;
    ssize_t bytes_received;
    
    while ((bytes_received = recv(sock_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        raw_response += buffer;
        
        // Check if we have complete response (simple check)
        if (raw_response.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }
    
    close(sock_fd);
    
    if (!response.parse(raw_response)) {
        response.set_status(Status::INTERNAL_SERVER_ERROR);
        response.set_text("Failed to parse response");
    }
    
    return response;
}

bool HTTPClient::parse_url(const std::string& url, std::string& host, int& port, std::string& path) {
    std::regex url_regex(R"(^https?://([^:/]+)(?::(\d+))?(/.*)?$)");
    std::smatch matches;
    
    if (!std::regex_match(url, matches, url_regex)) {
        return false;
    }
    
    host = matches[1].str();
    port = matches[2].matched ? std::stoi(matches[2].str()) : 
           (url.substr(0, 5) == "https" ? 443 : 80);
    path = matches[3].matched ? matches[3].str() : "/";
    
    return true;
}

int HTTPClient::connect_to_host(const std::string& host, int port) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        return -1;
    }
    
    struct hostent* server = gethostbyname(host.c_str());
    if (!server) {
        close(sock_fd);
        return -1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock_fd);
        return -1;
    }
    
    return sock_fd;
}

// Utility Functions
std::string url_encode(const std::string& str) {
    std::ostringstream encoded;
    
    for (unsigned char c : str) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else {
            encoded << '%';
            encoded << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(c);
        }
    }
    
    return encoded.str();
}

std::string url_decode(const std::string& str) {
    std::ostringstream decoded;
    
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            std::string hex = str.substr(i + 1, 2);
            try {
                int value = std::stoi(hex, nullptr, 16);
                decoded << static_cast<char>(value);
                i += 2;
            } catch (...) {
                // Invalid hex, just add the % character
                decoded << str[i];
            }
        } else if (str[i] == '+') {
            decoded << ' ';
        } else {
            decoded << str[i];
        }
    }
    
    return decoded.str();
}

std::string html_escape(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '<': escaped += "&lt;"; break;
            case '>': escaped += "&gt;"; break;
            case '&': escaped += "&amp;"; break;
            case '"': escaped += "&quot;"; break;
            case '\'': escaped += "&#39;"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

std::string get_mime_type(const std::string& filename) {
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return "application/octet-stream";
    }
    
    std::string ext = filename.substr(dot_pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == "html" || ext == "htm") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "json") return "application/json";
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "gif") return "image/gif";
    if (ext == "svg") return "image/svg+xml";
    if (ext == "txt") return "text/plain";
    if (ext == "pdf") return "application/pdf";
    
    return "application/octet-stream";
}

} // namespace HTTP
} // namespace Sapphire

// C API Implementation
extern "C" {
    using namespace Sapphire::HTTP;
    
    void* http_server_create(int port) {
        return new HTTPServer(port);
    }
    
    void http_server_destroy(void* server) {
        delete static_cast<HTTPServer*>(server);
    }
    
    void http_server_get(void* server, const char* path, void* /* handler */) {
        // Note: C API handler would need wrapper - simplified for now
        auto* srv = static_cast<HTTPServer*>(server);
        srv->get(path, [](const HTTPRequest& /* req */, HTTPResponse& res) {
            res.set_text("Hello from C API");
        });
    }
    
    void http_server_post(void* server, const char* path, void* /* handler */) {
        auto* srv = static_cast<HTTPServer*>(server);
        srv->post(path, [](const HTTPRequest& /* req */, HTTPResponse& res) {
            res.set_text("POST response from C API");
        });
    }
    
    void http_server_put(void* server, const char* path, void* /* handler */) {
        auto* srv = static_cast<HTTPServer*>(server);
        srv->put(path, [](const HTTPRequest& /* req */, HTTPResponse& res) {
            res.set_text("PUT response from C API");
        });
    }
    
    void http_server_delete(void* server, const char* path, void* /* handler */) {
        auto* srv = static_cast<HTTPServer*>(server);
        srv->delete_(path, [](const HTTPRequest& /* req */, HTTPResponse& res) {
            res.set_text("DELETE response from C API");
        });
    }
    
    int http_server_start(void* server) {
        return static_cast<HTTPServer*>(server)->start() ? 1 : 0;
    }
    
    void http_server_stop(void* server) {
        static_cast<HTTPServer*>(server)->stop();
    }
    
    void* http_client_create() {
        return new HTTPClient();
    }
    
    void http_client_destroy(void* client) {
        delete static_cast<HTTPClient*>(client);
    }
    
    void* http_client_get(void* client, const char* url) {
        auto response = static_cast<HTTPClient*>(client)->get(url);
        return new HTTPResponse(std::move(response));
    }
    
    void* http_client_post(void* client, const char* url, const char* body) {
        auto response = static_cast<HTTPClient*>(client)->post(url, body ? body : "");
        return new HTTPResponse(std::move(response));
    }
    
    int http_response_get_status(void* response) {
        return static_cast<int>(static_cast<HTTPResponse*>(response)->get_status());
    }
    
    const char* http_response_get_body(void* response) {
        static std::string body = static_cast<HTTPResponse*>(response)->get_body();
        return body.c_str();
    }
    
    const char* http_response_get_header(void* response, const char* name) {
        static std::string header = static_cast<HTTPResponse*>(response)->get_header(name);
        return header.c_str();
    }
    
    void http_response_destroy(void* response) {
        delete static_cast<HTTPResponse*>(response);
    }
    
    char* http_url_encode(const char* str) {
        std::string encoded = url_encode(str);
        char* result = new char[encoded.length() + 1];
        strcpy(result, encoded.c_str());
        return result;
    }
    
    char* http_url_decode(const char* str) {
        std::string decoded = url_decode(str);
        char* result = new char[decoded.length() + 1];
        strcpy(result, decoded.c_str());
        return result;
    }
    
    char* http_html_escape(const char* str) {
        std::string escaped = html_escape(str);
        char* result = new char[escaped.length() + 1];
        strcpy(result, escaped.c_str());
        return result;
    }
    
    void http_free_string(char* str) {
        delete[] str;
    }
}