#include "rules/builtin/max_line_length.hpp"

#include <regex>

#include "utils/string_utils.hpp"

namespace cclint {
namespace rules {
namespace builtin {

MaxLineLengthRule::MaxLineLengthRule() {}

void MaxLineLengthRule::initialize(const RuleParameters& params) {
    if (params.count("max_length")) {
        max_length_ = std::stoul(params.at("max_length"));
    }
    if (params.count("ignore_comments")) {
        ignore_comments_ = params.at("ignore_comments") == "true";
    }
    if (params.count("ignore_urls")) {
        ignore_urls_ = params.at("ignore_urls") == "true";
    }
}

void MaxLineLengthRule::check_file(const std::string& file_path, const std::string& content,
                                   diagnostic::DiagnosticEngine& diag_engine) {
    auto lines = utils::StringUtils::split(content, '\n');
    int line_num = 0;

    std::regex url_pattern(R"(https?://[^\s]+)");

    for (const auto& line : lines) {
        line_num++;

        // URLを含む行は除外（オプション）
        if (ignore_urls_ && std::regex_search(std::string(line), url_pattern)) {
            continue;
        }

        // コメント行を除外（オプション）
        if (ignore_comments_) {
            auto trimmed = utils::StringUtils::trim(line);
            if (trimmed.rfind("//", 0) == 0 || trimmed.rfind("/*", 0) == 0 ||
                trimmed.rfind("*", 0) == 0) {
                continue;
            }
        }

        if (line.length() > max_length_) {
            std::string message = "Line exceeds maximum length of " + std::to_string(max_length_) +
                                  " characters (current: " + std::to_string(line.length()) + ")";
            report_diagnostic(diag_engine, file_path, line_num, 1, message);
        }
    }
}

}  // namespace builtin
}  // namespace rules
}  // namespace cclint
