#include "rules/builtin/header_guard.hpp"

#include <regex>

#include "utils/string_utils.hpp"

namespace cclint {
namespace rules {
namespace builtin {

HeaderGuardRule::HeaderGuardRule() {}

void HeaderGuardRule::initialize(const RuleParameters& params) {
    if (params.count("allow_pragma_once")) {
        allow_pragma_once_ = params.at("allow_pragma_once") == "true";
    }
    if (params.count("require_pragma_once")) {
        require_pragma_once_ = params.at("require_pragma_once") == "true";
    }
}

void HeaderGuardRule::check_file(const std::string& file_path, const std::string& content,
                                 diagnostic::DiagnosticEngine& diag_engine) {
    // ヘッダーファイルのみチェック
    if (!is_header_file(file_path)) {
        return;
    }

    bool has_pragma = has_pragma_once(content);
    bool has_guard = has_header_guard(content);

    if (require_pragma_once_) {
        if (!has_pragma) {
            report_diagnostic(diag_engine, file_path, 1, 1, "Header file should use #pragma once");
        }
    } else {
        if (!has_pragma && !has_guard) {
            report_diagnostic(diag_engine, file_path, 1, 1,
                              "Header file missing header guard or #pragma once");
        }
    }
}

bool HeaderGuardRule::is_header_file(const std::string& file_path) const {
    // C++20の ends_with の代わりに、手動でサフィックスチェック
    auto has_suffix = [](const std::string& str, const std::string& suffix) {
        if (str.length() < suffix.length())
            return false;
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    };

    return has_suffix(file_path, ".h") || has_suffix(file_path, ".hpp") ||
           has_suffix(file_path, ".hh") || has_suffix(file_path, ".hxx");
}

bool HeaderGuardRule::has_pragma_once(const std::string& content) const {
    return content.find("#pragma once") != std::string::npos;
}

bool HeaderGuardRule::has_header_guard(const std::string& content) const {
    // #ifndef, #define, #endif のパターンを検出
    std::regex ifndef_pattern(R"(#\s*ifndef\s+[A-Za-z_][A-Za-z0-9_]*)");
    std::regex define_pattern(R"(#\s*define\s+[A-Za-z_][A-Za-z0-9_]*)");
    std::regex endif_pattern(R"(#\s*endif)");

    bool has_ifndef = std::regex_search(content, ifndef_pattern);
    bool has_define = std::regex_search(content, define_pattern);
    bool has_endif = std::regex_search(content, endif_pattern);

    return has_ifndef && has_define && has_endif;
}

}  // namespace builtin
}  // namespace rules
}  // namespace cclint
