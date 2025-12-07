#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace utils {

// Get current timestamp in ISO 8601 format
std::string get_current_timestamp();

// Return a fixed timestamp when provided (CLI/env) or fall back to now
std::string resolve_timestamp(const std::string& fixed_timestamp = "");

// Format fingerprint as colon-separated hex string
std::string format_fingerprint(const std::vector<uint8_t>& fingerprint);

// Parse host:port string
bool parse_host_port(const std::string& input, std::string& host, int& port);

// Split string by delimiter
std::vector<std::string> split(const std::string& str, char delimiter);

// Trim whitespace
std::string trim(const std::string& str);

// Convert to uppercase
std::string to_upper(const std::string& str);

// Convert to lowercase
std::string to_lower(const std::string& str);

// Check if file exists
bool file_exists(const std::string& path);

// Create directory if it doesn't exist
bool create_directory(const std::string& path);

} // namespace utils

#endif // UTILS_H

