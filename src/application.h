#pragma once

#include <atomic>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

#include "color.h"
#include "commands.h"
#include "config.h"
#include "connection.h"
#include "dbg.h"
#include "game_state.h"
#include "input.h"
#include "protocol.h"
#include "terminal_ui.h"

class Application {
public:
    explicit Application(const Config& cfg);
    ~Application();
    int run();

private:
    // Initialization
    void init_connection();
    void init_ui();
    void init_auto_login();
    void start_threads();

    // Main loop
    void save_terminal();
    void main_loop();
    void process_received();
    void cleanup_threads();
    void restore_terminal();

    // Members
    Config cfg_;
    std::string host_;
    int port_;

    Connection conn_;
    GameState state_;
    TerminalUI ui_;
    CommandHandler cmd_handler_;
    InputProcessor input_processor_;

    std::thread heartbeat_thread_;
    std::thread net_thread_;

    std::atomic<bool> running_{true};
};