#include "fingerprint_common.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>

class Visualizer {
public:
    static void generate_timeline_data(const std::vector<FingerprintRecord>& records, const std::string& output_file) {
        // Group by (type, host, port)
        std::map<std::tuple<std::string, std::string, int>, std::map<std::string, int>> timeline;
        
        for (const auto& record : records) {
            auto key = std::make_tuple(record.type, record.host, record.port);
            
            // Count unique fingerprints at each timestamp
            if (timeline[key].find(record.timestamp) == timeline[key].end()) {
                timeline[key][record.timestamp] = 0;
            }
            timeline[key][record.timestamp]++;
        }
        
        // Write JSON output for external visualization
        std::ofstream file(output_file);
        file << "{\n";
        file << "  \"type\": \"timeline\",\n";
        file << "  \"data\": [\n";
        
        bool first_group = true;
        for (const auto& [key, timestamps] : timeline) {
            if (!first_group) file << ",\n";
            first_group = false;
            
            file << "    {\n";
            file << "      \"type\": \"" << std::get<0>(key) << "\",\n";
            file << "      \"host\": \"" << std::get<1>(key) << "\",\n";
            file << "      \"port\": " << std::get<2>(key) << ",\n";
            file << "      \"points\": [\n";
            
            bool first_point = true;
            for (const auto& [timestamp, count] : timestamps) {
                if (!first_point) file << ",\n";
                first_point = false;
                file << "        {\"time\": \"" << timestamp << "\", \"count\": " << count << "}";
            }
            
            file << "\n      ]\n";
            file << "    }";
        }
        
        file << "\n  ]\n";
        file << "}\n";
        file.close();
        
        std::cout << "✓ Timeline data saved to " << output_file << std::endl;
        std::cout << "  Use this JSON file with external visualization tools" << std::endl;
    }
    
    static void generate_summary_data(const std::vector<FingerprintRecord>& records, const std::string& output_file) {
        // Count fingerprints per host
        std::map<std::string, std::map<std::string, int>> host_counts;
        std::map<std::string, std::set<std::string>> host_fingerprints;
        
        for (const auto& record : records) {
            host_counts[record.host][record.type]++;
            host_fingerprints[record.host].insert(record.fingerprint);
        }
        
        // Write JSON output
        std::ofstream file(output_file);
        file << "{\n";
        file << "  \"type\": \"summary\",\n";
        file << "  \"data\": [\n";
        
        bool first = true;
        for (const auto& [host, counts] : host_counts) {
            if (!first) file << ",\n";
            first = false;
            
            file << "    {\n";
            file << "      \"host\": \"" << host << "\",\n";
            file << "      \"tls_count\": " << (counts.find("TLS") != counts.end() ? counts.at("TLS") : 0) << ",\n";
            file << "      \"ssh_count\": " << (counts.find("SSH") != counts.end() ? counts.at("SSH") : 0) << ",\n";
            file << "      \"unique_fingerprints\": " << host_fingerprints[host].size() << "\n";
            file << "    }";
        }
        
        file << "\n  ]\n";
        file << "}\n";
        file.close();
        
        std::cout << "✓ Summary data saved to " << output_file << std::endl;
        std::cout << "  Use this JSON file with external visualization tools" << std::endl;
    }
    
    static void print_ascii_summary(const std::vector<FingerprintRecord>& records) {
        // Count fingerprints per host
        std::map<std::string, std::map<std::string, int>> host_counts;
        std::map<std::string, std::set<std::string>> host_fingerprints;
        
        for (const auto& record : records) {
            host_counts[record.host][record.type]++;
            host_fingerprints[record.host].insert(record.fingerprint);
        }
        
        std::cout << "\n=== Fingerprint Summary ===\n\n";
        std::cout << std::left << std::setw(30) << "Host" 
                  << std::setw(10) << "TLS" 
                  << std::setw(10) << "SSH" 
                  << std::setw(20) << "Unique FPs" << std::endl;
        std::cout << std::string(70, '-') << std::endl;
        
        for (const auto& [host, counts] : host_counts) {
            std::cout << std::left << std::setw(30) << host
                      << std::setw(10) << (counts.find("TLS") != counts.end() ? std::to_string(counts.at("TLS")) : "0")
                      << std::setw(10) << (counts.find("SSH") != counts.end() ? std::to_string(counts.at("SSH")) : "0")
                      << std::setw(20) << host_fingerprints[host].size() << std::endl;
        }
        std::cout << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <timeline|summary|all> [options]" << std::endl;
        return 1;
    }
    
    std::string chart_type = argv[1];
    std::string data_dir = "data";
    std::string type = "";
    std::string output_file = "";
    
    // Parse arguments
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--data-dir" && i + 1 < argc) {
            data_dir = argv[++i];
        } else if (arg == "--type" && i + 1 < argc) {
            type = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            output_file = argv[++i];
        }
    }
    
    // Load data
    FingerprintStorage storage(data_dir);
    auto records = storage.load_all(type);
    
    if (records.empty()) {
        std::cerr << "No fingerprint data found in " << data_dir << std::endl;
        return 1;
    }
    
    // Generate visualizations
    if (chart_type == "timeline" || chart_type == "all") {
        std::string timeline_file = output_file.empty() ? "data/timeline.json" : output_file;
        if (chart_type == "all" && !output_file.empty()) {
            timeline_file = output_file.substr(0, output_file.find_last_of('.')) + "_timeline.json";
        }
        Visualizer::generate_timeline_data(records, timeline_file);
    }
    
    if (chart_type == "summary" || chart_type == "all") {
        std::string summary_file = output_file.empty() ? "data/summary.json" : output_file;
        if (chart_type == "all" && !output_file.empty()) {
            summary_file = output_file.substr(0, output_file.find_last_of('.')) + "_summary.json";
        }
        Visualizer::generate_summary_data(records, summary_file);
        Visualizer::print_ascii_summary(records);
    }
    
    return 0;
}

