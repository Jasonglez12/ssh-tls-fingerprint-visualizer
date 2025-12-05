#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include "fingerprint_common.h"

struct AlertConfig {
    bool email_enabled = false;
    std::string email_smtp_server;
    std::string email_smtp_port;
    std::string email_username;
    std::string email_password;
    std::vector<std::string> email_recipients;
    
    bool webhook_enabled = false;
    std::string webhook_url;
    std::string webhook_method = "POST";
    std::map<std::string, std::string> webhook_headers;
    
    bool console_enabled = true; // Always enabled for CLI output
};

struct AlertData {
    std::string title;
    std::string message;
    std::vector<FingerprintRecord> changed_records;
    std::vector<FingerprintRecord> new_records;
    std::string timestamp;
};

class AlertManager {
public:
    AlertManager();
    
    // Load configuration from file or use defaults
    bool load_config(const std::string& config_file);
    
    // Send alert with diff data
    bool send_alert(const AlertData& alert_data);
    
    // Set configuration programmatically
    void set_config(const AlertConfig& config);
    
    // Get current configuration
    AlertConfig get_config() const;

private:
    AlertConfig config_;
    
    // Send email alert
    bool send_email_alert(const AlertData& alert_data);
    
    // Send webhook alert
    bool send_webhook_alert(const AlertData& alert_data);
    
    // Print console alert
    void print_console_alert(const AlertData& alert_data);
    
    // Generate alert message body
    std::string generate_alert_message(const AlertData& alert_data);
};

#endif // ALERT_MANAGER_H

