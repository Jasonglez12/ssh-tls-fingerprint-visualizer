#include "fingerprint_common.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <fstream>
#include <algorithm>

struct DiffResult {
    std::vector<FingerprintRecord> changed;
    std::vector<FingerprintRecord> new_records;
    std::vector<FingerprintRecord> unchanged;
};

class BaselineManager {
public:
    static std::map<std::tuple<std::string, std::string, int>, std::string> load_baseline(const std::string& baseline_file) {
        std::map<std::tuple<std::string, std::string, int>, std::string> baseline;
        
        if (!utils::file_exists(baseline_file)) {
            return baseline;
        }
        
        auto records = JSONWriter::read_all(baseline_file);
        for (const auto& record : records) {
            auto key = std::make_tuple(record.type, record.host, record.port);
            baseline[key] = record.fingerprint;
        }
        
        return baseline;
    }
    
    static void create_baseline(const std::string& data_dir, const std::string& output_file, const std::string& type = "", const std::string& fixed_timestamp = "") {
        FingerprintStorage storage(data_dir);
        auto latest = storage.load_latest(type);
        
        if (latest.empty()) {
            std::cerr << "No fingerprints found in " << data_dir << std::endl;
            return;
        }
        
        // Create baseline records
        std::vector<FingerprintRecord> baseline_records;
        for (const auto& [key, fingerprint] : latest) {
            FingerprintRecord record;
            record.timestamp = utils::resolve_timestamp(fixed_timestamp);
            record.type = std::get<0>(key);
            record.host = std::get<1>(key);
            record.port = std::get<2>(key);
            record.fingerprint = fingerprint;
            baseline_records.push_back(record);
        }
        
        // Write baseline JSON
        std::ofstream file(output_file, std::ios::trunc);
        file << "[\n";
        for (size_t i = 0; i < baseline_records.size(); ++i) {
            file << "  {\n";
            file << "    \"type\": \"" << baseline_records[i].type << "\",\n";
            file << "    \"host\": \"" << baseline_records[i].host << "\",\n";
            file << "    \"port\": " << baseline_records[i].port << ",\n";
            file << "    \"fingerprint\": \"" << baseline_records[i].fingerprint << "\",\n";
            file << "    \"baseline_date\": \"" << baseline_records[i].timestamp << "\"\n";
            file << "  }";
            if (i < baseline_records.size() - 1) file << ",";
            file << "\n";
        }
        file << "]\n";
        file.close();
        
        std::cout << "✓ Baseline created: " << output_file << std::endl;
        std::cout << "  " << baseline_records.size() << " fingerprints saved" << std::endl;
    }
    
    static DiffResult compare(const std::map<std::tuple<std::string, std::string, int>, std::string>& baseline,
                             const std::map<std::tuple<std::string, std::string, int>, std::string>& current) {
        DiffResult result;
        
        // Check all current fingerprints
        for (const auto& [key, current_fp] : current) {
            FingerprintRecord record;
            record.type = std::get<0>(key);
            record.host = std::get<1>(key);
            record.port = std::get<2>(key);
            record.fingerprint = current_fp;
            
            if (baseline.find(key) != baseline.end()) {
                std::string baseline_fp = baseline.at(key);
                if (baseline_fp != current_fp) {
                    result.changed.push_back(record);
                } else {
                    result.unchanged.push_back(record);
                }
            } else {
                result.new_records.push_back(record);
            }
        }
        
        return result;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <create|diff> [options]" << std::endl;
        std::cerr << "  create: Create baseline from current fingerprints" << std::endl;
        std::cerr << "  diff: Compare current fingerprints against baseline" << std::endl;
        return 1;
    }
    
    std::string action = argv[1];
    std::string baseline_file = "data/baseline.json";
    std::string data_dir = "data";
    std::string type = "";
    std::string output_file = "";
    std::string timestamp_override;
    
    // Parse arguments
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--baseline" && i + 1 < argc) {
            baseline_file = argv[++i];
        } else if (arg == "--data-dir" && i + 1 < argc) {
            data_dir = argv[++i];
        } else if (arg == "--type" && i + 1 < argc) {
            type = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            output_file = argv[++i];
        } else if (arg == "--timestamp" && i + 1 < argc) {
            timestamp_override = argv[++i];
        }
    }
    
    if (action == "create") {
        if (!timestamp_override.empty()) {
            std::cout << "Using fixed timestamp: " << timestamp_override << std::endl;
        }
        BaselineManager::create_baseline(data_dir, baseline_file, type, timestamp_override);
    } else if (action == "diff") {
        auto baseline = BaselineManager::load_baseline(baseline_file);
        if (baseline.empty()) {
            std::cerr << "No baseline found at " << baseline_file << std::endl;
            std::cerr << "Run with 'create' action to create a baseline first." << std::endl;
            return 1;
        }
        
        FingerprintStorage storage(data_dir);
        auto current = storage.load_latest(type);
        
        auto diff_result = BaselineManager::compare(baseline, current);
        
        // Print results
        std::cout << "\n=== Baseline Diff Results ===\n\n";
        
        if (!diff_result.changed.empty()) {
            std::cout << "⚠️  CHANGED (" << diff_result.changed.size() << "):" << std::endl;
            for (const auto& record : diff_result.changed) {
                std::cout << "  " << record.type << " " << record.host << ":" << record.port << std::endl;
                std::cout << "    Baseline: " << baseline[std::make_tuple(record.type, record.host, record.port)] << std::endl;
                std::cout << "    Current:  " << record.fingerprint << std::endl;
                std::cout << std::endl;
            }
        }
        
        if (!diff_result.new_records.empty()) {
            std::cout << "➕ NEW (" << diff_result.new_records.size() << "):" << std::endl;
            for (const auto& record : diff_result.new_records) {
                std::cout << "  " << record.type << " " << record.host << ":" << record.port << ": " << record.fingerprint << std::endl;
            }
            std::cout << std::endl;
        }
        
        if (!diff_result.unchanged.empty()) {
            std::cout << "✅ UNCHANGED (" << diff_result.unchanged.size() << "):" << std::endl;
            for (const auto& record : diff_result.unchanged) {
                std::cout << "  " << record.type << " " << record.host << ":" << record.port << std::endl;
            }
            std::cout << std::endl;
        }
        
        // Save results if output specified
        if (!output_file.empty()) {
            std::ofstream file(output_file);
            file << "{\n";
            file << "  \"timestamp\": \"" << utils::resolve_timestamp(timestamp_override) << "\",\n";
            file << "  \"summary\": {\n";
            file << "    \"changed_count\": " << diff_result.changed.size() << ",\n";
            file << "    \"new_count\": " << diff_result.new_records.size() << ",\n";
            file << "    \"unchanged_count\": " << diff_result.unchanged.size() << "\n";
            file << "  }\n";
            file << "}\n";
            file.close();
            std::cout << "✓ Results saved to " << output_file << std::endl;
        }
        
        // Exit with error code if changes detected
        return diff_result.changed.empty() ? 0 : 1;
    } else {
        std::cerr << "Invalid action: " << action << std::endl;
        return 1;
    }
    
    return 0;
}

