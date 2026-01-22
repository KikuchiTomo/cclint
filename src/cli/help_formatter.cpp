#include "help_formatter.hpp"

#include <sstream>

namespace cli {

constexpr const char* VERSION = "0.1.0-alpha";

std::string HelpFormatter::format_version() {
    std::ostringstream oss;
    oss << "cclint version " << VERSION << "\n";
    oss << "Customizable C++ Linter\n";
    return oss.str();
}

std::string HelpFormatter::format_help() {
    std::ostringstream oss;
    oss << format_usage();
    oss << "\n";
    oss << "A customizable C++ linter with LuaJIT scripting support.\n";
    oss << "\n";
    oss << format_options();
    oss << "\n";
    oss << format_examples();
    oss << "\n";
    oss << format_configuration();
    oss << "\n";
    oss << "For more information, visit:\n";
    oss << "  https://github.com/KikuchiTomo/cclint\n";
    return oss.str();
}

std::string HelpFormatter::format_usage() {
    return "Usage: cclint [OPTIONS] <compiler-command>\n";
}

std::string HelpFormatter::format_options() {
    std::ostringstream oss;
    oss << "Options:\n";
    oss << "  --help              Show this help message\n";
    oss << "  --version           Show version information\n";
    oss << "  --config=FILE       Specify configuration file\n";
    oss << "  --format=FORMAT     Output format: text, json, xml (default: "
           "text)\n";
    oss << "  -v, --verbose       Verbose output\n";
    oss << "  -q, --quiet         Quiet mode (errors only)\n";
    return oss.str();
}

std::string HelpFormatter::format_examples() {
    std::ostringstream oss;
    oss << "Examples:\n";
    oss << "  cclint g++ -std=c++17 main.cpp\n";
    oss << "  cclint --config=.cclint.yaml g++ main.cpp\n";
    oss << "  cclint --format=json clang++ -std=c++20 src/*.cpp\n";
    return oss.str();
}

std::string HelpFormatter::format_configuration() {
    std::ostringstream oss;
    oss << "Configuration:\n";
    oss << "  Create a cclint.yaml file in your project root with:\n";
    oss << "  - lua_scripts: paths to Lua rule scripts\n";
    oss << "  - Standard scripts available in: ${CCLINT_HOME}/scripts/rules/\n";
    return oss.str();
}

}  // namespace cli
