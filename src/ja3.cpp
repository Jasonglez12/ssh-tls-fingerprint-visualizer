#include "ja3.h"
#include <openssl/md5.h>
#include <openssl/ssl.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace ja3 {

std::string bytes_to_hex(const std::vector<uint8_t>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t byte : bytes) {
        ss << std::setw(2) << static_cast<unsigned int>(byte);
    }
    return ss.str();
}

std::string calculate_ja3(const TLSClientHello& client_hello) {
    std::stringstream ja3_string;
    
    // TLS Version
    ja3_string << client_hello.tls_version;
    
    // Cipher Suites (comma-separated)
    ja3_string << ",";
    for (size_t i = 0; i < client_hello.cipher_suites.size(); ++i) {
        if (i > 0) ja3_string << "-";
        ja3_string << client_hello.cipher_suites[i];
    }
    
    // Extensions (comma-separated)
    ja3_string << ",";
    for (size_t i = 0; i < client_hello.extensions.size(); ++i) {
        if (i > 0) ja3_string << "-";
        ja3_string << client_hello.extensions[i];
    }
    
    // Elliptic Curves
    ja3_string << ",";
    if (!client_hello.elliptic_curves.empty()) {
        for (size_t i = 0; i < client_hello.elliptic_curves.size(); ++i) {
            if (i > 0) ja3_string << "-";
            ja3_string << client_hello.elliptic_curves[i];
        }
    }
    
    // Elliptic Curve Point Formats
    ja3_string << ",";
    if (!client_hello.ec_point_formats.empty()) {
        for (size_t i = 0; i < client_hello.ec_point_formats.size(); ++i) {
            if (i > 0) ja3_string << "-";
            ja3_string << static_cast<int>(client_hello.ec_point_formats[i]);
        }
    }
    
    // Calculate MD5 hash
    std::string ja3_str = ja3_string.str();
    unsigned char md5_hash[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(ja3_str.c_str()), ja3_str.length(), md5_hash);
    
    // Convert to hex string (32 characters)
    return bytes_to_hex(std::vector<uint8_t>(md5_hash, md5_hash + MD5_DIGEST_LENGTH));
}

std::string calculate_ja3s(const TLSServerHello& server_hello) {
    std::stringstream ja3s_string;
    
    // TLS Version
    ja3s_string << server_hello.tls_version;
    
    // Cipher Suite
    ja3s_string << "," << server_hello.cipher_suite;
    
    // Extensions (comma-separated)
    ja3s_string << ",";
    if (!server_hello.extensions.empty()) {
        for (size_t i = 0; i < server_hello.extensions.size(); ++i) {
            if (i > 0) ja3s_string << "-";
            ja3s_string << server_hello.extensions[i];
        }
    }
    
    // Calculate MD5 hash
    std::string ja3s_str = ja3s_string.str();
    unsigned char md5_hash[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(ja3s_str.c_str()), ja3s_str.length(), md5_hash);
    
    // Convert to hex string (32 characters)
    return bytes_to_hex(std::vector<uint8_t>(md5_hash, md5_hash + MD5_DIGEST_LENGTH));
}

bool parse_client_hello(const std::vector<uint8_t>& data, TLSClientHello& client_hello) {
    if (data.size() < 5) return false; // Minimum handshake header
    
    size_t pos = 0;
    
    // Check Handshake Type (should be 0x01 for ClientHello)
    if (data[pos] != 0x01) return false;
    pos++;
    
    // Skip handshake length (3 bytes)
    pos += 3;
    
    // TLS Version (2 bytes)
    if (pos + 2 > data.size()) return false;
    client_hello.tls_version = (data[pos] << 8) | data[pos + 1];
    pos += 2;
    
    // Random (32 bytes) - skip
    pos += 32;
    
    // Session ID Length (1 byte)
    if (pos >= data.size()) return false;
    uint8_t session_id_len = data[pos];
    pos += 1 + session_id_len;
    
    // Cipher Suites Length (2 bytes)
    if (pos + 2 > data.size()) return false;
    uint16_t cipher_suites_len = (data[pos] << 8) | data[pos + 1];
    pos += 2;
    
    // Cipher Suites
    size_t cipher_end = pos + cipher_suites_len;
    while (pos + 2 <= cipher_end && pos + 2 <= data.size()) {
        uint16_t cipher = (data[pos] << 8) | data[pos + 1];
        client_hello.cipher_suites.push_back(cipher);
        pos += 2;
    }
    
    // Compression Methods Length (1 byte)
    if (pos >= data.size()) return false;
    uint8_t compression_len = data[pos];
    pos += 1 + compression_len;
    
    // Extensions Length (2 bytes)
    if (pos + 2 > data.size()) return false;
    uint16_t extensions_len = (data[pos] << 8) | data[pos + 1];
    pos += 2;
    
    // Parse Extensions
    size_t extensions_end = pos + extensions_len;
    while (pos + 4 <= extensions_end && pos + 4 <= data.size()) {
        uint16_t ext_type = (data[pos] << 8) | data[pos + 1];
        uint16_t ext_len = (data[pos + 2] << 8) | data[pos + 3];
        pos += 4;
        
        client_hello.extensions.push_back(ext_type);
        
        // Handle specific extensions
        if (ext_type == 0x000A) { // supported_groups (elliptic_curves)
            if (pos + 2 <= data.size()) {
                uint16_t groups_len = (data[pos] << 8) | data[pos + 1];
                pos += 2;
                size_t groups_end = pos + groups_len;
                while (pos + 2 <= groups_end && pos + 2 <= data.size()) {
                    uint16_t curve = (data[pos] << 8) | data[pos + 1];
                    client_hello.elliptic_curves.push_back(curve);
                    pos += 2;
                }
            } else {
                pos += ext_len;
            }
        } else if (ext_type == 0x000B) { // ec_point_formats
            if (pos + 1 <= data.size()) {
                uint8_t formats_len = data[pos];
                pos += 1;
                size_t formats_end = pos + formats_len;
                while (pos < formats_end && pos < data.size()) {
                    client_hello.ec_point_formats.push_back(data[pos]);
                    pos++;
                }
            } else {
                pos += ext_len;
            }
        } else if (ext_type == 0x0000) { // server_name
            if (pos + 3 <= data.size()) {
                uint16_t name_list_len = (data[pos] << 8) | data[pos + 1];
                pos += 2;
                if (pos + 1 <= data.size()) {
                    uint8_t name_type = data[pos];
                    pos += 1;
                    if (name_type == 0 && pos + 2 <= data.size()) { // host_name
                        uint16_t name_len = (data[pos] << 8) | data[pos + 1];
                        pos += 2;
                        if (pos + name_len <= data.size()) {
                            client_hello.sni = std::string(
                                reinterpret_cast<const char*>(data.data() + pos),
                                name_len
                            );
                            pos += name_len;
                        }
                    }
                }
            } else {
                pos += ext_len;
            }
        } else {
            pos += ext_len;
        }
    }
    
    return true;
}

bool parse_server_hello(const std::vector<uint8_t>& data, TLSServerHello& server_hello) {
    if (data.size() < 5) return false;
    
    size_t pos = 0;
    
    // Check Handshake Type (should be 0x02 for ServerHello)
    if (data[pos] != 0x02) return false;
    pos++;
    
    // Skip handshake length (3 bytes)
    pos += 3;
    
    // TLS Version (2 bytes)
    if (pos + 2 > data.size()) return false;
    server_hello.tls_version = (data[pos] << 8) | data[pos + 1];
    pos += 2;
    
    // Random (32 bytes) - skip
    pos += 32;
    
    // Session ID Length (1 byte)
    if (pos >= data.size()) return false;
    uint8_t session_id_len = data[pos];
    pos += 1 + session_id_len;
    
    // Cipher Suite (2 bytes)
    if (pos + 2 > data.size()) return false;
    server_hello.cipher_suite = (data[pos] << 8) | data[pos + 1];
    pos += 2;
    
    // Compression Method (1 byte) - skip
    pos += 1;
    
    // Extensions Length (2 bytes, optional)
    if (pos + 2 <= data.size()) {
        uint16_t extensions_len = (data[pos] << 8) | data[pos + 1];
        pos += 2;
        
        // Parse Extensions
        size_t extensions_end = pos + extensions_len;
        while (pos + 4 <= extensions_end && pos + 4 <= data.size()) {
            uint16_t ext_type = (data[pos] << 8) | data[pos + 1];
            uint16_t ext_len = (data[pos + 2] << 8) | data[pos + 3];
            pos += 4;
            
            server_hello.extensions.push_back(ext_type);
            pos += ext_len;
        }
    }
    
    return true;
}

} // namespace ja3

