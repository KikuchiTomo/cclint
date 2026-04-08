#include "option_parser.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace cli {

OptionParser& OptionParser::group(const std::string& name) {
    current_group_ = name;
    return *this;
}

OptionParser& OptionParser::flag(bool& target, std::initializer_list<std::string> names,
                                 const std::string& description) {
    OptionDef def;
    def.names = names;
    def.description = description;
    def.group_name = current_group_;
    def.type = OptType::Flag;
    def.flag_target = &target;
    options_.push_back(std::move(def));
    return *this;
}

OptionParser& OptionParser::option(std::string& target, std::initializer_list<std::string> names,
                                   const std::string& description, const std::string& metavar) {
    OptionDef def;
    def.names = names;
    def.description = description;
    def.metavar = metavar;
    def.group_name = current_group_;
    def.type = OptType::String;
    def.string_target = &target;
    options_.push_back(std::move(def));
    return *this;
}

OptionParser& OptionParser::option(std::optional<std::string>& target,
                                   std::initializer_list<std::string> names,
                                   const std::string& description, const std::string& metavar) {
    OptionDef def;
    def.names = names;
    def.description = description;
    def.metavar = metavar;
    def.group_name = current_group_;
    def.type = OptType::OptionalString;
    def.opt_string_target = &target;
    options_.push_back(std::move(def));
    return *this;
}

OptionParser& OptionParser::option(int& target, std::initializer_list<std::string> names,
                                   const std::string& description, const std::string& metavar) {
    OptionDef def;
    def.names = names;
    def.description = description;
    def.metavar = metavar;
    def.group_name = current_group_;
    def.type = OptType::Int;
    def.int_target = &target;
    options_.push_back(std::move(def));
    return *this;
}

void OptionParser::set_program(const std::string& name, const std::string& version) {
    program_name_ = name;
    program_version_ = version;
}

std::string OptionParser::version_string() const {
    std::ostringstream oss;
    oss << program_name_ << " version " << program_version_ << "\n";
    oss << "Customizable C++ Linter\n";
    return oss.str();
}

const OptionParser::OptionDef* OptionParser::find_option(const std::string& name) const {
    for (const auto& opt : options_) {
        for (const auto& n : opt.names) {
            if (n == name) {
                return &opt;
            }
        }
    }
    return nullptr;
}

const OptionParser::OptionDef* OptionParser::find_short_prefix(const std::string& arg) const {
    // For -jN style: find a short option (single dash, single char) that is a prefix of arg.
    // Only applies to int options.
    for (const auto& opt : options_) {
        if (opt.type != OptType::Int) continue;
        for (const auto& n : opt.names) {
            // Short option: starts with single dash, length 2 (e.g., "-j")
            if (n.size() == 2 && n[0] == '-' && n[1] != '-') {
                if (arg.size() > 2 && arg.rfind(n, 0) == 0) {
                    return &opt;
                }
            }
        }
    }
    return nullptr;
}

std::vector<std::string> OptionParser::parse(int argc, char** argv) {
    std::vector<std::string> remaining;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // Check for --option=value syntax
        auto eq_pos = arg.find('=');
        if (eq_pos != std::string::npos && arg.size() > 2 && arg[0] == '-' && arg[1] == '-') {
            std::string name = arg.substr(0, eq_pos);
            std::string value = arg.substr(eq_pos + 1);

            const auto* opt = find_option(name);
            if (opt) {
                switch (opt->type) {
                    case OptType::String:
                        *opt->string_target = value;
                        break;
                    case OptType::OptionalString:
                        *opt->opt_string_target = value;
                        break;
                    case OptType::Int:
                        *opt->int_target = std::stoi(value);
                        break;
                    case OptType::Flag:
                        // Flags don't take values via =, treat as unrecognized
                        remaining.push_back(arg);
                        break;
                }
                continue;
            }
        }

        // Check for exact match (flags and options with separate value)
        const auto* opt = find_option(arg);
        if (opt) {
            switch (opt->type) {
                case OptType::Flag:
                    *opt->flag_target = true;
                    break;
                case OptType::String:
                    if (i + 1 < argc) {
                        *opt->string_target = argv[++i];
                    } else {
                        throw std::runtime_error(arg + " requires an argument");
                    }
                    break;
                case OptType::OptionalString:
                    if (i + 1 < argc) {
                        *opt->opt_string_target = argv[++i];
                    } else {
                        throw std::runtime_error(arg + " requires an argument");
                    }
                    break;
                case OptType::Int:
                    if (i + 1 < argc) {
                        *opt->int_target = std::stoi(argv[++i]);
                    } else {
                        throw std::runtime_error(arg + " requires an argument");
                    }
                    break;
            }
            continue;
        }

        // Check for short option with concatenated value (-jN style)
        const auto* prefix_opt = find_short_prefix(arg);
        if (prefix_opt) {
            std::string value_str = arg.substr(2);
            *prefix_opt->int_target = std::stoi(value_str);
            continue;
        }

        // Unrecognized argument: collect as remaining
        remaining.push_back(arg);
    }

    return remaining;
}

std::string OptionParser::format_names(const OptionDef& opt) const {
    std::ostringstream oss;

    // Separate short and long names
    std::vector<std::string> short_names;
    std::vector<std::string> long_names;
    for (const auto& n : opt.names) {
        if (n.size() > 1 && n[0] == '-' && n[1] == '-') {
            long_names.push_back(n);
        } else {
            short_names.push_back(n);
        }
    }

    // Build names string: short first, then long
    bool first = true;
    for (const auto& n : short_names) {
        if (!first) oss << ", ";
        oss << n;
        first = false;
    }
    for (const auto& n : long_names) {
        if (!first) oss << ", ";
        oss << n;
        first = false;
    }

    // Append metavar for non-flag options
    if (opt.type != OptType::Flag && !opt.metavar.empty()) {
        oss << "=" << opt.metavar;
    }

    return oss.str();
}

std::string OptionParser::help(const std::string& program_description) const {
    std::ostringstream oss;

    oss << "Usage: " << program_name_ << " [OPTIONS] <compiler-command>\n";
    oss << "\n";

    if (!program_description.empty()) {
        oss << program_description << "\n";
        oss << "\n";
    }

    // Determine column width for alignment
    size_t max_name_width = 0;
    for (const auto& opt : options_) {
        size_t w = format_names(opt).size();
        if (w > max_name_width) max_name_width = w;
    }
    // Add padding: 2 for leading indent + 2 for gap
    size_t col_width = max_name_width + 4;
    if (col_width < 28) col_width = 28;

    // Group options
    std::string last_group;
    for (const auto& opt : options_) {
        if (opt.group_name != last_group) {
            if (!last_group.empty()) {
                oss << "\n";
            }
            oss << opt.group_name << ":\n";
            last_group = opt.group_name;
        }

        std::string names_str = format_names(opt);
        oss << "  " << names_str;

        // Pad to alignment column
        if (names_str.size() + 2 < col_width) {
            oss << std::string(col_width - names_str.size() - 2, ' ');
        } else {
            oss << "\n" << std::string(col_width, ' ');
        }

        oss << opt.description << "\n";
    }

    oss << "\n";
    oss << "Examples:\n";
    oss << "  " << program_name_ << " g++ -std=c++17 main.cpp\n";
    oss << "  " << program_name_ << " --config=.cclint.yaml g++ main.cpp\n";
    oss << "  " << program_name_ << " --format=json clang++ -std=c++20 src/*.cpp\n";
    oss << "\n";
    oss << "Configuration:\n";
    oss << "  Create a cclint.yaml file in your project root with:\n";
    oss << "  - lua_scripts: paths to Lua rule scripts\n";
    oss << "  - Standard scripts available in: ${CCLINT_HOME}/rules/\n";
    oss << "\n";
    oss << "For more information, visit:\n";
    oss << "  https://github.com/KikuchiTomo/cclint\n";

    return oss.str();
}

}  // namespace cli
