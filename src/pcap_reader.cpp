#include "pcap_reader.h"
#include <sstream>
#include <iomanip>
#include <cstring>

#ifdef HAVE_PCAP
#include <pcap/pcap.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#else
// Stub implementation when libpcap is not available
struct pcap {};
typedef struct pcap pcap_t;
struct pcap_pkthdr {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t caplen;
    uint32_t len;
};
#endif

PCAPReader::PCAPReader() : handle_(nullptr), is_open_(false) {
}

PCAPReader::~PCAPReader() {
    close();
}

bool PCAPReader::open_file(const std::string& filename) {
#ifdef HAVE_PCAP
    char errbuf[PCAP_ERRBUF_SIZE];
    handle_ = pcap_open_offline(filename.c_str(), errbuf);
    
    if (handle_ == nullptr) {
        error_msg_ = std::string("Failed to open PCAP file: ") + errbuf;
        is_open_ = false;
        return false;
    }
    
    is_open_ = true;
    error_msg_.clear();
    return true;
#else
    error_msg_ = "libpcap support not compiled in. Install libpcap and rebuild.";
    is_open_ = false;
    return false;
#endif
}

void PCAPReader::close() {
#ifdef HAVE_PCAP
    if (handle_ != nullptr) {
        pcap_close(handle_);
        handle_ = nullptr;
    }
#endif
    is_open_ = false;
}

bool PCAPReader::is_open() const {
    return is_open_;
}

std::string PCAPReader::get_error() const {
    return error_msg_;
}

bool PCAPReader::process_packets(TLSHandshakeCallback callback) {
#ifdef HAVE_PCAP
    if (!is_open_ || handle_ == nullptr) {
        error_msg_ = "PCAP file not open";
        return false;
    }
    
    struct pcap_pkthdr* header;
    const uint8_t* packet_data;
    int result;
    
    while ((result = pcap_next_ex(handle_, &header, &packet_data)) == 1) {
        // Check if packet is TCP
        if (!is_tcp_packet(packet_data, header->caplen)) {
            continue;
        }
        
        // Extract IP addresses and ports
        std::string src_ip, dst_ip;
        uint16_t src_port, dst_port;
        if (!extract_ip_ports(packet_data, header->caplen, src_ip, dst_ip, src_port, dst_port)) {
            continue;
        }
        
        // Check if it's TLS traffic (port 443 or other common TLS ports)
        if (dst_port != 443 && src_port != 443 && dst_port != 8443 && src_port != 8443) {
            continue;
        }
        
        // Extract TCP payload (TLS handshake)
        std::vector<uint8_t> handshake_data;
        bool is_client_hello = false, is_server_hello = false;
        
        if (extract_tls_handshake(packet_data, header->caplen, handshake_data, is_client_hello, is_server_hello)) {
            if (is_client_hello || is_server_hello) {
                TLSHandshakePacket handshake;
                handshake.handshake_data = handshake_data;
                handshake.src_port = src_port;
                handshake.dst_port = dst_port;
                handshake.src_ip = src_ip;
                handshake.dst_ip = dst_ip;
#ifdef HAVE_PCAP
                handshake.timestamp_sec = header->ts.tv_sec;
                handshake.timestamp_usec = header->ts.tv_usec;
#else
                handshake.timestamp_sec = header->ts_sec;
                handshake.timestamp_usec = header->ts_usec;
#endif
                handshake.is_client_hello = is_client_hello;
                handshake.is_server_hello = is_server_hello;
                
                callback(handshake);
            }
        }
    }
    
        if (result == -1) {
#ifdef HAVE_PCAP
        error_msg_ = std::string("Error reading PCAP: ") + pcap_geterr(handle_);
#else
        error_msg_ = "Error reading PCAP";
#endif
        return false;
    }
    
    return true;
#else
    error_msg_ = "libpcap support not compiled in";
    return false;
#endif
}

bool PCAPReader::is_tcp_packet(const uint8_t* packet, uint32_t packet_len) {
#ifdef HAVE_PCAP
    if (packet_len < 14) return false; // Minimum Ethernet header
    
    // Check for IPv4 (0x0800) or IPv6 (0x86DD)
    uint16_t ethertype = (packet[12] << 8) | packet[13];
    
    size_t ip_header_offset = 14;
    
    if (ethertype == 0x0800) { // IPv4
        if (packet_len < ip_header_offset + 20) return false;
        uint8_t ip_proto = packet[ip_header_offset + 9];
        return ip_proto == 6; // TCP protocol
    } else if (ethertype == 0x86DD) { // IPv6
        if (packet_len < ip_header_offset + 40) return false;
        uint8_t ip_proto = packet[ip_header_offset + 6];
        return ip_proto == 6; // TCP protocol
    }
    
    return false;
#else
    return false;
#endif
}

bool PCAPReader::extract_ip_ports(const uint8_t* packet, uint32_t packet_len,
                                  std::string& src_ip, std::string& dst_ip,
                                  uint16_t& src_port, uint16_t& dst_port) {
#ifdef HAVE_PCAP
    if (packet_len < 14) return false;
    
    uint16_t ethertype = (packet[12] << 8) | packet[13];
    size_t ip_offset = 14;
    bool is_ipv6 = (ethertype == 0x86DD);
    
    if (ethertype == 0x0800) { // IPv4
        if (packet_len < ip_offset + 20) return false;
        
        // Extract IP addresses
        src_ip = ip_to_string(packet + ip_offset + 12, false);
        dst_ip = ip_to_string(packet + ip_offset + 16, false);
        
        // Get IP header length
        uint8_t ip_header_len = (packet[ip_offset] & 0x0F) * 4;
        
        // TCP header starts after IP header
        size_t tcp_offset = ip_offset + ip_header_len;
        if (packet_len < tcp_offset + 4) return false;
        
        // Extract ports
        src_port = (packet[tcp_offset] << 8) | packet[tcp_offset + 1];
        dst_port = (packet[tcp_offset + 2] << 8) | packet[tcp_offset + 3];
        
        return true;
    } else if (ethertype == 0x86DD) { // IPv6
        if (packet_len < ip_offset + 40) return false;
        
        // Extract IPv6 addresses
        src_ip = ip_to_string(packet + ip_offset + 8, true);
        dst_ip = ip_to_string(packet + ip_offset + 24, true);
        
        // TCP header starts after IPv6 header (40 bytes)
        size_t tcp_offset = ip_offset + 40;
        if (packet_len < tcp_offset + 4) return false;
        
        // Extract ports
        src_port = (packet[tcp_offset] << 8) | packet[tcp_offset + 1];
        dst_port = (packet[tcp_offset + 2] << 8) | packet[tcp_offset + 3];
        
        return true;
    }
    
    return false;
#else
    return false;
#endif
}

bool PCAPReader::extract_tls_handshake(const uint8_t* packet, uint32_t packet_len,
                                       std::vector<uint8_t>& handshake_data,
                                       bool& is_client_hello, bool& is_server_hello) {
#ifdef HAVE_PCAP
    if (packet_len < 14) return false;
    
    // Find TCP payload
    uint16_t ethertype = (packet[12] << 8) | packet[13];
    size_t ip_offset = 14;
    size_t tcp_offset;
    
    if (ethertype == 0x0800) { // IPv4
        if (packet_len < ip_offset + 20) return false;
        uint8_t ip_header_len = (packet[ip_offset] & 0x0F) * 4;
        tcp_offset = ip_offset + ip_header_len;
    } else if (ethertype == 0x86DD) { // IPv6
        tcp_offset = ip_offset + 40;
    } else {
        return false;
    }
    
    if (packet_len < tcp_offset + 20) return false; // Minimum TCP header
    
    // Get TCP header length
    uint8_t tcp_header_len = ((packet[tcp_offset + 12] >> 4) & 0x0F) * 4;
    
    // TCP payload starts here
    size_t payload_offset = tcp_offset + tcp_header_len;
    uint32_t payload_len = packet_len - payload_offset;
    
    if (payload_len < 5) return false; // Minimum TLS record header
    
    // Check TLS record type (0x16 = Handshake)
    if (packet[payload_offset] != 0x16) return false;
    
    // Check TLS version (should be 0x0300-0x0304)
    uint16_t tls_version = (packet[payload_offset + 1] << 8) | packet[payload_offset + 2];
    if (tls_version < 0x0300 || tls_version > 0x0304) return false;
    
    // Get TLS record length
    uint16_t record_len = (packet[payload_offset + 3] << 8) | packet[payload_offset + 4];
    
    if (payload_len < 5 + record_len) return false;
    
    // Check handshake type (offset 5 from TLS record start)
    uint8_t handshake_type = packet[payload_offset + 5];
    is_client_hello = (handshake_type == 0x01);
    is_server_hello = (handshake_type == 0x02);
    
    if (!is_client_hello && !is_server_hello) return false;
    
    // Extract handshake data (skip TLS record header)
    handshake_data.assign(packet + payload_offset + 5, packet + payload_offset + 5 + record_len - 5);
    
    return true;
#else
    return false;
#endif
}

std::string PCAPReader::ip_to_string(const uint8_t* ip_addr, bool is_ipv6) {
#ifdef HAVE_PCAP
    char buffer[INET6_ADDRSTRLEN];
    
    if (is_ipv6) {
#ifdef _WIN32
        struct sockaddr_in6 sa6;
        memcpy(&sa6.sin6_addr, ip_addr, 16);
        inet_ntop(AF_INET6, &sa6.sin6_addr, buffer, INET6_ADDRSTRLEN);
#else
        inet_ntop(AF_INET6, ip_addr, buffer, INET6_ADDRSTRLEN);
#endif
    } else {
#ifdef _WIN32
        struct sockaddr_in sa;
        sa.sin_addr.s_addr = *reinterpret_cast<const uint32_t*>(ip_addr);
        inet_ntop(AF_INET, &sa.sin_addr, buffer, INET_ADDRSTRLEN);
#else
        inet_ntop(AF_INET, ip_addr, buffer, INET_ADDRSTRLEN);
#endif
    }
    
    return std::string(buffer);
#else
    return "";
#endif
}

