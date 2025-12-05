#ifndef PCAP_READER_H
#define PCAP_READER_H

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

// Forward declaration for libpcap types
struct pcap;
typedef struct pcap pcap_t;
struct pcap_pkthdr;

// TLS handshake data extracted from PCAP
struct TLSHandshakePacket {
    std::vector<uint8_t> handshake_data;
    uint16_t src_port;
    uint16_t dst_port;
    std::string src_ip;
    std::string dst_ip;
    uint32_t timestamp_sec;
    uint32_t timestamp_usec;
    bool is_client_hello;
    bool is_server_hello;
};

// Callback function type for TLS handshake packets
using TLSHandshakeCallback = std::function<void(const TLSHandshakePacket&)>;

class PCAPReader {
public:
    PCAPReader();
    ~PCAPReader();
    
    // Open PCAP file for reading
    bool open_file(const std::string& filename);
    
    // Close PCAP file
    void close();
    
    // Check if PCAP file is open
    bool is_open() const;
    
    // Process all packets and call callback for each TLS handshake
    bool process_packets(TLSHandshakeCallback callback);
    
    // Get error message (if any)
    std::string get_error() const;

private:
    pcap_t* handle_;
    std::string error_msg_;
    bool is_open_;
    
    // Helper: Check if packet is TCP
    bool is_tcp_packet(const uint8_t* packet, uint32_t packet_len);
    
    // Helper: Extract IP addresses and ports from packet
    bool extract_ip_ports(const uint8_t* packet, uint32_t packet_len,
                         std::string& src_ip, std::string& dst_ip,
                         uint16_t& src_port, uint16_t& dst_port);
    
    // Helper: Extract TLS handshake from TCP payload
    bool extract_tls_handshake(const uint8_t* tcp_payload, uint32_t payload_len,
                              std::vector<uint8_t>& handshake_data,
                              bool& is_client_hello, bool& is_server_hello);
    
    // Helper: Convert IP address to string
    std::string ip_to_string(const uint8_t* ip_addr, bool is_ipv6);
};

#endif // PCAP_READER_H

