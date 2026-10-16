#include "application.h"

#include <chrono>
#include <iostream>

#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#endif

#include "color.h"
#include "dbg.h"

Application::Application(const Config& cfg)
    : cfg_(cfg),
      conn_(),
      state_(conn_),
      ui_(),
      cmd_handler_(state_, conn_, ui_),
      input_processor_(cmd_handler_, state_, conn_, ui_),
      running_(true)
{
    host_ = cfg.host;
    port_ = cfg.port;
    ui_.set_logs_dir(cfg.logs_dir);
}

Application::~Application()
{
    // Cleanup handled by run()
}

int Application::run()
{
    DBG("Built at " << __DATE__ << " " << __TIME__);

    // Initialization
    init_connection();
    init_auto_login();
    start_threads();

    // Main loop
    main_loop();

    // Cleanup
    cleanup_threads();
    return 0;
}

void Application::init_connection()
{
    ui_.add_line(color::convert("$HIG【系统】正在连接服务器 " + host_ + ":"
                                + std::to_string(port_) + "...$COM"));

    if (!conn_.connect(host_, port_)) {
        ui_.add_line(color::convert("$RED【系统】连接失败！$COM"));
        std::this_thread::sleep_for(std::chrono::seconds(2));
        throw std::runtime_error("Connection failed");
    }
    ui_.add_line(color::convert("$GRN【系统】连接成功.$COM"));
}

void Application::init_auto_login()
{
    if (!cfg_.account.empty() && !cfg_.password.empty()) {
        conn_.send_login(cfg_.account, cfg_.password);
        ui_.add_line("Auto-login as " + cfg_.account + "...");
        state_.set_account_id(cfg_.account);
    }
    else {
        ui_.add_line("Please enter login credentials:");
        ui_.add_line("  Enter /login <account> <password>");
    }
}

void Application::start_threads()
{
    // Heartbeat thread
    heartbeat_thread_ = std::thread([this]() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::seconds(15));
            if (!running_) break;
            std::string tune_cmd
                = std::to_string(state_.player_id()) + " tune 0 reducedrop\t";
            conn_.send_command_utf8(tune_cmd);
        }
    });

    // Network thread
    net_thread_ = std::thread([this]() {
        while (running_) {
            process_received();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });
}

void Application::save_terminal()
{
#ifdef _WIN32
    // Windows 控制台默认即为行输入+回显，无需设置。
#else
    struct termios t;
    if (tcgetattr(STDIN_FILENO, &t) == 0) {
        t.c_lflag |= ICANON | ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
    }
#endif
}

void Application::main_loop()
{
    save_terminal();

    std::string line;
    std::cout << "> ";
    while (running_ && std::getline(std::cin, line)) {
        bool keep = input_processor_.process_line(line);
        if (!keep || input_processor_.should_quit()) {
            running_ = false;
            ui_.shutdown();
        }
        process_received();
        std::cout << "> ";
    }
    restore_terminal();
}

void Application::process_received()
{
    auto messages = conn_.drain_received();
    for (const auto& raw : messages) {
        auto decoded = protocol::decode_str(raw);
        auto utf8    = protocol::gbk_to_utf8(decoded);
        auto msg     = protocol::parse_message(utf8);
        if (msg) {
            auto lines = state_.handle_message(*msg);
            for (const auto& line : lines) ui_.add_line(line);
        }

        // Check if server closed connection
        if (conn_.is_eof()) {
            ui_.add_line(
                color::convert("$REDServer closed the connection.$COM"));
            running_ = false;
        }
    }
}

void Application::cleanup_threads()
{
    running_ = false;
    conn_.disconnect();
    if (heartbeat_thread_.joinable()) heartbeat_thread_.detach();
    if (net_thread_.joinable()) net_thread_.join();
}

void Application::restore_terminal()
{
#ifdef _WIN32
    // Windows 控制台默认即为行输入+回显，无需恢复。
#else
    struct termios t;
    if (tcgetattr(STDIN_FILENO, &t) == 0) {
        t.c_lflag |= ICANON | ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
    }
#endif
}