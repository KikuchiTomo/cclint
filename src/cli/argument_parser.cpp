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

void ArgumentParser::parse_cclint_options(std::vector<std::string>& args, ParsedArguments& result) {
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
        // Verbose flags
        } else if (*it == "--verbose-compiler" || *it == "--vc") {
            result.verbose.compiler = true;
            it = args.erase(it);
        } else if (*it == "--verbose-rules" || *it == "--vr") {
            result.verbose.rules = true;
            it = args.erase(it);
        } else if (*it == "--verbose-progress" || *it == "--vp") {
            result.verbose.progress = true;
            it = args.erase(it);
        } else if (*it == "--verbose-all" || *it == "--va") {
            result.verbose.enable_all();
            it = args.erase(it);
        } else if (*it == "--debug" || *it == "-d") {
            result.verbose.debug = true;
            result.verbose.enable_all();
            it = args.erase(it);
        } else if (*it == "--help" || *it == "-h") {
            result.show_help = true;
            it = args.erase(it);
        } else if (*it == "--version") {
            result.show_version = true;
            it = args.erase(it);
        } else if (it->rfind("--max-errors=", 0) == 0) {
            result.max_errors = std::stoi(it->substr(13));
            it = args.erase(it);
        } else if (it->rfind("--jobs=", 0) == 0) {
            result.num_threads = std::stoi(it->substr(7));
            it = args.erase(it);
        } else if (it->rfind("-j", 0) == 0 && it->size() > 2) {
            result.num_threads = std::stoi(it->substr(2));
            it = args.erase(it);
        } else if (*it == "--no-cache") {
            result.enable_cache = false;
            it = args.erase(it);
        } else if (*it == "--profile") {
            result.enable_profile = true;
            it = args.erase(it);
        } else if (*it == "--fix") {
            result.enable_fix = true;
            it = args.erase(it);
        } else if (*it == "--fix-preview") {
            result.fix_preview = true;
            it = args.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<std::string>
ArgumentParser::extract_compiler_command(const std::vector<std::string>& args) {
    return args;  // 残りの引数がすべてコンパイラコマンド
}

bool ArgumentParser::is_cclint_option(const std::string& arg) {
    return arg.rfind("--config", 0) == 0 || arg.rfind("--format", 0) == 0 ||
           arg.rfind("--verbose", 0) == 0 || arg.rfind("--vc", 0) == 0 ||
           arg.rfind("--vr", 0) == 0 || arg.rfind("--vp", 0) == 0 ||
           arg.rfind("--va", 0) == 0 || arg == "--debug" || arg == "-d" ||
           arg == "--help" || arg == "-h" || arg == "--version";
}

}  // namespace cli
