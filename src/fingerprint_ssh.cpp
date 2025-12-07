#include "fingerprint_common.h"
#include "utils.h"
#include <openssl/sha.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <memory>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#define popen _popen
#define pclose _pclose
#else
#include <unistd.h>
#include <cstdlib>
#endif

class SSHFingerprint {
public:
    static std::vector<uint8_t> get_host_key(const std::string& host, int port = 22, int timeout = 10) {
        // Use ssh-keyscan to get host key
        std::stringstream cmd;
        cmd << "ssh-keyscan -p " << port << " -t rsa,ecdsa,ed25519 " << host << " 2>/dev/null";
        
        FILE* pipe = popen(cmd.str().c_str(), "r");
        if (!pipe) {
            std::cerr << "Error running ssh-keyscan. Is OpenSSH installed?" << std::endl;
            return std::vector<uint8_t>();
        }
        
        std::string output;
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            output += buffer;
        }
        pclose(pipe);
        
        // Parse ssh-keyscan output
        // Format: hostname type key
        std::istringstream iss(output);
        std::string line;
        
        while (std::getline(iss, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            std::istringstream line_stream(line);
            std::string hostname, key_type, key_data;
            
            if (line_stream >> hostname >> key_type >> key_data) {
                // Decode base64 key
                return decode_base64(key_data);
            }
        }
        
        return std::vector<uint8_t>();
    }
    
    static std::vector<uint8_t> sha256_hash(const std::vector<uint8_t>& data) {
        std::vector<uint8_t> hash(SHA256_DIGEST_LENGTH);
        SHA256(data.data(), data.size(), hash.data());
        return hash;
    }
    
private:
    static std::vector<uint8_t> decode_base64(const std::string& encoded) {
        // Simple base64 decoder (for production use OpenSSL's base64 functions)
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::vector<uint8_t> result;
        
        int val = 0, valb = -8;
        for (char c : encoded) {
            if (c == '=' || c == '\n' || c == '\r') break;
            size_t pos = chars.find(c);
            if (pos == std::string::npos) continue;
            
            val = (val << 6) + pos;
            valb += 6;
            
            if (valb >= 0) {
                result.push_back((val >> valb) & 0xFF);
                valb -= 8;
            }
        }
        
        return result;
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
    int port = 22; // Default SSH port
    
    if (!utils::parse_host_port(target, host, port)) {
        std::cerr << "Error parsing host:port: " << target << std::endl;
        return 1;
    }
    
    if (port == -1) port = 22;
    
    // Get host key
    std::cout << "Collecting SSH fingerprint from " << host << ":" << port << "..." << std::endl;
    auto key_data = SSHFingerprint::get_host_key(host, port, timeout);
    
    if (key_data.empty()) {
        std::cerr << "Failed to collect SSH host key" << std::endl;
        return 1;
    }
    
    // Calculate SHA-256 fingerprint
    auto hash = SSHFingerprint::sha256_hash(key_data);
    std::string fingerprint = utils::format_fingerprint(hash);
    
    // Save fingerprint
    FingerprintRecord record;
    record.timestamp = utils::resolve_timestamp(timestamp_override);
    record.type = "SSH";
    record.host = host;
    record.port = port;
    record.fingerprint = fingerprint;
    
    FingerprintStorage storage(data_dir);
    storage.save(record);
    
    std::cout << "✓ SSH fingerprint saved: " << fingerprint << std::endl;
    std::cout << "  Host: " << host << ":" << port << std::endl;
    std::cout << "  Timestamp: " << record.timestamp << std::endl;
    
    return 0;
}

