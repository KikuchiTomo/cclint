#pragma once

#include <optional>
#include <string>
#include <vector>

namespace cli {

struct VerboseFlags {
    bool compiler = false;  // --verbose-compiler / --vc : show compiler output
    bool rules = false;     // --verbose-rules / --vr    : show rule loading
    bool progress = false;  // --verbose-progress / --vp : show analysis progress
    bool debug = false;     // --debug / -d              : show everything

    // --verbose-all / --va sets all info flags
    void enable_all() {
        compiler = true;
        rules = true;
        progress = true;
    }
};

struct ParsedArguments {
    std::optional<std::string> config_file;
    std::vector<std::string> compiler_command;
    std::string output_format = "text";
    VerboseFlags verbose;
    bool show_help = false;
    bool show_version = false;
    int max_errors = 0;   // 0 = unlimited
    int num_threads = 0;  // 0 = auto-detect
    bool enable_cache = true;
    bool enable_profile = false;
    bool enable_fix = false;
    bool fix_preview = false;

    // Auto-generated help and version text
    std::string help_text;
    std::string version_text;
};

class ArgumentParser {
public:
    ParsedArguments parse(int argc, char** argv);
};

}  // namespace cli
