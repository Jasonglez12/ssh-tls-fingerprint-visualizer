#ifndef JA3_H
#define JA3_H

#include <string>
#include <vector>
#include <cstdint>

// TLS Handshake structure for JA3/JA3S calculation
struct TLSClientHello {
    uint16_t tls_version;
    std::vector<uint16_t> cipher_suites;
    std::vector<uint16_t> extensions;
    std::vector<uint16_t> elliptic_curves;
    std::vector<uint8_t> ec_point_formats;
    std::string sni; // Server Name Indication
};

struct TLSServerHello {
    uint16_t tls_version;
    uint16_t cipher_suite;
    std::vector<uint16_t> extensions;
};

namespace ja3 {
    // Calculate JA3 fingerprint from ClientHello
    // Formula: md5(TLSVersion + "," + CipherSuites + "," + Extensions + "," + EllipticCurves + "," + EllipticCurvePointFormats)
    std::string calculate_ja3(const TLSClientHello& client_hello);
    
    // Calculate JA3S fingerprint from ServerHello
    // Formula: md5(TLSVersion + "," + CipherSuite + "," + Extensions)
    std::string calculate_ja3s(const TLSServerHello& server_hello);
    
    // Parse TLS ClientHello from raw bytes
    bool parse_client_hello(const std::vector<uint8_t>& data, TLSClientHello& client_hello);
    
    // Parse TLS ServerHello from raw bytes
    bool parse_server_hello(const std::vector<uint8_t>& data, TLSServerHello& server_hello);
    
    // Helper: Convert bytes to hex string (lowercase, no separators)
    std::string bytes_to_hex(const std::vector<uint8_t>& bytes);
}

#endif // JA3_H

