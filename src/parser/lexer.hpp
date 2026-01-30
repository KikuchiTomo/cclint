#pragma once

#include <string>
#include <vector>

// Use the enhanced token types for complete C++ support
#include "parser/token_types_enhanced.hpp"

namespace cclint {
namespace parser {

// Note: Token and TokenType are now defined in token_types_enhanced.hpp
// This legacy lexer file is kept for backward compatibility but no longer
// defines its own Token types to avoid ODR (One Definition Rule) violations

/// 字句解析器
class Lexer {
public:
    explicit Lexer(const std::string& source);

    /// すべてのトークンを取得
    std::vector<Token> tokenize();

    /// 次のトークンを取得
    Token next_token();

    /// 現在位置を取得
    size_t get_position() const { return pos_; }

    /// 現在行を取得
    int get_line() const { return line_; }

    /// 現在列を取得
    int get_column() const { return column_; }

private:
    std::string source_;
    size_t pos_ = 0;
    int line_ = 1;
    int column_ = 1;

    char current_char() const;
    char peek_char(int offset = 1) const;
    void advance();
    void skip_whitespace();
    Token read_identifier();
    Token read_number();
    Token read_string(char quote);
    Token read_comment();
    Token read_preprocessor();
    TokenType get_keyword_type(const std::string& word) const;
    bool is_alpha(char c) const;
    bool is_digit(char c) const;
    bool is_alnum(char c) const;
};

}  // namespace parser
}  // namespace cclint
