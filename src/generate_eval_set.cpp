#include "fingerprint_common.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

struct EvalHost {
    std::string host;
    int port;
    std::string type;
};

// Common hosts for evaluation
const std::vector<EvalHost> DEFAULT_HOSTS = {
    {"github.com", 443, "TLS"},
    {"github.com", 22, "SSH"},
    {"google.com", 443, "TLS"},
    {"example.com", 443, "TLS"},
    {"www.cloudflare.com", 443, "TLS"},
};

int collect_tls_fingerprint(const std::string& host, int port, const std::string& data_dir) {
    std::stringstream cmd;
    cmd << "fingerprint_tls " << host << ":" << port << " --data-dir " << data_dir;
    
    return system(cmd.str().c_str());
}

int collect_ssh_fingerprint(const std::string& host, int port, const std::string& data_dir) {
    std::stringstream cmd;
    cmd << "fingerprint_ssh " << host << ":" << port << " --data-dir " << data_dir;
    
    return system(cmd.str().c_str());
}

int main(int argc, char* argv[]) {
    std::string data_dir = "data";
    int timeout = 10;
    std::vector<EvalHost> hosts = DEFAULT_HOSTS;
    
    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--data-dir" && i + 1 < argc) {
            data_dir = argv[++i];
        } else if (arg == "--timeout" && i + 1 < argc) {
            timeout = std::stoi(argv[++i]);
        } else if (arg == "--hosts") {
            // Parse custom hosts
            hosts.clear();
            ++i;
            while (i < argc && argv[i][0] != '-') {
                std::string host_str = argv[i++];
                auto tokens = utils::split(host_str, ':');
                if (tokens.size() == 3) {
                    EvalHost host;
                    host.type = utils::to_upper(tokens[0]);
                    host.host = tokens[1];
                    try {
                        host.port = std::stoi(tokens[2]);
                        hosts.push_back(host);
                    } catch (...) {
                        std::cerr << "Invalid port in: " << host_str << std::endl;
                    }
                } else {
                    std::cerr << "Invalid host format: " << host_str << " (expected TYPE:HOST:PORT)" << std::endl;
                }
            }
            --i; // Adjust for loop increment
        }
    }
    
    std::cout << "Generating evaluation dataset...\n" << std::endl;
    
    int success_count = 0;
    int failed_count = 0;
    std::vector<EvalHost> successful;
    std::vector<EvalHost> failed;
    
    for (const auto& eval_host : hosts) {
        std::cout << "Collecting " << eval_host.type << " fingerprint for " 
                  << eval_host.host << ":" << eval_host.port << "..." << std::endl;
        
        int result = 0;
        if (eval_host.type == "TLS") {
            result = collect_tls_fingerprint(eval_host.host, eval_host.port, data_dir);
        } else if (eval_host.type == "SSH") {
            result = collect_ssh_fingerprint(eval_host.host, eval_host.port, data_dir);
        } else {
            std::cerr << "Unknown type: " << eval_host.type << std::endl;
            failed.push_back(eval_host);
            failed_count++;
            continue;
        }
        
        if (result == 0) {
            success_count++;
            successful.push_back(eval_host);
        } else {
            failed_count++;
            failed.push_back(eval_host);
        }
    }
    
    // Print summary
    std::cout << "\n=== Evaluation Dataset Summary ===\n" << std::endl;
    std::cout << "✓ Successfully collected: " << success_count << std::endl;
    for (const auto& host : successful) {
        std::cout << "  " << host.type << " " << host.host << ":" << host.port << std::endl;
    }
    
    if (!failed.empty()) {
        std::cout << "\n✗ Failed: " << failed_count << std::endl;
        for (const auto& host : failed) {
            std::cout << "  " << host.type << " " << host.host << ":" << host.port << std::endl;
        }
    }
    
    // Save evaluation metadata
    std::string eval_file = data_dir + "/eval_metadata.json";
    std::ofstream file(eval_file);
    file << "{\n";
    file << "  \"created\": \"" << utils::get_current_timestamp() << "\",\n";
    file << "  \"hosts\": [\n";
    
    for (size_t i = 0; i < successful.size(); ++i) {
        file << "    {\n";
        file << "      \"type\": \"" << successful[i].type << "\",\n";
        file << "      \"host\": \"" << successful[i].host << "\",\n";
        file << "      \"port\": " << successful[i].port << "\n";
        file << "    }";
        if (i < successful.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "  ],\n";
    file << "  \"failed\": [\n";
    
    for (size_t i = 0; i < failed.size(); ++i) {
        file << "    {\n";
        file << "      \"type\": \"" << failed[i].type << "\",\n";
        file << "      \"host\": \"" << failed[i].host << "\",\n";
        file << "      \"port\": " << failed[i].port << "\n";
        file << "    }";
        if (i < failed.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    file.close();
    
    std::cout << "\n✓ Evaluation metadata saved to " << eval_file << std::endl;
    
    return failed_count > 0 ? 1 : 0;
}

