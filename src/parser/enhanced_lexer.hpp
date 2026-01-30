#pragma once

#include <memory>
#include <string>
#include <vector>

#include "parser/token_types_enhanced.hpp"

namespace cclint {
namespace parser {

/// Enhanced lexer for complete C++ tokenization
class EnhancedLexer {
public:
    explicit EnhancedLexer(const std::string& source, const std::string& filename = "");

    /// Tokenize the entire source
    std::vector<Token> tokenize();

    /// Get all errors encountered during lexing
    const std::vector<std::string>& errors() const { return errors_; }

    /// Check if there were any errors
    bool has_errors() const { return !errors_.empty(); }

private:
    // ========== Character access ==========
    char current() const;
    char peek(int offset = 1) const;
    void advance(int count = 1);
    bool is_eof() const;

    // ========== Position tracking ==========
    void mark_position();
    void reset_to_mark();
    int get_line() const { return line_; }
    int get_column() const { return column_; }

    // ========== Character classification ==========
    bool is_whitespace(char c) const;
    bool is_newline(char c) const;
    bool is_digit(char c) const;
    bool is_hex_digit(char c) const;
    bool is_octal_digit(char c) const;
    bool is_binary_digit(char c) const;
    bool is_identifier_start(char c) const;
    bool is_identifier_continue(char c) const;

    // ========== Token lexing ==========
    Token lex_token();

    // Identifiers and keywords
    Token lex_identifier();
    Token lex_keyword_or_identifier(const std::string& text);

    // Numeric literals
    Token lex_numeric();
    Token lex_integer();
    Token lex_floating();
    Token lex_binary();       // 0b...
    Token lex_octal();        // 0...
    Token lex_hexadecimal();  // 0x...
    std::string lex_integer_suffix();
    std::string lex_floating_suffix();

    // Character literals
    Token lex_character();
    Token lex_wide_character();   // L'...'
    Token lex_utf8_character();   // u8'...'
    Token lex_utf16_character();  // u'...'
    Token lex_utf32_character();  // U'...'
    char lex_escape_sequence();

    // String literals
    Token lex_string();
    Token lex_wide_string();   // L"..."
    Token lex_utf8_string();   // u8"..."
    Token lex_utf16_string();  // u"..."
    Token lex_utf32_string();  // U"..."
    Token lex_raw_string();    // R"(...)"
    std::string lex_raw_string_delimiter();

    // User-defined literals
    Token lex_user_defined_literal(Token base_literal);

    // Operators and punctuation
    Token lex_operator();
    Token lex_multi_char_operator();

    // Comments
    Token lex_line_comment();
    Token lex_block_comment();

    // Preprocessor
    Token lex_preprocessor_directive();

    // Attributes
    Token lex_attribute();

    // Whitespace
    void skip_whitespace();
    void skip_line();

    // ========== Utilities ==========
    void add_error(const std::string& message);
    Token make_token(TokenType type, const std::string& text);
    Token make_token(TokenType type, const std::string& text, const std::string& value);

    // ========== Member variables ==========
    std::string source_;
    std::string filename_;
    size_t pos_;
    int line_;
    int column_;

    // Position marking (for backtracking)
    size_t marked_pos_;
    int marked_line_;
    int marked_column_;

    // Flags
    bool at_start_of_line_;
    bool had_whitespace_before_;

    // Errors
    std::vector<std::string> errors_;
};

}  // namespace parser
}  // namespace cclint
