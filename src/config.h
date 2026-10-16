#pragma once

#include <string>

struct Config {
    std::string host = "127.0.0.1";
    int port         = 6600;
    std::string account;
    std::string password;
    std::string logs_dir;   // Directory for log files, default  ~/.local/share/jianghu/logs
};

// Return the path to  ~/.local/share/jianghu/config or fall back to ./jianghu.conf
std::string config_path();

// Load configuration from file. Returns Config with defaults for any missing
// keys.
Config load_config(const std::string &path = "");
