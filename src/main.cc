#include <iostream>
#include <string>

#include "application.h"
#include "compat.h"
#include "config.h"
#include "dbg.h"

/** 解析命令行参数：-h/--help、-d/--dbg、-c/--config <file>。
 *  返回 0 继续运行，1 应直接退出（help 或错误）。 */
static int parse_args(int argc, char *argv[], Config &cfg, bool &show_help,
                      int &exit_code)
{
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            show_help = true;
            exit_code = 0;
            return 1;
        }
        if (arg == "-d" || arg == "--dbg") {
            dbg_enable(true);
            continue;
        }
        if (arg == "-c" || arg == "--config") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << arg << "\n";
                exit_code = 1;
                return 1;
            }
            cfg = load_config(argv[++i]);
            continue;
        }
        if (arg.rfind("--config=", 0) == 0) {
            cfg = load_config(arg.substr(9));
            continue;
        }
        if (!arg.empty() && arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << "\n";
            exit_code = 1;
            return 1;
        }
        std::cout << "Positional arg: " << arg << "\n";
    }
    return 0;
}

int main(int argc, char *argv[])
{
    init_console();

    Config cfg = load_config();

    bool show_help = false;
    int exit_code  = 0;
    if (parse_args(argc, argv, cfg, show_help, exit_code)) {
        if (show_help) {
            std::cout << "Usage: " << argv[0]
                      << " [-h] [--dbg] [--config file]\n";
        }
        return exit_code;
    }
    DBG("Built at " << __DATE__ << " " << __TIME__);

    try {
        Application app(cfg);
        return app.run();
    } catch (const std::exception &e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}
