#include "utils.h"
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace utils {

std::string get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count() << "Z";
    return ss.str();
}

std::string format_fingerprint(const std::vector<uint8_t>& fingerprint) {
    std::stringstream ss;
    ss << std::hex << std::uppercase;
    for (size_t i = 0; i < fingerprint.size(); ++i) {
        if (i > 0) ss << ':';
        ss << std::setfill('0') << std::setw(2) << static_cast<int>(fingerprint[i]);
    }
    return ss.str();
}

bool parse_host_port(const std::string& input, std::string& host, int& port) {
    size_t colon_pos = input.find_last_of(':');
    if (colon_pos == std::string::npos) {
        host = input;
        port = -1; // Default port will be set by caller
        return true;
    }
    
    host = input.substr(0, colon_pos);
    std::string port_str = input.substr(colon_pos + 1);
    
    try {
        port = std::stoi(port_str);
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::string to_upper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

bool file_exists(const std::string& path) {
    return std::filesystem::exists(path);
}

bool create_directory(const std::string& path) {
    try {
        std::filesystem::create_directories(path);
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace utils

