#pragma once

#ifndef CROW_H
#define CROW_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <sstream>
#include <thread>
#include <mutex>
#include <regex>
#include <algorithm>
#include <fstream>
#include <cctype>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #ifdef _MSC_VER
    #pragma comment(lib, "ws2_32.lib")
    #endif
    typedef SOCKET socket_t;
    #define CLOSE_SOCKET(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define CLOSE_SOCKET(s) close(s)
#endif

namespace crow {

    // HTTP Methods
    enum class HTTPMethod {
        GET,
        POST,
        PUT,
        DELETE_METHOD,
        OPTIONS,
        HEAD,
        PATCH,
        UNKNOWN
    };

    inline HTTPMethod string_to_method(const std::string& method) {
        if (method == "GET") return HTTPMethod::GET;
        if (method == "POST") return HTTPMethod::POST;
        if (method == "PUT") return HTTPMethod::PUT;
        if (method == "DELETE") return HTTPMethod::DELETE_METHOD;
        if (method == "OPTIONS") return HTTPMethod::OPTIONS;
        if (method == "HEAD") return HTTPMethod::HEAD;
        if (method == "PATCH") return HTTPMethod::PATCH;
        return HTTPMethod::UNKNOWN;
    }

    inline std::string method_to_string(HTTPMethod method) {
        switch (method) {
            case HTTPMethod::GET: return "GET";
            case HTTPMethod::POST: return "POST";
            case HTTPMethod::PUT: return "PUT";
            case HTTPMethod::DELETE_METHOD: return "DELETE";
            case HTTPMethod::OPTIONS: return "OPTIONS";
            case HTTPMethod::HEAD: return "HEAD";
            case HTTPMethod::PATCH: return "PATCH";
            default: return "UNKNOWN";
        }
    }

    // Lightweight JSON Parser and Builder
    namespace json {

        enum class Type {
            Null,
            Boolean,
            Number,
            String,
            Array,
            Object
        };

        class rvalue;

        class wvalue {
        public:
            Type type = Type::Null;
            bool bool_val = false;
            double num_val = 0.0;
            std::string str_val;
            std::vector<wvalue> arr_val;
            std::vector<std::pair<std::string, wvalue>> obj_val;

            wvalue() : type(Type::Null) {}
            wvalue(std::nullptr_t) : type(Type::Null) {}
            wvalue(bool b) : type(Type::Boolean), bool_val(b) {}
            wvalue(int n) : type(Type::Number), num_val(n) {}
            wvalue(long long n) : type(Type::Number), num_val(static_cast<double>(n)) {}
            wvalue(size_t n) : type(Type::Number), num_val(static_cast<double>(n)) {}
            wvalue(double d) : type(Type::Number), num_val(d) {}
            wvalue(const char* s) : type(Type::String), str_val(s ? s : "") {}
            wvalue(const std::string& s) : type(Type::String), str_val(s) {}
            wvalue(const std::vector<wvalue>& arr) : type(Type::Array), arr_val(arr) {}

            wvalue& operator[](const std::string& key) {
                if (type != Type::Object) {
                    type = Type::Object;
                    obj_val.clear();
                }
                for (auto& p : obj_val) {
                    if (p.first == key) return p.second;
                }
                obj_val.push_back({key, wvalue()});
                return obj_val.back().second;
            }

            const wvalue& operator[](const std::string& key) const {
                static wvalue null_val;
                if (type != Type::Object) return null_val;
                for (const auto& p : obj_val) {
                    if (p.first == key) return p.second;
                }
                return null_val;
            }

            wvalue& operator[](size_t index) {
                if (type != Type::Array) {
                    type = Type::Array;
                    arr_val.clear();
                }
                if (index >= arr_val.size()) {
                    arr_val.resize(index + 1);
                }
                return arr_val[index];
            }

            const wvalue& operator[](size_t index) const {
                static wvalue null_val;
                if (type != Type::Array || index >= arr_val.size()) return null_val;
                return arr_val[index];
            }

            void push_back(const wvalue& val) {
                if (type != Type::Array) {
                    type = Type::Array;
                    arr_val.clear();
                }
                arr_val.push_back(val);
            }

            size_t size() const {
                if (type == Type::Array) return arr_val.size();
                if (type == Type::Object) return obj_val.size();
                return 0;
            }

            std::string dump() const {
                switch (type) {
                    case Type::Null: return "null";
                    case Type::Boolean: return bool_val ? "true" : "false";
                    case Type::Number: {
                        std::ostringstream ss;
                        if (num_val == static_cast<long long>(num_val)) {
                            ss << static_cast<long long>(num_val);
                        } else {
                            ss << num_val;
                        }
                        return ss.str();
                    }
                    case Type::String: {
                        std::ostringstream ss;
                        ss << "\"";
                        for (char c : str_val) {
                            if (c == '"') ss << "\\\"";
                            else if (c == '\\') ss << "\\\\";
                            else if (c == '\b') ss << "\\b";
                            else if (c == '\f') ss << "\\f";
                            else if (c == '\n') ss << "\\n";
                            else if (c == '\r') ss << "\\r";
                            else if (c == '\t') ss << "\\t";
                            else ss << c;
                        }
                        ss << "\"";
                        return ss.str();
                    }
                    case Type::Array: {
                        std::string res = "[";
                        for (size_t i = 0; i < arr_val.size(); ++i) {
                            if (i > 0) res += ",";
                            res += arr_val[i].dump();
                        }
                        res += "]";
                        return res;
                    }
                    case Type::Object: {
                        std::string res = "{";
                        for (size_t i = 0; i < obj_val.size(); ++i) {
                            if (i > 0) res += ",";
                            res += "\"" + obj_val[i].first + "\":" + obj_val[i].second.dump();
                        }
                        res += "}";
                        return res;
                    }
                }
                return "null";
            }
        };

        class rvalue {
        public:
            Type type = Type::Null;
            bool bool_val = false;
            double num_val = 0.0;
            std::string str_val;
            std::vector<rvalue> arr_val;
            std::unordered_map<std::string, rvalue> obj_val;

            rvalue() : type(Type::Null) {}
            explicit operator bool() const { return type != Type::Null; }

            bool has(const std::string& key) const {
                if (type != Type::Object) return false;
                return obj_val.find(key) != obj_val.end();
            }

            const rvalue& operator[](const std::string& key) const {
                static rvalue null_val;
                if (type != Type::Object) return null_val;
                auto it = obj_val.find(key);
                if (it != obj_val.end()) return it->second;
                return null_val;
            }

            const rvalue& operator[](size_t idx) const {
                static rvalue null_val;
                if (type != Type::Array || idx >= arr_val.size()) return null_val;
                return arr_val[idx];
            }

            size_t size() const {
                if (type == Type::Array) return arr_val.size();
                if (type == Type::Object) return obj_val.size();
                return 0;
            }

            std::string s() const { return str_val; }
            double d() const { return num_val; }
            int i() const { return static_cast<int>(num_val); }
            long long l() const { return static_cast<long long>(num_val); }
            bool b() const { return bool_val; }
        };

        inline void skip_whitespace(const std::string& str, size_t& pos) {
            while (pos < str.size() && (str[pos] == ' ' || str[pos] == '\t' || str[pos] == '\r' || str[pos] == '\n')) {
                pos++;
            }
        }

        inline std::string parse_string(const std::string& str, size_t& pos) {
            pos++; // skip opening quote
            std::string res;
            while (pos < str.size()) {
                char c = str[pos++];
                if (c == '"') return res;
                if (c == '\\' && pos < str.size()) {
                    char esc = str[pos++];
                    if (esc == 'n') res += '\n';
                    else if (esc == 't') res += '\t';
                    else if (esc == 'r') res += '\r';
                    else if (esc == '\\') res += '\\';
                    else if (esc == '"') res += '"';
                    else res += esc;
                } else {
                    res += c;
                }
            }
            return res;
        }

        inline rvalue parse_val(const std::string& str, size_t& pos);

        inline rvalue parse_val(const std::string& str, size_t& pos) {
            skip_whitespace(str, pos);
            rvalue val;
            if (pos >= str.size()) return val;

            if (str[pos] == '"') {
                val.type = Type::String;
                val.str_val = parse_string(str, pos);
                return val;
            }

            if (str[pos] == '{') {
                val.type = Type::Object;
                pos++; // skip '{'
                while (pos < str.size()) {
                    skip_whitespace(str, pos);
                    if (pos < str.size() && str[pos] == '}') { pos++; break; }
                    if (str[pos] != '"') break;
                    std::string key = parse_string(str, pos);
                    skip_whitespace(str, pos);
                    if (pos < str.size() && str[pos] == ':') pos++;
                    rvalue child = parse_val(str, pos);
                    val.obj_val[key] = child;
                    skip_whitespace(str, pos);
                    if (pos < str.size() && str[pos] == ',') { pos++; }
                    else if (pos < str.size() && str[pos] == '}') { pos++; break; }
                }
                return val;
            }

            if (str[pos] == '[') {
                val.type = Type::Array;
                pos++; // skip '['
                while (pos < str.size()) {
                    skip_whitespace(str, pos);
                    if (pos < str.size() && str[pos] == ']') { pos++; break; }
                    rvalue child = parse_val(str, pos);
                    val.arr_val.push_back(child);
                    skip_whitespace(str, pos);
                    if (pos < str.size() && str[pos] == ',') { pos++; }
                    else if (pos < str.size() && str[pos] == ']') { pos++; break; }
                }
                return val;
            }

            if (str.substr(pos, 4) == "true") {
                val.type = Type::Boolean;
                val.bool_val = true;
                pos += 4;
                return val;
            }

            if (str.substr(pos, 5) == "false") {
                val.type = Type::Boolean;
                val.bool_val = false;
                pos += 5;
                return val;
            }

            if (str.substr(pos, 4) == "null") {
                val.type = Type::Null;
                pos += 4;
                return val;
            }

            // Number
            size_t start = pos;
            if (str[pos] == '-') pos++;
            while (pos < str.size() && (std::isdigit(str[pos]) || str[pos] == '.' || str[pos] == 'e' || str[pos] == 'E' || str[pos] == '+' || str[pos] == '-')) {
                pos++;
            }
            if (pos > start) {
                val.type = Type::Number;
                try {
                    val.num_val = std::stod(str.substr(start, pos - start));
                } catch (...) {
                    val.num_val = 0;
                }
                return val;
            }

            return val;
        }

        inline rvalue load(const std::string& str) {
            size_t pos = 0;
            return parse_val(str, pos);
        }
    } // namespace json

    // HTTP Request
    class request {
    public:
        HTTPMethod method = HTTPMethod::GET;
        std::string raw_url;
        std::string url;
        std::unordered_map<std::string, std::string> url_params;
        std::unordered_map<std::string, std::string> headers;
        std::string body;

        std::string get_header_value(const std::string& key) const {
            std::string lower_key = key;
            std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
            for (const auto& pair : headers) {
                std::string k = pair.first;
                std::transform(k.begin(), k.end(), k.begin(), ::tolower);
                if (k == lower_key) return pair.second;
            }
            return "";
        }
    };

    // HTTP Response
    class response {
    public:
        int code = 200;
        std::unordered_map<std::string, std::string> headers;
        std::string body;

        response() {
            set_header("Content-Type", "text/plain; charset=utf-8");
            set_header("Access-Control-Allow-Origin", "*");
            set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        }

        response(int status) : response() {
            code = status;
        }

        response(const std::string& b) : response() {
            body = b;
        }

        response(int status, const std::string& b) : response() {
            code = status;
            body = b;
        }

        response(const json::wvalue& j) : response() {
            set_header("Content-Type", "application/json; charset=utf-8");
            body = j.dump();
        }

        response(int status, const json::wvalue& j) : response() {
            code = status;
            set_header("Content-Type", "application/json; charset=utf-8");
            body = j.dump();
        }

        void set_header(const std::string& key, const std::string& val) {
            headers[key] = val;
        }

        void write(const std::string& b) {
            body += b;
        }

        void end() {}
    };

    // Route Handler Function
    typedef std::function<response(const request&)> HandlerFunc;

    struct RouteEntry {
        std::string pattern;
        std::regex regex_pattern;
        std::vector<std::string> param_names;
        std::vector<HTTPMethod> methods;
        HandlerFunc handler;
    };

    class RouteRule {
    public:
        RouteEntry& entry;
        RouteRule(RouteEntry& e) : entry(e) {}

        RouteRule& methods(HTTPMethod m1) {
            entry.methods.push_back(m1);
            return *this;
        }

        RouteRule& methods(HTTPMethod m1, HTTPMethod m2) {
            entry.methods.push_back(m1);
            entry.methods.push_back(m2);
            return *this;
        }

        RouteRule& methods(HTTPMethod m1, HTTPMethod m2, HTTPMethod m3) {
            entry.methods.push_back(m1);
            entry.methods.push_back(m2);
            entry.methods.push_back(m3);
            return *this;
        }

        RouteRule& methods(HTTPMethod m1, HTTPMethod m2, HTTPMethod m3, HTTPMethod m4) {
            entry.methods.push_back(m1);
            entry.methods.push_back(m2);
            entry.methods.push_back(m3);
            entry.methods.push_back(m4);
            return *this;
        }

        void operator()(HandlerFunc h) {
            entry.handler = h;
        }
    };

    class SimpleApp {
    private:
        int server_port = 8080;
        std::string bind_address = "0.0.0.0";
        std::vector<RouteEntry> routes;
        std::string static_dir = "frontend";
        bool is_running = false;
        socket_t server_fd = INVALID_SOCKET;

        std::string get_mime_type(const std::string& path) const {
            if (path.rfind(".html") != std::string::npos) return "text/html; charset=utf-8";
            if (path.rfind(".css") != std::string::npos) return "text/css; charset=utf-8";
            if (path.rfind(".js") != std::string::npos) return "application/javascript; charset=utf-8";
            if (path.rfind(".json") != std::string::npos) return "application/json; charset=utf-8";
            if (path.rfind(".png") != std::string::npos) return "image/png";
            if (path.rfind(".jpg") != std::string::npos || path.rfind(".jpeg") != std::string::npos) return "image/jpeg";
            if (path.rfind(".svg") != std::string::npos) return "image/svg+xml";
            if (path.rfind(".ico") != std::string::npos) return "image/x-icon";
            return "text/plain; charset=utf-8";
        }

    public:
        SimpleApp() {
#ifdef _WIN32
            WSADATA wsa;
            WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
        }

        ~SimpleApp() {
            stop();
#ifdef _WIN32
            WSACleanup();
#endif
        }

        SimpleApp& port(int p) {
            server_port = p;
            return *this;
        }

        SimpleApp& bindaddr(const std::string& addr) {
            bind_address = addr;
            return *this;
        }

        SimpleApp& multithreaded() {
            return *this;
        }

        SimpleApp& static_dir_path(const std::string& dir) {
            static_dir = dir;
            return *this;
        }

        RouteRule route(const std::string& path) {
            RouteEntry entry;
            entry.pattern = path;

            // Convert path with <string> or <int> or <arg> to regex
            std::string regex_str = "^";
            size_t pos = 0;
            while (pos < path.length()) {
                size_t start = path.find('<', pos);
                if (start == std::string::npos) {
                    regex_str += path.substr(pos);
                    break;
                }
                regex_str += path.substr(pos, start - pos);
                size_t end = path.find('>', start);
                if (end == std::string::npos) {
                    regex_str += path.substr(start);
                    break;
                }
                std::string tag = path.substr(start + 1, end - start - 1);
                entry.param_names.push_back(tag);
                regex_str += "([^/]+)";
                pos = end + 1;
            }
            regex_str += "$";
            entry.regex_pattern = std::regex(regex_str);

            routes.push_back(entry);
            return RouteRule(routes.back());
        }

        void stop() {
            is_running = false;
            if (server_fd != INVALID_SOCKET) {
                CLOSE_SOCKET(server_fd);
                server_fd = INVALID_SOCKET;
            }
        }

        void handle_client(socket_t client_fd) {
            std::vector<char> buffer(65536);
            int bytes_read = recv(client_fd, buffer.data(), static_cast<int>(buffer.size()) - 1, 0);
            if (bytes_read <= 0) {
                CLOSE_SOCKET(client_fd);
                return;
            }
            buffer[bytes_read] = '\0';
            std::string req_str(buffer.data(), bytes_read);

            // Parse HTTP Request
            std::istringstream stream(req_str);
            std::string line;
            if (!std::getline(stream, line)) {
                CLOSE_SOCKET(client_fd);
                return;
            }

            // Remove \r
            if (!line.empty() && line.back() == '\r') line.pop_back();

            std::istringstream line_stream(line);
            std::string method_str, raw_path, version;
            line_stream >> method_str >> raw_path >> version;

            request req;
            req.method = string_to_method(method_str);
            req.raw_url = raw_path;

            // Parse URL and query params
            size_t q_pos = raw_path.find('?');
            if (q_pos != std::string::npos) {
                req.url = raw_path.substr(0, q_pos);
                std::string q_str = raw_path.substr(q_pos + 1);
                std::istringstream q_stream(q_str);
                std::string q_pair;
                while (std::getline(q_stream, q_pair, '&')) {
                    size_t eq_pos = q_pair.find('=');
                    if (eq_pos != std::string::npos) {
                        req.url_params[q_pair.substr(0, eq_pos)] = q_pair.substr(eq_pos + 1);
                    } else if (!q_pair.empty()) {
                        req.url_params[q_pair] = "";
                    }
                }
            } else {
                req.url = raw_path;
            }

            // Parse Headers
            int content_length = 0;
            while (std::getline(stream, line) && line != "\r" && !line.empty()) {
                if (line.back() == '\r') line.pop_back();
                size_t col_pos = line.find(':');
                if (col_pos != std::string::npos) {
                    std::string h_key = line.substr(0, col_pos);
                    std::string h_val = line.substr(col_pos + 1);
                    while (!h_val.empty() && h_val.front() == ' ') h_val.erase(0, 1);
                    req.headers[h_key] = h_val;
                    std::string lower_k = h_key;
                    std::transform(lower_k.begin(), lower_k.end(), lower_k.begin(), ::tolower);
                    if (lower_k == "content-length") {
                        try { content_length = std::stoi(h_val); } catch (...) {}
                    }
                }
            }

            // Parse Body
            size_t header_end = req_str.find("\r\n\r\n");
            if (header_end != std::string::npos) {
                size_t body_start = header_end + 4;
                if (body_start < req_str.size()) {
                    req.body = req_str.substr(body_start);
                }
            }
            while (content_length > 0 && static_cast<int>(req.body.size()) < content_length) {
                int more = recv(client_fd, buffer.data(), static_cast<int>(buffer.size()) - 1, 0);
                if (more <= 0) break;
                buffer[more] = '\0';
                req.body.append(buffer.data(), more);
            }

            // Handle OPTIONS preflight
            response res;
            if (req.method == HTTPMethod::OPTIONS) {
                res.code = 204;
                res.set_header("Access-Control-Allow-Origin", "*");
                res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
                res.body = "";
            } else {
                // Match Routes
                bool matched = false;
                for (auto& r : routes) {
                    std::smatch match;
                    if (std::regex_match(req.url, match, r.regex_pattern)) {
                        // Check method
                        bool method_ok = r.methods.empty();
                        for (auto m : r.methods) {
                            if (m == req.method) { method_ok = true; break; }
                        }
                        if (!method_ok) continue;

                        // Fill named url params
                        for (size_t i = 1; i < match.size(); ++i) {
                            if (i - 1 < r.param_names.size()) {
                                req.url_params[r.param_names[i - 1]] = match[i].str();
                            }
                            req.url_params["arg" + std::to_string(i - 1)] = match[i].str();
                        }

                        try {
                            res = r.handler(req);
                        } catch (const std::exception& e) {
                            res.code = 500;
                            json::wvalue err;
                            err["error"] = e.what();
                            err["status"] = "error";
                            res = response(500, err);
                        } catch (...) {
                            res.code = 500;
                            json::wvalue err;
                            err["error"] = "Internal server error";
                            err["status"] = "error";
                            res = response(500, err);
                        }
                        matched = true;
                        break;
                    }
                }

                // If not matched, try serving static frontend file
                if (!matched) {
                    std::string file_path = static_dir;
                    if (req.url == "/" || req.url.empty()) {
                        file_path += "/index.html";
                    } else {
                        file_path += req.url;
                    }

                    std::ifstream file(file_path, std::ios::binary);
                    if (file.is_open()) {
                        std::ostringstream ss;
                        ss << file.rdbuf();
                        res.code = 200;
                        res.set_header("Content-Type", get_mime_type(file_path));
                        res.body = ss.str();
                    } else {
                        // Check if file exists in pages/
                        std::string alt_path = static_dir + "/pages" + req.url;
                        if (alt_path.rfind(".html") == std::string::npos) alt_path += ".html";
                        std::ifstream alt_file(alt_path, std::ios::binary);
                        if (alt_file.is_open()) {
                            std::ostringstream ss;
                            ss << alt_file.rdbuf();
                            res.code = 200;
                            res.set_header("Content-Type", "text/html; charset=utf-8");
                            res.body = ss.str();
                        } else {
                            res.code = 404;
                            json::wvalue err;
                            err["error"] = "Not Found: " + req.url;
                            err["status"] = "fail";
                            res = response(404, err);
                        }
                    }
                }
            }

            // Build response string
            std::string status_text = "OK";
            if (res.code == 201) status_text = "Created";
            else if (res.code == 204) status_text = "No Content";
            else if (res.code == 400) status_text = "Bad Request";
            else if (res.code == 401) status_text = "Unauthorized";
            else if (res.code == 403) status_text = "Forbidden";
            else if (res.code == 404) status_text = "Not Found";
            else if (res.code == 500) status_text = "Internal Server Error";

            std::ostringstream out;
            out << "HTTP/1.1 " << res.code << " " << status_text << "\r\n";
            res.set_header("Content-Length", std::to_string(res.body.size()));
            res.set_header("Connection", "close");
            for (const auto& h : res.headers) {
                out << h.first << ": " << h.second << "\r\n";
            }
            out << "\r\n";
            out << res.body;

            std::string out_str = out.str();
            send(client_fd, out_str.c_str(), static_cast<int>(out_str.size()), 0);
            CLOSE_SOCKET(client_fd);
        }

        void run() {
            server_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd == INVALID_SOCKET) {
                std::cerr << "[Crow] Failed to create socket." << std::endl;
                return;
            }

            int opt = 1;
#ifdef _WIN32
            setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
            setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(static_port(server_port));

            if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
                std::cerr << "[Crow] Failed to bind to port " << server_port << std::endl;
                CLOSE_SOCKET(server_fd);
                return;
            }

            if (listen(server_fd, 64) == SOCKET_ERROR) {
                std::cerr << "[Crow] Listen failed." << std::endl;
                CLOSE_SOCKET(server_fd);
                return;
            }

            is_running = true;
            std::cout << "[FinTrack Server] Crow HTTP Server listening on http://localhost:" << server_port << std::endl;
            std::cout << "[FinTrack Server] Serving frontend files from: " << static_dir << std::endl;

            while (is_running) {
                sockaddr_in client_addr{};
                int addr_len = sizeof(client_addr);
                socket_t client_fd = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&addr_len);
                if (client_fd == INVALID_SOCKET) {
                    if (!is_running) break;
                    continue;
                }

                std::thread([this, client_fd]() {
                    this->handle_client(client_fd);
                }).detach();
            }
        }

    private:
        static int static_port(int p) { return p; }
    };

} // namespace crow

#define CROW_ROUTE(app, url) app.route(url)

#endif // CROW_H
