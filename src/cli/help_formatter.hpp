#pragma once

#include <string>

namespace cli {

class HelpFormatter {
public:
    static std::string format_help();
    static std::string format_version();

private:
    static std::string format_usage();
    static std::string format_options();
    static std::string format_examples();
    static std::string format_configuration();
};

}  // namespace cli
