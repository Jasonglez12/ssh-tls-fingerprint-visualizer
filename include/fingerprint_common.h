#ifndef FINGERPRINT_COMMON_H
#define FINGERPRINT_COMMON_H

#include <string>
#include <vector>
#include <fstream>
#include <map>

struct FingerprintRecord {
    std::string timestamp;
    std::string type;
    std::string host;
    int port;
    std::string fingerprint;
};

// CSV operations
class CSVWriter {
public:
    CSVWriter(const std::string& filename);
    ~CSVWriter();
    
    void write_header();
    void write_record(const FingerprintRecord& record);
    
private:
    std::ofstream file;
    bool header_written;
};

class CSVReader {
public:
    CSVReader(const std::string& filename);
    ~CSVReader();
    
    std::vector<FingerprintRecord> read_all();
    bool is_open() const { return file.is_open(); }
    
private:
    std::ifstream file;
};

// JSON operations (simple JSON writer - for production use nlohmann/json)
class JSONWriter {
public:
    static void append_record(const std::string& filename, const FingerprintRecord& record);
    static std::vector<FingerprintRecord> read_all(const std::string& filename);
};

// Storage operations
class FingerprintStorage {
public:
    FingerprintStorage(const std::string& data_dir = "data");
    
    void save(const FingerprintRecord& record);
    std::vector<FingerprintRecord> load_all(const std::string& type = "");
    std::map<std::tuple<std::string, std::string, int>, std::string> load_latest(const std::string& type = "");
    
private:
    std::string data_dir;
    std::string get_csv_path(const std::string& type) const;
    std::string get_json_path(const std::string& type) const;
};

#endif // FINGERPRINT_COMMON_H

