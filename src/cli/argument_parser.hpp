#pragma once

#include <optional>
#include <string>
#include <vector>

namespace cli {

struct ParsedArguments {
    std::optional<std::string> config_file;
    std::vector<std::string> compiler_command;
    std::string output_format = "text";
    int verbosity = 1;  // 0: quiet, 1: normal, 2: verbose
    bool show_help = false;
    bool show_version = false;
};

class ArgumentParser {
public:
    ParsedArguments parse(int argc, char** argv);

private:
    void parse_cclint_options(std::vector<std::string>& args,
                               ParsedArguments& result);

    bool is_cclint_option(const std::string& arg);
    std::vector<std::string> extract_compiler_command(
        const std::vector<std::string>& args);
};

}  // namespace cli
