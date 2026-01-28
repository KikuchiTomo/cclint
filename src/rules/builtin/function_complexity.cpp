#include "rules/builtin/function_complexity.hpp"

#include <regex>
#include <sstream>

namespace cclint {
namespace rules {
namespace builtin {

FunctionComplexityRule::FunctionComplexityRule() {
    set_enabled(true);
    set_severity(config::Severity::Warning);
}

void FunctionComplexityRule::initialize(const RuleParameters& params) {
    auto it = params.find("max_complexity");
    if (it != params.end()) {
        try {
            max_complexity_ = std::stoi(it->second);
        } catch (...) {
            // パース失敗時はデフォルト値を使用
        }
    }
}

void FunctionComplexityRule::check_file(const std::string& file_path, const std::string& content,
                                        diagnostic::DiagnosticEngine& diag_engine) {
    // 簡易的な関数検出（正規表現ベース）
    // 注: 完全な実装ではAST解析を使用するべき
    std::regex func_pattern(R"((\w+)\s+(\w+)\s*\([^)]*\)\s*\{([^{}]*(?:\{[^{}]*\}[^{}]*)*)\})");

    std::string::const_iterator search_start(content.cbegin());
    std::smatch match;

    // int line = 1;
    // size_t pos = 0;

    while (std::regex_search(search_start, content.cend(), match, func_pattern)) {
        std::string return_type = match[1].str();
        std::string func_name = match[2].str();
        std::string func_body = match[3].str();

        // 複雑度を計算
        int complexity = calculate_complexity(func_body);

        if (complexity > max_complexity_) {
            // マッチ位置の行番号を計算
            size_t match_pos = match.position() + (search_start - content.cbegin());
            int func_line = 1 + std::count(content.begin(), content.begin() + match_pos, '\n');

            std::ostringstream msg;
            msg << "Function '" << func_name << "' has cyclomatic complexity of " << complexity
                << " (max allowed is " << max_complexity_ << ")";

            report_diagnostic(diag_engine, file_path, func_line, 0, msg.str());
        }

        search_start = match.suffix().first;
    }
}

int FunctionComplexityRule::calculate_complexity(const std::string& function_body) const {
    // サイクロマティック複雑度 = 制御文の数 + 1
    int complexity = 1;

    complexity += count_control_statements(function_body);

    return complexity;
}

int FunctionComplexityRule::count_control_statements(const std::string& code) const {
    int count = 0;

    // if
    std::regex if_pattern(R"(\bif\s*\()");
    count += std::distance(std::sregex_iterator(code.begin(), code.end(), if_pattern),
                           std::sregex_iterator());

    // else if
    std::regex elif_pattern(R"(\belse\s+if\s*\()");
    count += std::distance(std::sregex_iterator(code.begin(), code.end(), elif_pattern),
                           std::sregex_iterator());

    // for
    std::regex for_pattern(R"(\bfor\s*\()");
    count += std::distance(std::sregex_iterator(code.begin(), code.end(), for_pattern),
                           std::sregex_iterator());

    // while
    std::regex while_pattern(R"(\bwhile\s*\()");
    count += std::distance(std::sregex_iterator(code.begin(), code.end(), while_pattern),
                           std::sregex_iterator());

    // do-while
    std::regex do_pattern(R"(\bdo\s*\{)");
    count += std::distance(std::sregex_iterator(code.begin(), code.end(), do_pattern),
                           std::sregex_iterator());

    // switch case
    std::regex case_pattern(R"(\bcase\s+)");
    count += std::distance(std::sregex_iterator(code.begin(), code.end(), case_pattern),
                           std::sregex_iterator());

    // catch
    std::regex catch_pattern(R"(\bcatch\s*\()");
    count += std::distance(std::sregex_iterator(code.begin(), code.end(), catch_pattern),
                           std::sregex_iterator());

    // ternary operator
    std::regex ternary_pattern(R"(\?[^:]+:)");
    count += std::distance(std::sregex_iterator(code.begin(), code.end(), ternary_pattern),
                           std::sregex_iterator());

    // logical && and ||
    std::regex and_pattern(R"(&&)");
    count += std::distance(std::sregex_iterator(code.begin(), code.end(), and_pattern),
                           std::sregex_iterator());

    std::regex or_pattern(R"(\|\|)");
    count += std::distance(std::sregex_iterator(code.begin(), code.end(), or_pattern),
                           std::sregex_iterator());

    return count;
}

}  // namespace builtin
}  // namespace rules
}  // namespace cclint
