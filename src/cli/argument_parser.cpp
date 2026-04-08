#include "argument_parser.hpp"

#include "option_parser.hpp"

namespace cli {

ParsedArguments ArgumentParser::parse(int argc, char** argv) {
    ParsedArguments args;

    // Temporary flags for verbose-all and debug (which set multiple flags)
    bool verbose_all = false;
    bool debug_mode = false;
    bool no_cache = false;

    OptionParser opts;
    opts.set_program("cclint", "0.1.0-alpha");

    opts.group("General");
    opts.flag(args.show_help, {"--help", "-h"}, "Show this help message");
    opts.flag(args.show_version, {"--version"}, "Show version information");
    opts.option(args.config_file, {"--config"}, "Configuration file", "FILE");
    opts.option(args.output_format, {"--format"}, "Output format: text, json, xml", "FORMAT");
    opts.option(args.max_errors, {"--max-errors"}, "Stop after N errors (default: unlimited)", "N");
    opts.option(args.num_threads, {"--jobs", "-j"}, "Number of parallel jobs (default: auto)", "N");
    opts.flag(no_cache, {"--no-cache"}, "Disable caching");
    opts.flag(args.enable_fix, {"--fix"}, "Enable auto-fix mode");
    opts.flag(args.fix_preview, {"--fix-preview"}, "Show fix preview without applying");
    opts.flag(args.enable_profile, {"--profile"}, "Show detailed performance stats");

    opts.group("Verbosity");
    opts.flag(args.verbose.compiler, {"--verbose-compiler", "--vc"},
              "Show compiler warnings/errors");
    opts.flag(args.verbose.rules, {"--verbose-rules", "--vr"}, "Show rule loading info");
    opts.flag(args.verbose.progress, {"--verbose-progress", "--vp"},
              "Show analysis progress/stats");
    opts.flag(verbose_all, {"--verbose-all", "--va"}, "Enable all verbose output");
    opts.flag(debug_mode, {"--debug", "-d"}, "Debug mode (all output + internals)");

    args.compiler_command = opts.parse(argc, argv);

    // Post-parse: handle composite flags
    if (no_cache) {
        args.enable_cache = false;
    }
    if (verbose_all) {
        args.verbose.enable_all();
    }
    if (debug_mode) {
        args.verbose.debug = true;
        args.verbose.enable_all();
    }

    // Store generated help/version text
    args.help_text = opts.help("A customizable C++ linter with LuaJIT scripting support.");
    args.version_text = opts.version_string();

    return args;
}

}  // namespace cli
