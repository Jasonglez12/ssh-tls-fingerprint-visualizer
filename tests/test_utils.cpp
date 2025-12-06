// Unit tests for utils.cpp
#include <cassert>
#include <iostream>
#include "../include/utils.h"

void test_format_fingerprint() {
    std::vector<uint8_t> hash = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    std::string result = utils::format_fingerprint(hash);
    assert(result == "AA:BB:CC:DD:EE:FF");
    std::cout << "✓ test_format_fingerprint passed" << std::endl;
}

void test_parse_host_port() {
    std::string host;
    int port;
    
    // Test with port
    assert(utils::parse_host_port("example.com:443", host, port));
    assert(host == "example.com");
    assert(port == 443);
    
    // Test without port
    assert(utils::parse_host_port("example.com", host, port));
    assert(host == "example.com");
    assert(port == -1);
    
    std::cout << "✓ test_parse_host_port passed" << std::endl;
}

void test_split() {
    auto tokens = utils::split("a,b,c", ',');
    assert(tokens.size() == 3);
    assert(tokens[0] == "a");
    assert(tokens[1] == "b");
    assert(tokens[2] == "c");
    std::cout << "✓ test_split passed" << std::endl;
}

void test_trim() {
    assert(utils::trim("  hello  ") == "hello");
    assert(utils::trim("\t\nworld\n\t") == "world");
    std::cout << "✓ test_trim passed" << std::endl;
}

int main() {
    std::cout << "Running utils tests..." << std::endl;
    try {
        test_format_fingerprint();
        test_parse_host_port();
        test_split();
        test_trim();
        std::cout << "\nAll utils tests passed!" << std::endl;
        return 0;
    } catch (...) {
        std::cerr << "Tests failed!" << std::endl;
        return 1;
    }
}

