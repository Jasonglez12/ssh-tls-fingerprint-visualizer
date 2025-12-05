#include "pcap_reader.h"
#include "ja3.h"
#include "fingerprint_common.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <algorithm>

struct JA3Record {
    std::string ja3;
    std::string ja3s;
    std::string src_ip;
    std::string dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    std::string timestamp;
    std::string sni; // Server Name Indication
};

void save_ja3_record(const JA3Record& record, const std::string& data_dir) {
    // Create extended fingerprint record
    FingerprintRecord fp_record;
    fp_record.timestamp = record.timestamp;
    fp_record.type = "JA3";
    fp_record.host = record.dst_ip;
    fp_record.port = record.dst_port;
    fp_record.fingerprint = record.ja3;
    
    FingerprintStorage storage(data_dir);
    storage.save(fp_record);
    
    // Also save JA3S if available
    if (!record.ja3s.empty()) {
        FingerprintRecord ja3s_record;
        ja3s_record.timestamp = record.timestamp;
        ja3s_record.type = "JA3S";
        ja3s_record.host = record.dst_ip;
        ja3s_record.port = record.dst_port;
        ja3s_record.fingerprint = record.ja3s;
        storage.save(ja3s_record);
    }
}

std::string format_timestamp(uint32_t sec, uint32_t usec) {
    std::time_t time = static_cast<std::time_t>(sec);
    std::tm* tm = std::gmtime(&time);
    
    std::stringstream ss;
    ss << std::put_time(tm, "%Y-%m-%dT%H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(6) << usec << "Z";
    return ss.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <pcap_file> [options]" << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "  --data-dir DIR      Data directory (default: data)" << std::endl;
        std::cerr << "  --output FORMAT     Output format: json, csv, or both (default: both)" << std::endl;
        std::cerr << "  --stats             Print statistics" << std::endl;
        return 1;
    }
    
    std::string pcap_file = argv[1];
    std::string data_dir = "data";
    std::string output_format = "both";
    bool print_stats = false;
    
    // Parse arguments
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--data-dir" && i + 1 < argc) {
            data_dir = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            output_format = argv[++i];
        } else if (arg == "--stats") {
            print_stats = true;
        }
    }
    
    // Open PCAP file
    PCAPReader reader;
    if (!reader.open_file(pcap_file)) {
        std::cerr << "Error: " << reader.get_error() << std::endl;
        return 1;
    }
    
    std::cout << "Processing PCAP file: " << pcap_file << std::endl;
    std::cout << "Extracting JA3/JA3S fingerprints..." << std::endl;
    
    std::vector<JA3Record> records;
    std::map<std::string, int> ja3_counts;
    std::map<std::string, int> ja3s_counts;
    std::map<std::string, std::string> client_hellos; // IP:Port -> JA3
    std::map<std::string, std::string> server_hellos; // IP:Port -> JA3S
    
    // Process packets
    reader.process_packets([&](const TLSHandshakePacket& packet) {
        JA3Record record;
        record.src_ip = packet.src_ip;
        record.dst_ip = packet.dst_ip;
        record.src_port = packet.src_port;
        record.dst_port = packet.dst_port;
        record.timestamp = format_timestamp(packet.timestamp_sec, packet.timestamp_usec);
        
        if (packet.is_client_hello) {
            TLSClientHello client_hello;
            if (ja3::parse_client_hello(packet.handshake_data, client_hello)) {
                record.ja3 = ja3::calculate_ja3(client_hello);
                record.sni = client_hello.sni;
                
                // Store client hello for pairing
                std::string key = packet.src_ip + ":" + std::to_string(packet.src_port) + 
                                 "->" + packet.dst_ip + ":" + std::to_string(packet.dst_port);
                client_hellos[key] = record.ja3;
                
                ja3_counts[record.ja3]++;
            }
        } else if (packet.is_server_hello) {
            TLSServerHello server_hello;
            if (ja3::parse_server_hello(packet.handshake_data, server_hello)) {
                record.ja3s = ja3::calculate_ja3s(server_hello);
                
                // Try to pair with client hello
                std::string key = packet.dst_ip + ":" + std::to_string(packet.dst_port) + 
                                 "->" + packet.src_ip + ":" + std::to_string(packet.src_port);
                if (client_hellos.find(key) != client_hellos.end()) {
                    record.ja3 = client_hellos[key];
                }
                
                ja3s_counts[record.ja3s]++;
            }
        }
        
        if (!record.ja3.empty() || !record.ja3s.empty()) {
            records.push_back(record);
            
            // Save to storage
            if (output_format == "json" || output_format == "both") {
                save_ja3_record(record, data_dir);
            }
        }
    });
    
    reader.close();
    
    // Print results
    std::cout << "\n✓ Processed " << records.size() << " TLS handshake packets" << std::endl;
    std::cout << "  Found " << ja3_counts.size() << " unique JA3 fingerprints" << std::endl;
    std::cout << "  Found " << ja3s_counts.size() << " unique JA3S fingerprints" << std::endl;
    
    // Print statistics if requested
    if (print_stats) {
        std::cout << "\n=== JA3 Fingerprint Statistics ===\n" << std::endl;
        
        // Most common JA3 fingerprints
        std::vector<std::pair<std::string, int>> ja3_sorted;
        for (const auto& [ja3, count] : ja3_counts) {
            ja3_sorted.push_back({ja3, count});
        }
        std::sort(ja3_sorted.begin(), ja3_sorted.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        std::cout << "Top 10 JA3 fingerprints:" << std::endl;
        int limit = std::min(10, static_cast<int>(ja3_sorted.size()));
        for (int i = 0; i < limit; ++i) {
            std::cout << "  " << std::setw(5) << ja3_sorted[i].second << "x  " 
                      << ja3_sorted[i].first << std::endl;
        }
        
        // Most common JA3S fingerprints
        std::vector<std::pair<std::string, int>> ja3s_sorted;
        for (const auto& [ja3s, count] : ja3s_counts) {
            ja3s_sorted.push_back({ja3s, count});
        }
        std::sort(ja3s_sorted.begin(), ja3s_sorted.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        std::cout << "\nTop 10 JA3S fingerprints:" << std::endl;
        limit = std::min(10, static_cast<int>(ja3s_sorted.size()));
        for (int i = 0; i < limit; ++i) {
            std::cout << "  " << std::setw(5) << ja3s_sorted[i].second << "x  " 
                      << ja3s_sorted[i].first << std::endl;
        }
    }
    
    std::cout << "\n✓ Fingerprints saved to " << data_dir << std::endl;
    
    return 0;
}

