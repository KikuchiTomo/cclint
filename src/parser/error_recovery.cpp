#include "parser/error_recovery.hpp"

#include <sstream>

namespace cclint {
namespace parser {

std::string ParseError::format() const {
    std::ostringstream oss;

    // ファイル名と位置
    if (!filename.empty()) {
        oss << filename << ":";
    }
    oss << line << ":" << column << ": ";

    // 重要度
    switch (severity) {
        case Severity::Fatal:
            oss << "fatal error: ";
            break;
        case Severity::Error:
            oss << "error: ";
            break;
        case Severity::Warning:
            oss << "warning: ";
            break;
        case Severity::Note:
            oss << "note: ";
            break;
    }

    // メッセージ
    oss << message;

    // 期待されたトークン
    if (!expected_tokens.empty()) {
        oss << " (expected ";
        for (size_t i = 0; i < expected_tokens.size(); ++i) {
            if (i > 0)
                oss << " or ";
            // TokenTypeを文字列に変換（簡易版）
            oss << "token";
        }
        oss << ")";
    }

    // 修正提案
    if (!fix_suggestion.empty()) {
        oss << "\n  suggestion: " << fix_suggestion;
    }

    return oss.str();
}

void ErrorRecovery::add_error(const ParseError& error) {
    errors_.push_back(error);
}

bool ErrorRecovery::has_fatal_errors() const {
    for (const auto& error : errors_) {
        if (error.severity == ParseError::Severity::Fatal) {
            return true;
        }
    }
    return false;
}

size_t ErrorRecovery::warning_count() const {
    size_t count = 0;
    for (const auto& error : errors_) {
        if (error.severity == ParseError::Severity::Warning) {
            ++count;
        }
    }
    return count;
}

RecoveryStrategy ErrorRecovery::suggest_recovery(const std::vector<TokenType>& expected,
                                                 TokenType /* found */) {
    // セミコロンが期待されている場合
    for (auto exp : expected) {
        if (exp == TokenType::Semicolon) {
            return RecoveryStrategy::SkipToSemicolon;
        }
    }

    // 閉じ括弧が期待されている場合
    for (auto exp : expected) {
        if (exp == TokenType::RightBrace || exp == TokenType::RightParen ||
            exp == TokenType::RightBracket) {
            return RecoveryStrategy::SkipToCloseBrace;
        }
    }

    // デフォルト: 次の文までスキップ
    return RecoveryStrategy::SkipToNextStatement;
}

std::string ErrorRecovery::format_errors(const std::vector<ParseError>& errors) {
    std::ostringstream oss;

    for (size_t i = 0; i < errors.size(); ++i) {
        if (i > 0)
            oss << "\n";
        oss << errors[i].format();
    }

    // サマリー
    if (!errors.empty()) {
        size_t error_count = 0;
        size_t warning_count = 0;

        for (const auto& error : errors) {
            if (error.severity == ParseError::Severity::Error ||
                error.severity == ParseError::Severity::Fatal) {
                ++error_count;
            } else if (error.severity == ParseError::Severity::Warning) {
                ++warning_count;
            }
        }

        oss << "\n\n";
        if (error_count > 0) {
            oss << error_count << " error(s) ";
        }
        if (warning_count > 0) {
            if (error_count > 0)
                oss << "and ";
            oss << warning_count << " warning(s) ";
        }
        oss << "generated.";
    }

    return oss.str();
}

}  // namespace parser
}  // namespace cclint
