#include "cli/argument_parser.hpp"
#include "cli/help_formatter.hpp"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    try {
        // コマンドライン引数解析
        cli::ArgumentParser parser;
        auto args = parser.parse(argc, argv);

        // --help
        if (args.show_help || (argc == 1)) {
            std::cout << cli::HelpFormatter::format_help();
            return 0;
        }

        // --version
        if (args.show_version) {
            std::cout << cli::HelpFormatter::format_version();
            return 0;
        }

        // まだ実装されていないことを通知
        std::cout << "cclint is under active development.\n"
                  << "\n"
                  << "Current status:\n"
                  << "  [x] Project documentation complete\n"
                  << "  [x] 100+ Lua APIs designed\n"
                  << "  [x] Standard script library structure defined\n"
                  << "  [x] Command-line parser implemented\n"
                  << "  [ ] Configuration loader (Milestone 1)\n"
                  << "  [ ] C++ parser integration (Milestone 1)\n"
                  << "  [ ] Rule engine (Milestone 2)\n"
                  << "  [ ] LuaJIT integration (Milestone 3)\n"
                  << "\n";

        if (args.config_file) {
            std::cout << "Config file: " << *args.config_file << "\n";
        }
        std::cout << "Output format: " << args.output_format << "\n";
        std::cout << "Verbosity: " << args.verbosity << "\n";

        if (!args.compiler_command.empty()) {
            std::cout << "Compiler command: ";
            for (const auto& arg : args.compiler_command) {
                std::cout << arg << " ";
            }
            std::cout << "\n";
        }

        std::cout << "\n"
                  << "See docs/milestones.md for the development roadmap.\n"
                  << "Use --help to see the planned features.\n";

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }
}
