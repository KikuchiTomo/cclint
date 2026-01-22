#include "argument_parser.hpp"

#include <algorithm>
#include <stdexcept>

namespace cli {

ParsedArguments ArgumentParser::parse(int argc, char** argv) {
    ParsedArguments result;
    std::vector<std::string> args(argv + 1, argv + argc);

    if (args.empty()) {
        return result;
    }

    // cclintオプションを抽出
    parse_cclint_options(args, result);

    // 残りをコンパイラコマンドとして扱う
    result.compiler_command = extract_compiler_command(args);

    return result;
}

void ArgumentParser::parse_cclint_options(std::vector<std::string>& args,
                                          ParsedArguments& result) {
    auto it = args.begin();
    while (it != args.end()) {
        if (*it == "--config") {
            if (++it == args.end()) {
                throw std::runtime_error("--config requires an argument");
            }
            result.config_file = *it;
            it = args.erase(--it, ++it);
        } else if (it->rfind("--config=", 0) == 0) {
            result.config_file = it->substr(9);
            it = args.erase(it);
        } else if (*it == "--format") {
            if (++it == args.end()) {
                throw std::runtime_error("--format requires an argument");
            }
            result.output_format = *it;
            it = args.erase(--it, ++it);
        } else if (it->rfind("--format=", 0) == 0) {
            result.output_format = it->substr(9);
            it = args.erase(it);
        } else if (*it == "-v" || *it == "--verbose") {
            result.verbosity = 2;
            it = args.erase(it);
        } else if (*it == "-q" || *it == "--quiet") {
            result.verbosity = 0;
            it = args.erase(it);
        } else if (*it == "--help" || *it == "-h") {
            result.show_help = true;
            it = args.erase(it);
        } else if (*it == "--version") {
            result.show_version = true;
            it = args.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<std::string> ArgumentParser::extract_compiler_command(
    const std::vector<std::string>& args) {
    return args;  // 残りの引数がすべてコンパイラコマンド
}

bool ArgumentParser::is_cclint_option(const std::string& arg) {
    return arg.rfind("--config", 0) == 0 || arg.rfind("--format", 0) == 0 ||
           arg == "-v" || arg == "--verbose" || arg == "-q" ||
           arg == "--quiet" || arg == "--help" || arg == "-h" ||
           arg == "--version";
}

}  // namespace cli
