#include <iostream>
#include <string>
#include <vector>

// cclint version
constexpr const char* VERSION = "0.1.0-alpha";

void print_version() {
    std::cout << "cclint version " << VERSION << std::endl;
    std::cout << "Customizable C++ Linter" << std::endl;
}

void print_help() {
    std::cout << "Usage: cclint [OPTIONS] <compiler-command>\n"
              << "\n"
              << "A customizable C++ linter with LuaJIT scripting support.\n"
              << "\n"
              << "Options:\n"
              << "  --help              Show this help message\n"
              << "  --version           Show version information\n"
              << "  --config=FILE       Specify configuration file\n"
              << "  --format=FORMAT     Output format: text, json, xml (default: text)\n"
              << "  -v, --verbose       Verbose output\n"
              << "  -q, --quiet         Quiet mode (errors only)\n"
              << "\n"
              << "Examples:\n"
              << "  cclint g++ -std=c++17 main.cpp\n"
              << "  cclint --config=.cclint.yaml g++ main.cpp\n"
              << "  cclint --format=json clang++ -std=c++20 src/*.cpp\n"
              << "\n"
              << "Configuration:\n"
              << "  Create a cclint.yaml file in your project root with:\n"
              << "  - lua_scripts: paths to Lua rule scripts\n"
              << "  - Standard scripts available in: ${CCLINT_HOME}/scripts/rules/\n"
              << "\n"
              << "For more information, visit:\n"
              << "  https://github.com/KikuchiTomo/cclint\n"
              << std::endl;
}

int main(int argc, char** argv) {
    // 引数を vector に変換
    std::vector<std::string> args(argv, argv + argc);

    // 引数が1つもない場合はヘルプを表示
    if (args.size() == 1) {
        print_help();
        return 0;
    }

    // 最初の引数をチェック
    std::string first_arg = args[1];

    if (first_arg == "--help" || first_arg == "-h") {
        print_help();
        return 0;
    }

    if (first_arg == "--version") {
        print_version();
        return 0;
    }

    // まだ実装されていないことを通知
    std::cout << "cclint is under active development.\n"
              << "\n"
              << "Current status:\n"
              << "  [x] Project documentation complete\n"
              << "  [x] 100+ Lua APIs designed\n"
              << "  [x] Standard script library structure defined\n"
              << "  [ ] Command-line parser (Milestone 1)\n"
              << "  [ ] Configuration loader (Milestone 1)\n"
              << "  [ ] C++ parser integration (Milestone 1)\n"
              << "  [ ] Rule engine (Milestone 2)\n"
              << "  [ ] LuaJIT integration (Milestone 3)\n"
              << "\n"
              << "See docs/milestones.md for the development roadmap.\n"
              << "Use --help to see the planned features.\n"
              << std::endl;

    return 0;
}
