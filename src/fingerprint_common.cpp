#include "fingerprint_common.h"
#include "utils.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>

// CSV Writer
CSVWriter::CSVWriter(const std::string& filename) : header_written(false) {
    file.open(filename, std::ios::app);
}

CSVWriter::~CSVWriter() {
    if (file.is_open()) {
        file.close();
    }
}

void CSVWriter::write_header() {
    if (!header_written && file.is_open()) {
        file << "timestamp,type,host,port,fingerprint\n";
        header_written = true;
    }
}

void CSVWriter::write_record(const FingerprintRecord& record) {
    if (!file.is_open()) return;
    
    if (!header_written) {
        write_header();
    }
    
    file << record.timestamp << ","
         << record.type << ","
         << record.host << ","
         << record.port << ","
         << record.fingerprint << "\n";
}

// CSV Reader
CSVReader::CSVReader(const std::string& filename) {
    file.open(filename);
}

CSVReader::~CSVReader() {
    if (file.is_open()) {
        file.close();
    }
}

std::vector<FingerprintRecord> CSVReader::read_all() {
    std::vector<FingerprintRecord> records;
    
    if (!file.is_open()) return records;
    
    std::string line;
    bool first_line = true;
    
    while (std::getline(file, line)) {
        if (first_line) {
            first_line = false;
            continue; // Skip header
        }
        
        if (line.empty()) continue;
        
        auto tokens = utils::split(line, ',');
        if (tokens.size() >= 5) {
            FingerprintRecord record;
            record.timestamp = tokens[0];
            record.type = tokens[1];
            record.host = tokens[2];
            try {
                record.port = std::stoi(tokens[3]);
            } catch (...) {
                continue;
            }
            record.fingerprint = tokens[4];
            records.push_back(record);
        }
    }
    
    return records;
}

// JSON Writer (simple implementation - for production use nlohmann/json)
void JSONWriter::append_record(const std::string& filename, const FingerprintRecord& record) {
    // Check if file exists and read existing records
    std::vector<FingerprintRecord> existing;
    if (std::filesystem::exists(filename)) {
        existing = read_all(filename);
    }
    
    // Add new record
    existing.push_back(record);
    
    // Write all records
    std::ofstream file(filename, std::ios::trunc);
    file << "[\n";
    
    for (size_t i = 0; i < existing.size(); ++i) {
        file << "  {\n";
        file << "    \"timestamp\": \"" << existing[i].timestamp << "\",\n";
        file << "    \"type\": \"" << existing[i].type << "\",\n";
        file << "    \"host\": \"" << existing[i].host << "\",\n";
        file << "    \"port\": " << existing[i].port << ",\n";
        file << "    \"fingerprint\": \"" << existing[i].fingerprint << "\"\n";
        file << "  }";
        if (i < existing.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "]\n";
    file.close();
}

std::vector<FingerprintRecord> JSONWriter::read_all(const std::string& filename) {
    std::vector<FingerprintRecord> records;
    
    if (!std::filesystem::exists(filename)) {
        return records;
    }
    
    std::ifstream file(filename);
    if (!file.is_open()) return records;
    
    // Simple JSON parser (basic implementation)
    std::string line;
    FingerprintRecord record;
    bool in_record = false;
    std::string key, value;
    
    while (std::getline(file, line)) {
        line = utils::trim(line);
        
        if (line == "{" || line.find("\"timestamp\"") != std::string::npos) {
            in_record = true;
            record = FingerprintRecord();
        }
        
        if (in_record) {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                key = utils::trim(line.substr(0, colon_pos));
                value = utils::trim(line.substr(colon_pos + 1));
                
                // Remove quotes
                if (!value.empty() && value.front() == '"' && value.back() == '"') {
                    value = value.substr(1, value.length() - 2);
                }
                // Remove trailing comma
                if (!value.empty() && value.back() == ',') {
                    value.pop_back();
                }
                
                if (key.find("timestamp") != std::string::npos) {
                    record.timestamp = value;
                } else if (key.find("type") != std::string::npos) {
                    record.type = value;
                } else if (key.find("host") != std::string::npos) {
                    record.host = value;
                } else if (key.find("port") != std::string::npos) {
                    try {
                        record.port = std::stoi(value);
                    } catch (...) {}
                } else if (key.find("fingerprint") != std::string::npos) {
                    record.fingerprint = value;
                }
            }
            
            if (line == "}" || line == "},") {
                records.push_back(record);
                in_record = false;
            }
        }
    }
    
    file.close();
    return records;
}

// Fingerprint Storage
FingerprintStorage::FingerprintStorage(const std::string& data_dir) : data_dir(data_dir) {
    utils::create_directory(data_dir);
}

void FingerprintStorage::save(const FingerprintRecord& record) {
    std::string csv_path = get_csv_path(record.type);
    std::string json_path = get_json_path(record.type);
    
    CSVWriter csv_writer(csv_path);
    csv_writer.write_record(record);
    
    JSONWriter::append_record(json_path, record);
}

std::vector<FingerprintRecord> FingerprintStorage::load_all(const std::string& type) {
    std::vector<FingerprintRecord> all_records;
    
    if (type == "TLS" || type.empty()) {
        std::string json_path = get_json_path("TLS");
        auto tls_records = JSONWriter::read_all(json_path);
        all_records.insert(all_records.end(), tls_records.begin(), tls_records.end());
    }
    
    if (type == "SSH" || type.empty()) {
        std::string json_path = get_json_path("SSH");
        auto ssh_records = JSONWriter::read_all(json_path);
        all_records.insert(all_records.end(), ssh_records.begin(), ssh_records.end());
    }
    
    return all_records;
}

std::map<std::tuple<std::string, std::string, int>, std::string> FingerprintStorage::load_latest(const std::string& type) {
    auto all_records = load_all(type);
    std::map<std::tuple<std::string, std::string, int>, std::pair<std::string, std::string>> latest; // timestamp -> fingerprint
    
    for (const auto& record : all_records) {
        auto key = std::make_tuple(record.type, record.host, record.port);
        
        if (latest.find(key) == latest.end() || record.timestamp > latest[key].first) {
            latest[key] = {record.timestamp, record.fingerprint};
        }
    }
    
    std::map<std::tuple<std::string, std::string, int>, std::string> result;
    for (const auto& [key, value] : latest) {
        result[key] = value.second;
    }
    
    return result;
}

std::string FingerprintStorage::get_csv_path(const std::string& type) const {
    return data_dir + "/" + utils::to_lower(type) + "_fingerprints.csv";
}

std::string FingerprintStorage::get_json_path(const std::string& type) const {
    return data_dir + "/" + utils::to_lower(type) + "_fingerprints.json";
}

