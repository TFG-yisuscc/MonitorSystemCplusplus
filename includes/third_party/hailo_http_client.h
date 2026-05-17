//
// Minimal HTTP/1.1 client using POSIX sockets.
// Supports single-response POST/GET and streaming NDJSON POST.
// Designed for Hailo-Ollama (plain HTTP, local network, no TLS needed).
//
#pragma once
#include <string>
#include <functional>
#include <stdexcept>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <sstream>

namespace hailo_http {

struct Response {
    int status_code = 0;
    std::string body;
};

// Line received during streaming, plus the nanosecond timestamp of arrival.
struct StreamLine {
    std::string text;
    int64_t     ts_ns = 0;
};

class Client {
public:
    Client(const std::string& host, int port, int timeout_s = 3600)
        : host_(host), port_(port), timeout_s_(timeout_s) {}

    // Single-response POST (stream:false).
    Response post(const std::string& path, const std::string& json_body) const {
        int sock = open_socket();
        send_all(sock, build_request("POST", path, json_body));
        std::string raw = recv_all(sock);
        ::close(sock);
        return parse_response(raw);
    }

    // GET with no body (e.g. /api/tags).
    Response get(const std::string& path) const {
        int sock = open_socket();
        send_all(sock, build_request("GET", path, ""));
        std::string raw = recv_all(sock);
        ::close(sock);
        return parse_response(raw);
    }

    // Streaming POST (stream:true).
    // Calls on_line for every non-empty NDJSON line as it arrives.
    // on_line returns false → detiene la lectura y cierra el socket inmediatamente.
    // Esto es necesario porque hailo-ollama envía Connection:keep-alive y no cierra
    // el socket al terminar; debemos cerrar nosotros tras recibir "done":true.
    void post_streaming(const std::string& path, const std::string& json_body,
                        const std::function<bool(const StreamLine&)>& on_line) const {
        int sock = open_socket();

        // Timeout corto por recv: el Hailo genera ~2.6 tok/s (385 ms/tok).
        // Si no llegan datos en 30 s asumimos fin de stream (hailo-ollama usa
        // Connection:keep-alive y puede no enviar \n al final de la última línea).
        struct timeval tv_stream{ 30, 0 };
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv_stream, sizeof(tv_stream));

        send_all(sock, build_request("POST", path, json_body));

        bool chunked = skip_headers_and_detect_chunked(sock);

        std::string line_buf;
        char buf[4096];

        bool stop = false;
        auto dispatch = [&](const std::string& line) {
            if (line.empty() || stop) return;
            StreamLine sl;
            sl.text  = line;
            sl.ts_ns = now_ns();
            if (!on_line(sl)) stop = true;
        };

        if (chunked) {
            while (!stop) {
                std::string size_str = read_line_raw(sock);
                if (size_str.empty()) break;  // timeout o conexión cerrada
                auto semi = size_str.find(';');
                if (semi != std::string::npos) size_str.resize(semi);

                size_t chunk_size = 0;
                try { chunk_size = std::stoul(size_str, nullptr, 16); }
                catch (...) { break; }
                if (chunk_size == 0) break;

                std::string chunk = recv_exact(sock, chunk_size);
                recv_exact(sock, 2);  // trailing \r\n

                for (char c : chunk) {
                    if (c == '\n') { dispatch(line_buf); line_buf.clear(); }
                    else if (c != '\r') line_buf += c;
                    if (stop) break;
                }
                // Hailo-Ollama omite \n al final del último chunk: despachar inmediatamente
                // para no esperar el timeout de SO_RCVTIMEO.
                if (!stop && !line_buf.empty()) { dispatch(line_buf); line_buf.clear(); }
            }
        } else {
            ssize_t n;
            // n == 0 → EOF;  n < 0 → error o timeout de SO_RCVTIMEO → salimos en ambos casos
            while (!stop && (n = ::recv(sock, buf, sizeof(buf), 0)) > 0) {
                for (ssize_t i = 0; i < n && !stop; ++i) {
                    char c = buf[i];
                    if (c == '\n') { dispatch(line_buf); line_buf.clear(); }
                    else if (c != '\r') line_buf += c;
                }
            }
        }
        // Despachar línea pendiente (última línea sin \n final)
        if (!stop && !line_buf.empty()) dispatch(line_buf);
        ::close(sock);
    }

private:
    std::string host_;
    int         port_;
    int         timeout_s_;

    static int64_t now_ns() {
        struct timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return static_cast<int64_t>(ts.tv_sec) * 1'000'000'000LL + ts.tv_nsec;
    }

    int open_socket() const {
        struct addrinfo hints{}, *res = nullptr;
        hints.ai_family   = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        int rc = getaddrinfo(host_.c_str(), std::to_string(port_).c_str(), &hints, &res);
        if (rc != 0)
            throw std::runtime_error("hailo_http: cannot resolve '" + host_ + "': " +
                                     gai_strerror(rc));

        // Iteramos todas las direcciones devueltas (puede haber ::1 y 127.0.0.1).
        // Esto es necesario en binarios estáticos donde "localhost" puede resolver
        // a IPv6 primero aunque el servidor solo escuche en IPv4.
        struct timeval tv{ timeout_s_, 0 };
        for (struct addrinfo* rp = res; rp != nullptr; rp = rp->ai_next) {
            int sock = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (sock < 0) continue;

            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

            if (::connect(sock, rp->ai_addr, rp->ai_addrlen) == 0) {
                freeaddrinfo(res);
                return sock;
            }
            ::close(sock);
        }
        freeaddrinfo(res);
        throw std::runtime_error("hailo_http: connection failed to " + host_ +
                                 ":" + std::to_string(port_));
    }

    std::string build_request(const std::string& method, const std::string& path,
                              const std::string& body) const {
        std::string req = method + " " + path + " HTTP/1.1\r\n"
            "Host: " + host_ + ":" + std::to_string(port_) + "\r\n"
            "Connection: close\r\n";
        if (!body.empty())
            req += "Content-Type: application/json\r\n"
                   "Content-Length: " + std::to_string(body.size()) + "\r\n";
        req += "\r\n" + body;
        return req;
    }

    static void send_all(int sock, const std::string& data) {
        size_t sent = 0;
        while (sent < data.size()) {
            ssize_t n = ::send(sock, data.c_str() + sent, data.size() - sent, 0);
            if (n <= 0) throw std::runtime_error("hailo_http: send() failed");
            sent += n;
        }
    }

    static std::string recv_all(int sock) {
        std::string buf;
        char chunk[4096];
        ssize_t n;
        while ((n = ::recv(sock, chunk, sizeof(chunk), 0)) > 0)
            buf.append(chunk, static_cast<size_t>(n));
        return buf;
    }

    // Read exactly n bytes from socket.
    static std::string recv_exact(int sock, size_t n) {
        std::string buf(n, '\0');
        size_t got = 0;
        while (got < n) {
            ssize_t r = ::recv(sock, buf.data() + got, n - got, 0);
            if (r <= 0) throw std::runtime_error("hailo_http: recv_exact() failed");
            got += static_cast<size_t>(r);
        }
        return buf;
    }

    // Read a CRLF-terminated line from socket (returns line without CRLF).
    static std::string read_line_raw(int sock) {
        std::string line;
        char c;
        while (::recv(sock, &c, 1, 0) == 1) {
            if (c == '\n') break;
            if (c != '\r') line += c;
        }
        return line;
    }

    // Reads HTTP headers from socket, returns true if Transfer-Encoding: chunked.
    static bool skip_headers_and_detect_chunked(int sock) {
        bool chunked = false;
        std::string line;
        // First line is the status line — skip.
        read_line_raw(sock);
        // Remaining header lines until blank line.
        while (!(line = read_line_raw(sock)).empty()) {
            std::string lower = line;
            for (char& c : lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            if (lower.find("transfer-encoding") != std::string::npos &&
                lower.find("chunked") != std::string::npos)
                chunked = true;
        }
        return chunked;
    }

    static Response parse_response(const std::string& raw) {
        auto sep = raw.find("\r\n\r\n");
        if (sep == std::string::npos)
            throw std::runtime_error("hailo_http: malformed response (no header/body separator)");

        Response resp;
        if (std::sscanf(raw.c_str(), "HTTP/1.%*d %d", &resp.status_code) != 1)
            throw std::runtime_error("hailo_http: cannot parse HTTP status line");

        resp.body = raw.substr(sep + 4);
        while (!resp.body.empty() &&
               (resp.body.back() == '\0' || resp.body.back() == '\r' || resp.body.back() == '\n'))
            resp.body.pop_back();

        return resp;
    }
};

} // namespace hailo_http
