#include "lsp_server.hpp"
#include "utils/logger.hpp"

#include <iostream>

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // LSP通信用にログをファイルに出力
    utils::Logger::instance().set_log_file("/tmp/cclint-lsp.log");
    utils::Logger::instance().set_level(utils::LogLevel::Debug);

    utils::Logger::instance().info("cclint LSP Server starting...");

    cclint::lsp::LSPServer server;
    server.run(std::cin, std::cout);

    utils::Logger::instance().info("cclint LSP Server stopped");

    return 0;
}
