#pragma once

#include <functional>
#include <initializer_list>
#include <optional>
#include <string>
#include <vector>

namespace cli {

class OptionParser {
public:
    /// Start a new option group for help display.
    OptionParser& group(const std::string& name);

    /// Register a bool flag.
    OptionParser& flag(bool& target, std::initializer_list<std::string> names,
                       const std::string& description);

    /// Register a string option.
    OptionParser& option(std::string& target, std::initializer_list<std::string> names,
                         const std::string& description, const std::string& metavar = "VALUE");

    /// Register an optional string option.
    OptionParser& option(std::optional<std::string>& target,
                         std::initializer_list<std::string> names,
                         const std::string& description, const std::string& metavar = "VALUE");

    /// Register an int option.
    OptionParser& option(int& target, std::initializer_list<std::string> names,
                         const std::string& description, const std::string& metavar = "N");

    /// Parse args. Returns remaining (unrecognized) arguments.
    std::vector<std::string> parse(int argc, char** argv);

    /// Generate formatted help text.
    std::string help(const std::string& program_description = "") const;

    /// Set program name and version for help/version output.
    void set_program(const std::string& name, const std::string& version);

    /// Get formatted version string.
    std::string version_string() const;

private:
    enum class OptType { Flag, String, OptionalString, Int };

    struct OptionDef {
        std::vector<std::string> names;
        std::string description;
        std::string metavar;
        std::string group_name;
        OptType type;

        // Targets (only one is active based on type)
        bool* flag_target = nullptr;
        std::string* string_target = nullptr;
        std::optional<std::string>* opt_string_target = nullptr;
        int* int_target = nullptr;
    };

    std::string program_name_;
    std::string program_version_;
    std::string current_group_;
    std::vector<OptionDef> options_;

    /// Find an option definition matching the given name. Returns nullptr if not found.
    const OptionDef* find_option(const std::string& name) const;

    /// Check if a short option prefix matches (for -jN style).
    const OptionDef* find_short_prefix(const std::string& arg) const;

    /// Format the names column for help display.
    std::string format_names(const OptionDef& opt) const;
};

}  // namespace cli
