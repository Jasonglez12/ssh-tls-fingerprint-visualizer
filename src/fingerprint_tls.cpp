#include "fingerprint_common.h"
#include "utils.h"
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define close closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>
#endif

class TLSCertificate {
public:
    static std::vector<uint8_t> get_certificate(const std::string& host, int port, int timeout = 10) {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
        
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        
        std::vector<uint8_t> cert_data;
        
        // Create socket
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            std::cerr << "Error creating socket" << std::endl;
            return cert_data;
        }
        
        // Set timeout
#ifdef _WIN32
        DWORD timeout_ms = timeout * 1000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout_ms, sizeof(timeout_ms));
#else
        struct timeval tv;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));
#endif
        
        // Resolve hostname
        struct hostent* host_info = gethostbyname(host.c_str());
        if (!host_info) {
            std::cerr << "Error resolving hostname: " << host << std::endl;
            close(sock);
            return cert_data;
        }
        
        // Connect
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        memcpy(&server_addr.sin_addr, host_info->h_addr_list[0], host_info->h_length);
        
        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Error connecting to " << host << ":" << port << std::endl;
            close(sock);
            return cert_data;
        }
        
        // Create SSL context
        SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
        if (!ctx) {
            std::cerr << "Error creating SSL context" << std::endl;
            close(sock);
            return cert_data;
        }
        
        // Don't verify certificate (we just want to get it)
        SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);
        
        // Create SSL connection
        SSL* ssl = SSL_new(ctx);
        if (!ssl) {
            std::cerr << "Error creating SSL connection" << std::endl;
            SSL_CTX_free(ctx);
            close(sock);
            return cert_data;
        }
        
        SSL_set_fd(ssl, sock);
        
        // Connect
        if (SSL_connect(ssl) <= 0) {
            std::cerr << "Error in SSL handshake" << std::endl;
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            close(sock);
            return cert_data;
        }
        
        // Get certificate
        X509* cert = SSL_get_peer_certificate(ssl);
        if (cert) {
            // Get DER encoded certificate
            unsigned char* der = nullptr;
            int len = i2d_X509(cert, &der);
            if (len > 0) {
                cert_data.assign(der, der + len);
                OPENSSL_free(der);
            }
            X509_free(cert);
        }
        
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sock);
        
#ifdef _WIN32
        WSACleanup();
#endif
        
        return cert_data;
    }
    
    static std::vector<uint8_t> sha256_hash(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> hash(SHA256_DIGEST_LENGTH);
        SHA256(data.data(), data.size(), hash.data());
        return hash;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <host[:port]> [--data-dir DIR] [--timeout SECONDS] [--timestamp FIXED]" << std::endl;
        return 1;
    }

    std::string target = argv[1];
    std::string data_dir = "data";
    int timeout = 10;
    std::string timestamp_override;
    
    // Parse arguments
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--data-dir" && i + 1 < argc) {
            data_dir = argv[++i];
        } else if (arg == "--timeout" && i + 1 < argc) {
            timeout = std::stoi(argv[++i]);
        } else if (arg == "--timestamp" && i + 1 < argc) {
            timestamp_override = argv[++i];
        }
    }
    
    // Parse host:port
    std::string host;
    int port = 443; // Default HTTPS port
    
    if (!utils::parse_host_port(target, host, port)) {
        std::cerr << "Error parsing host:port: " << target << std::endl;
        return 1;
    }
    
    if (port == -1) port = 443;
    
    // Get certificate
    std::cout << "Collecting TLS fingerprint from " << host << ":" << port << "..." << std::endl;
    auto cert_data = TLSCertificate::get_certificate(host, port, timeout);
    
    if (cert_data.empty()) {
        std::cerr << "Failed to collect TLS certificate" << std::endl;
        return 1;
    }
    
    // Calculate SHA-256 fingerprint
    auto hash = TLSCertificate::sha256_hash(cert_data);
    std::string fingerprint = utils::format_fingerprint(hash);
    
    // Save fingerprint
    FingerprintRecord record;
    record.timestamp = utils::resolve_timestamp(timestamp_override);
    record.type = "TLS";
    record.host = host;
    record.port = port;
    record.fingerprint = fingerprint;
    
    FingerprintStorage storage(data_dir);
    storage.save(record);
    
    std::cout << "✓ TLS fingerprint saved: " << fingerprint << std::endl;
    std::cout << "  Host: " << host << ":" << port << std::endl;
    std::cout << "  Timestamp: " << record.timestamp << std::endl;
    
    return 0;
}

