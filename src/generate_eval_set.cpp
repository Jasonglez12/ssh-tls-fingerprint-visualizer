#include "fingerprint_common.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>

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

int collect_tls_fingerprint(const std::string& host, int port, const std::string& data_dir, int timeout, const std::string& timestamp_override) {
    std::stringstream cmd;
    cmd << "./fingerprint_tls " << host << ":" << port << " --data-dir " << data_dir;
    cmd << " --timeout " << timeout;
    if (!timestamp_override.empty()) {
        cmd << " --timestamp \"" << timestamp_override << "\"";
    }

    return system(cmd.str().c_str());
}

int collect_ssh_fingerprint(const std::string& host, int port, const std::string& data_dir, int timeout, const std::string& timestamp_override) {
    std::stringstream cmd;
    cmd << "./fingerprint_ssh " << host << ":" << port << " --data-dir " << data_dir;
    cmd << " --timeout " << timeout;
    if (!timestamp_override.empty()) {
        cmd << " --timestamp \"" << timestamp_override << "\"";
    }

    return system(cmd.str().c_str());
}

int main(int argc, char* argv[]) {
    std::string data_dir = "data";
    int timeout = 10;
    std::vector<EvalHost> hosts = DEFAULT_HOSTS;
    std::string hosts_file;
    std::string timestamp_override;
    bool should_shuffle = false;
    unsigned int seed = 0;
    bool allow_failures = false;

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--data-dir" && i + 1 < argc) {
            data_dir = argv[++i];
        } else if (arg == "--timeout" && i + 1 < argc) {
            timeout = std::stoi(argv[++i]);
        } else if (arg == "--hosts-file" && i + 1 < argc) {
            hosts_file = argv[++i];
        } else if (arg == "--timestamp" && i + 1 < argc) {
            timestamp_override = argv[++i];
        } else if (arg == "--seed" && i + 1 < argc) {
            seed = static_cast<unsigned int>(std::stoul(argv[++i]));
            should_shuffle = true;
        } else if (arg == "--allow-failures") {
            allow_failures = true;
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

    // Load hosts from file if provided
    if (!hosts_file.empty()) {
        hosts.clear();
        std::ifstream file(hosts_file);
        std::string line;
        while (std::getline(file, line)) {
            line = utils::trim(line);
            if (line.empty() || line[0] == '#') continue;

            auto tokens = utils::split(line, ':');
            if (tokens.size() == 3) {
                EvalHost host;
                host.type = utils::to_upper(tokens[0]);
                host.host = tokens[1];
                try {
                    host.port = std::stoi(tokens[2]);
                    hosts.push_back(host);
                } catch (...) {
                    std::cerr << "Invalid port in host file: " << line << std::endl;
                }
            } else {
                std::cerr << "Invalid line in host file: " << line << " (expected TYPE:HOST:PORT)" << std::endl;
            }
        }
    }

    // Shuffle using deterministic seed when provided
    if (should_shuffle) {
        std::mt19937 rng(seed);
        std::shuffle(hosts.begin(), hosts.end(), rng);
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
            result = collect_tls_fingerprint(eval_host.host, eval_host.port, data_dir, timeout, timestamp_override);
        } else if (eval_host.type == "SSH") {
            result = collect_ssh_fingerprint(eval_host.host, eval_host.port, data_dir, timeout, timestamp_override);
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
    file << "  \"created\": \"" << utils::resolve_timestamp(timestamp_override) << "\",\n";
    if (!hosts_file.empty()) {
        file << "  \"hosts_file\": \"" << hosts_file << "\",\n";
    }
    if (should_shuffle) {
        file << "  \"seed\": " << seed << ",\n";
    }
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
    
    return (failed_count > 0 && !allow_failures) ? 1 : 0;
}

