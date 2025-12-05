#include "alert_manager.h"
#include "utils.h"
#include <iostream>
#include <sstream>
#include <fstream>
#ifdef HAVE_CURL
#include <curl/curl.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

AlertManager::AlertManager() {
    config_.console_enabled = true;
}

bool AlertManager::load_config(const std::string& config_file) {
    // Simple config file parsing (JSON-like)
    // For production, use a proper JSON library
    std::ifstream file(config_file);
    if (!file.is_open()) {
        return false;
    }
    
    // Basic parsing - in production use nlohmann/json or similar
    std::string line;
    while (std::getline(file, line)) {
        // Simple key=value parsing
        size_t eq_pos = line.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = utils::trim(line.substr(0, eq_pos));
            std::string value = utils::trim(line.substr(eq_pos + 1));
            
            if (key == "email_enabled") {
                config_.email_enabled = (value == "true" || value == "1");
            } else if (key == "email_smtp_server") {
                config_.email_smtp_server = value;
            } else if (key == "email_smtp_port") {
                config_.email_smtp_port = value;
            } else if (key == "email_username") {
                config_.email_username = value;
            } else if (key == "email_password") {
                config_.email_password = value;
            } else if (key == "webhook_enabled") {
                config_.webhook_enabled = (value == "true" || value == "1");
            } else if (key == "webhook_url") {
                config_.webhook_url = value;
            }
        }
    }
    
    return true;
}

void AlertManager::set_config(const AlertConfig& config) {
    config_ = config;
}

AlertConfig AlertManager::get_config() const {
    return config_;
}

bool AlertManager::send_alert(const AlertData& alert_data) {
    bool success = true;
    
    // Always print to console
    print_console_alert(alert_data);
    
    // Send email if enabled
    if (config_.email_enabled) {
        if (!send_email_alert(alert_data)) {
            std::cerr << "Warning: Failed to send email alert" << std::endl;
            success = false;
        }
    }
    
    // Send webhook if enabled
    if (config_.webhook_enabled) {
        if (!send_webhook_alert(alert_data)) {
            std::cerr << "Warning: Failed to send webhook alert" << std::endl;
            success = false;
        }
    }
    
    return success;
}

void AlertManager::print_console_alert(const AlertData& alert_data) {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    FINGERPRINT ALERT                      ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
    std::cout << "Title: " << alert_data.title << "\n";
    std::cout << "Time:  " << alert_data.timestamp << "\n";
    std::cout << "\n";
    
    if (!alert_data.message.empty()) {
        std::cout << alert_data.message << "\n\n";
    }
    
    if (!alert_data.changed_records.empty()) {
        std::cout << "⚠️  CHANGED FINGERPRINTS (" << alert_data.changed_records.size() << "):\n";
        for (const auto& record : alert_data.changed_records) {
            std::cout << "  " << record.type << " " << record.host << ":" << record.port 
                      << " -> " << record.fingerprint << "\n";
        }
        std::cout << "\n";
    }
    
    if (!alert_data.new_records.empty()) {
        std::cout << "➕ NEW FINGERPRINTS (" << alert_data.new_records.size() << "):\n";
        for (const auto& record : alert_data.new_records) {
            std::cout << "  " << record.type << " " << record.host << ":" << record.port 
                      << " -> " << record.fingerprint << "\n";
        }
        std::cout << "\n";
    }
    
    std::cout << "═══════════════════════════════════════════════════════════════\n\n";
}

std::string AlertManager::generate_alert_message(const AlertData& alert_data) {
    std::stringstream ss;
    
    ss << alert_data.title << "\n\n";
    ss << "Timestamp: " << alert_data.timestamp << "\n\n";
    
    if (!alert_data.message.empty()) {
        ss << alert_data.message << "\n\n";
    }
    
    if (!alert_data.changed_records.empty()) {
        ss << "CHANGED FINGERPRINTS (" << alert_data.changed_records.size() << "):\n";
        for (const auto& record : alert_data.changed_records) {
            ss << "  - " << record.type << " " << record.host << ":" << record.port 
               << " -> " << record.fingerprint << "\n";
        }
        ss << "\n";
    }
    
    if (!alert_data.new_records.empty()) {
        ss << "NEW FINGERPRINTS (" << alert_data.new_records.size() << "):\n";
        for (const auto& record : alert_data.new_records) {
            ss << "  - " << record.type << " " << record.host << ":" << record.port 
               << " -> " << record.fingerprint << "\n";
        }
        ss << "\n";
    }
    
    return ss.str();
}

bool AlertManager::send_email_alert(const AlertData& alert_data) {
    // Email sending using SMTP
    // For simplicity, this is a placeholder - in production, use libcurl with SMTP
    // or a dedicated email library
    
    std::cerr << "Email alerting not fully implemented (requires SMTP library)" << std::endl;
    std::cerr << "Consider using webhooks or implementing SMTP with libcurl" << std::endl;
    
    // TODO: Implement SMTP sending using libcurl
    return false;
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool AlertManager::send_webhook_alert(const AlertData& alert_data) {
#ifndef HAVE_CURL
    std::cerr << "Webhook alerting requires libcurl. Install libcurl-dev and rebuild." << std::endl;
    return false;
#else
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    
    // Generate JSON payload
    std::stringstream json_payload;
    json_payload << "{\n";
    json_payload << "  \"title\": \"" << alert_data.title << "\",\n";
    json_payload << "  \"timestamp\": \"" << alert_data.timestamp << "\",\n";
    json_payload << "  \"message\": \"" << alert_data.message << "\",\n";
    json_payload << "  \"changed_count\": " << alert_data.changed_records.size() << ",\n";
    json_payload << "  \"new_count\": " << alert_data.new_records.size() << ",\n";
    json_payload << "  \"changed\": [\n";
    
    for (size_t i = 0; i < alert_data.changed_records.size(); ++i) {
        const auto& record = alert_data.changed_records[i];
        json_payload << "    {\n";
        json_payload << "      \"type\": \"" << record.type << "\",\n";
        json_payload << "      \"host\": \"" << record.host << "\",\n";
        json_payload << "      \"port\": " << record.port << ",\n";
        json_payload << "      \"fingerprint\": \"" << record.fingerprint << "\"\n";
        json_payload << "    }";
        if (i < alert_data.changed_records.size() - 1) json_payload << ",";
        json_payload << "\n";
    }
    
    json_payload << "  ],\n";
    json_payload << "  \"new\": [\n";
    
    for (size_t i = 0; i < alert_data.new_records.size(); ++i) {
        const auto& record = alert_data.new_records[i];
        json_payload << "    {\n";
        json_payload << "      \"type\": \"" << record.type << "\",\n";
        json_payload << "      \"host\": \"" << record.host << "\",\n";
        json_payload << "      \"port\": " << record.port << ",\n";
        json_payload << "      \"fingerprint\": \"" << record.fingerprint << "\"\n";
        json_payload << "    }";
        if (i < alert_data.new_records.size() - 1) json_payload << ",";
        json_payload << "\n";
    }
    
    json_payload << "  ]\n";
    json_payload << "}\n";
    
    std::string payload = json_payload.str();
    std::string response;
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    // Add custom headers from config
    for (const auto& [key, value] : config_.webhook_headers) {
        std::string header = key + ": " + value;
        headers = curl_slist_append(headers, header.c_str());
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, config_.webhook_url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    CURLcode res = curl_easy_perform(curl);
    
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return (res == CURLE_OK && response_code >= 200 && response_code < 300);
#endif
}

