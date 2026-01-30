#include "parser/enhanced_lexer.hpp"

#include <cctype>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace cclint {
namespace parser {

EnhancedLexer::EnhancedLexer(const std::string& source, const std::string& filename)
    : source_(source),
      filename_(filename),
      pos_(0),
      line_(1),
      column_(1),
      marked_pos_(0),
      marked_line_(1),
      marked_column_(1),
      at_start_of_line_(true),
      had_whitespace_before_(false) {}

std::vector<Token> EnhancedLexer::tokenize() {
    std::cerr << "[DEBUG] EnhancedLexer::tokenize() start" << std::endl;
    std::vector<Token> tokens;

    int token_count = 0;
    while (!is_eof()) {
        std::cerr << "[DEBUG] Lexing token #" << token_count++ << std::endl;
        Token token = lex_token();
        std::cerr << "[DEBUG] Token type: " << static_cast<int>(token.type) << std::endl;
        if (token.type != TokenType::Whitespace && token.type != TokenType::Newline) {
            token.filename = filename_;
            tokens.push_back(token);
        }
    }

    std::cerr << "[DEBUG] Adding EOF token" << std::endl;
    // Add EOF token
    Token eof;
    eof.type = TokenType::Eof;
    eof.filename = filename_;
    eof.line = line_;
    eof.column = column_;
    tokens.push_back(eof);

    std::cerr << "[DEBUG] EnhancedLexer::tokenize() end, " << tokens.size() << " tokens" << std::endl;
    return tokens;
}

// ========== Character access ==========

char EnhancedLexer::current() const {
    if (pos_ < source_.size()) {
        return source_[pos_];
    }
    return '\0';
}

char EnhancedLexer::peek(int offset) const {
    size_t pos = pos_ + offset;
    if (pos < source_.size()) {
        return source_[pos];
    }
    return '\0';
}

void EnhancedLexer::advance(int count) {
    for (int i = 0; i < count && pos_ < source_.size(); ++i) {
        if (source_[pos_] == '\n') {
            line_++;
            column_ = 1;
            at_start_of_line_ = true;
        } else {
            column_++;
            if (!is_whitespace(source_[pos_])) {
                at_start_of_line_ = false;
            }
        }
        pos_++;
    }
}

bool EnhancedLexer::is_eof() const {
    return pos_ >= source_.size();
}

// ========== Position tracking ==========

void EnhancedLexer::mark_position() {
    marked_pos_ = pos_;
    marked_line_ = line_;
    marked_column_ = column_;
}

void EnhancedLexer::reset_to_mark() {
    pos_ = marked_pos_;
    line_ = marked_line_;
    column_ = marked_column_;
}

// ========== Character classification ==========

bool EnhancedLexer::is_whitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f';
}

bool EnhancedLexer::is_newline(char c) const {
    return c == '\n';
}

bool EnhancedLexer::is_digit(char c) const {
    return c >= '0' && c <= '9';
}

bool EnhancedLexer::is_hex_digit(char c) const {
    return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

bool EnhancedLexer::is_octal_digit(char c) const {
    return c >= '0' && c <= '7';
}

bool EnhancedLexer::is_binary_digit(char c) const {
    return c == '0' || c == '1';
}

bool EnhancedLexer::is_identifier_start(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool EnhancedLexer::is_identifier_continue(char c) const {
    return is_identifier_start(c) || is_digit(c);
}

// ========== Token lexing ==========

Token EnhancedLexer::lex_token() {
    std::cerr << "[DEBUG] lex_token: start, pos=" << pos_ << " char='" << current() << "' (" << static_cast<int>(current()) << ")" << std::endl;
    had_whitespace_before_ = false;

    // Skip whitespace
    while (is_whitespace(current())) {
        advance();
        had_whitespace_before_ = true;
    }

    std::cerr << "[DEBUG] lex_token: after whitespace skip, pos=" << pos_ << std::endl;

    if (is_newline(current())) {
        std::cerr << "[DEBUG] lex_token: found newline" << std::endl;
        char c = current();
        advance();
        return make_token(TokenType::Newline, std::string(1, c));
    }

    if (is_eof()) {
        std::cerr << "[DEBUG] lex_token: at EOF" << std::endl;
        return make_token(TokenType::Eof, "");
    }

    int start_line = line_;
    int start_column = column_;

    char c = current();
    std::cerr << "[DEBUG] lex_token: lexing char '" << c << "' at line=" << line_ << " col=" << column_ << std::endl;

    // Comments
    if (c == '/') {
        if (peek() == '/') {
            return lex_line_comment();
        } else if (peek() == '*') {
            return lex_block_comment();
        }
    }

    // Preprocessor directives (must be at start of line)
    if (c == '#' && at_start_of_line_) {
        return lex_preprocessor_directive();
    }

    // Check for L, u8, u, U, R prefixes BEFORE identifier check
    if (c == 'L') {
        if (peek() == '\'') {
            return lex_wide_character();
        } else if (peek() == '"') {
            return lex_wide_string();
        }
    } else if (c == 'u') {
        if (peek() == '8' && peek(2) == '\'') {
            return lex_utf8_character();
        } else if (peek() == '8' && peek(2) == '"') {
            return lex_utf8_string();
        } else if (peek() == '\'') {
            return lex_utf16_character();
        } else if (peek() == '"') {
            return lex_utf16_string();
        }
    } else if (c == 'U') {
        if (peek() == '\'') {
            return lex_utf32_character();
        } else if (peek() == '"') {
            return lex_utf32_string();
        }
    } else if (c == 'R') {
        if (peek() == '"') {
            return lex_raw_string();
        }
    }

    // Identifiers and keywords
    if (is_identifier_start(c)) {
        return lex_identifier();
    }

    // Numeric literals
    if (is_digit(c)) {
        return lex_numeric();
    }

    // Character literals
    if (c == '\'') {
        return lex_character();
    }

    // String literals
    if (c == '"') {
        return lex_string();
    }

    // Attributes [[
    if (c == '[' && peek() == '[') {
        return lex_attribute();
    }

    // Operators and punctuation
    return lex_operator();
}

// ========== Identifiers and keywords ==========

Token EnhancedLexer::lex_identifier() {
    int start_line = line_;
    int start_column = column_;
    std::string text;

    while (is_identifier_continue(current())) {
        text += current();
        advance();
    }

    return lex_keyword_or_identifier(text);
}

Token EnhancedLexer::lex_keyword_or_identifier(const std::string& text) {
    TokenType type = keyword_from_string(text);
    return make_token(type, text);
}

// ========== Numeric literals ==========

Token EnhancedLexer::lex_numeric() {
    // Check for 0x, 0b, 0 prefix
    if (current() == '0') {
        char next = peek();
        if (next == 'x' || next == 'X') {
            return lex_hexadecimal();
        } else if (next == 'b' || next == 'B') {
            return lex_binary();
        } else if (is_octal_digit(next)) {
            return lex_octal();
        } else if (next == '.') {
            return lex_floating();
        }
    }

    // Check if it's floating point
    mark_position();
    bool has_dot = false;
    bool has_exp = false;

    while (is_digit(current()) || current() == '.' || current() == 'e' || current() == 'E' ||
           current() == '+' || current() == '-' || current() == '\'') {
        if (current() == '.') {
            if (has_dot)
                break;
            has_dot = true;
        } else if (current() == 'e' || current() == 'E') {
            if (has_exp)
                break;
            has_exp = true;
            advance();
            if (current() == '+' || current() == '-') {
                advance();
            }
            continue;
        } else if (current() == '\'') {
            // Digit separator (C++14)
            advance();
            continue;
        }
        advance();
    }

    reset_to_mark();

    if (has_dot || has_exp) {
        return lex_floating();
    } else {
        return lex_integer();
    }
}

Token EnhancedLexer::lex_integer() {
    std::string text;

    while (is_digit(current()) || current() == '\'') {
        if (current() != '\'') {  // Skip digit separators
            text += current();
        }
        advance();
    }

    // Integer suffix (u, l, ll, ul, ull, etc.)
    std::string suffix = lex_integer_suffix();
    text += suffix;

    Token token = make_token(TokenType::IntegerLiteral, text);

    // Check for user-defined literal
    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

Token EnhancedLexer::lex_floating() {
    std::string text;

    // Integer part
    while (is_digit(current()) || current() == '\'') {
        if (current() != '\'') {
            text += current();
        }
        advance();
    }

    // Decimal point
    if (current() == '.') {
        text += current();
        advance();

        // Fractional part
        while (is_digit(current()) || current() == '\'') {
            if (current() != '\'') {
                text += current();
            }
            advance();
        }
    }

    // Exponent
    if (current() == 'e' || current() == 'E') {
        text += current();
        advance();

        if (current() == '+' || current() == '-') {
            text += current();
            advance();
        }

        while (is_digit(current()) || current() == '\'') {
            if (current() != '\'') {
                text += current();
            }
            advance();
        }
    }

    // Floating suffix (f, l, F, L)
    std::string suffix = lex_floating_suffix();
    text += suffix;

    Token token = make_token(TokenType::FloatingLiteral, text);

    // Check for user-defined literal
    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

Token EnhancedLexer::lex_binary() {
    std::string text = "0";
    advance();  // skip '0'
    text += current();
    advance();  // skip 'b' or 'B'

    while (is_binary_digit(current()) || current() == '\'') {
        if (current() != '\'') {
            text += current();
        }
        advance();
    }

    std::string suffix = lex_integer_suffix();
    text += suffix;

    Token token = make_token(TokenType::IntegerLiteral, text);

    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

Token EnhancedLexer::lex_octal() {
    std::string text = "0";
    advance();  // skip '0'

    while (is_octal_digit(current()) || current() == '\'') {
        if (current() != '\'') {
            text += current();
        }
        advance();
    }

    std::string suffix = lex_integer_suffix();
    text += suffix;

    Token token = make_token(TokenType::IntegerLiteral, text);

    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

Token EnhancedLexer::lex_hexadecimal() {
    std::string text = "0";
    advance();  // skip '0'
    text += current();
    advance();  // skip 'x' or 'X'

    while (is_hex_digit(current()) || current() == '\'') {
        if (current() != '\'') {
            text += current();
        }
        advance();
    }

    // Hexadecimal floating point (C++17)
    if (current() == '.') {
        text += current();
        advance();

        while (is_hex_digit(current()) || current() == '\'') {
            if (current() != '\'') {
                text += current();
            }
            advance();
        }

        // Binary exponent (p or P)
        if (current() == 'p' || current() == 'P') {
            text += current();
            advance();

            if (current() == '+' || current() == '-') {
                text += current();
                advance();
            }

            while (is_digit(current()) || current() == '\'') {
                if (current() != '\'') {
                    text += current();
                }
                advance();
            }

            std::string suffix = lex_floating_suffix();
            text += suffix;

            Token token = make_token(TokenType::FloatingLiteral, text);

            if (is_identifier_start(current())) {
                return lex_user_defined_literal(token);
            }

            return token;
        }
    }

    std::string suffix = lex_integer_suffix();
    text += suffix;

    Token token = make_token(TokenType::IntegerLiteral, text);

    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

std::string EnhancedLexer::lex_integer_suffix() {
    std::string suffix;

    // u or U
    if (current() == 'u' || current() == 'U') {
        suffix += current();
        advance();
    }

    // l, ll, L, LL
    if (current() == 'l' || current() == 'L') {
        suffix += current();
        advance();
        if (current() == 'l' || current() == 'L') {
            suffix += current();
            advance();
        }
    }

    // u or U can also come after l/ll
    if (suffix.empty() || suffix.find('u') == std::string::npos) {
        if (current() == 'u' || current() == 'U') {
            suffix += current();
            advance();
        }
    }

    return suffix;
}

std::string EnhancedLexer::lex_floating_suffix() {
    std::string suffix;

    if (current() == 'f' || current() == 'F' || current() == 'l' || current() == 'L') {
        suffix += current();
        advance();
    }

    return suffix;
}

// ========== Character literals ==========

Token EnhancedLexer::lex_character() {
    std::string text;
    text += current();  // '
    advance();

    if (current() == '\\') {
        advance();
        char escape = lex_escape_sequence();
        text += '\\';
        text += escape;
    } else {
        text += current();
        advance();
    }

    if (current() == '\'') {
        text += current();
        advance();
    } else {
        add_error("Unterminated character literal");
    }

    Token token = make_token(TokenType::CharLiteral, text);

    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

Token EnhancedLexer::lex_wide_character() {
    std::string text = "L";
    advance();          // skip 'L'
    text += current();  // '
    advance();

    if (current() == '\\') {
        advance();
        char escape = lex_escape_sequence();
        text += '\\';
        text += escape;
    } else {
        text += current();
        advance();
    }

    if (current() == '\'') {
        text += current();
        advance();
    } else {
        add_error("Unterminated wide character literal");
    }

    Token token = make_token(TokenType::WideCharLiteral, text);

    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

Token EnhancedLexer::lex_utf8_character() {
    std::string text = "u8";
    advance();          // skip 'u'
    advance();          // skip '8'
    text += current();  // '
    advance();

    if (current() == '\\') {
        advance();
        char escape = lex_escape_sequence();
        text += '\\';
        text += escape;
    } else {
        text += current();
        advance();
    }

    if (current() == '\'') {
        text += current();
        advance();
    } else {
        add_error("Unterminated UTF-8 character literal");
    }

    Token token = make_token(TokenType::Utf8CharLiteral, text);

    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

Token EnhancedLexer::lex_utf16_character() {
    std::string text = "u";
    advance();          // skip 'u'
    text += current();  // '
    advance();

    if (current() == '\\') {
        advance();
        char escape = lex_escape_sequence();
        text += '\\';
        text += escape;
    } else {
        text += current();
        advance();
    }

    if (current() == '\'') {
        text += current();
        advance();
    } else {
        add_error("Unterminated UTF-16 character literal");
    }

    Token token = make_token(TokenType::Utf16CharLiteral, text);

    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

Token EnhancedLexer::lex_utf32_character() {
    std::string text = "U";
    advance();          // skip 'U'
    text += current();  // '
    advance();

    if (current() == '\\') {
        advance();
        char escape = lex_escape_sequence();
        text += '\\';
        text += escape;
    } else {
        text += current();
        advance();
    }

    if (current() == '\'') {
        text += current();
        advance();
    } else {
        add_error("Unterminated UTF-32 character literal");
    }

    Token token = make_token(TokenType::Utf32CharLiteral, text);

    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

char EnhancedLexer::lex_escape_sequence() {
    char c = current();

    switch (c) {
        case 'n':
            advance();
            return 'n';
        case 't':
            advance();
            return 't';
        case 'r':
            advance();
            return 'r';
        case 'b':
            advance();
            return 'b';
        case 'f':
            advance();
            return 'f';
        case 'v':
            advance();
            return 'v';
        case 'a':
            advance();
            return 'a';
        case '\\':
            advance();
            return '\\';
        case '\'':
            advance();
            return '\'';
        case '"':
            advance();
            return '"';
        case '?':
            advance();
            return '?';
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': {
            // Octal escape sequence
            advance();
            return c;
        }
        case 'x': {
            // Hexadecimal escape sequence
            advance();
            return 'x';
        }
        case 'u':
        case 'U': {
            // Unicode escape sequence
            advance();
            return c;
        }
        default:
            advance();
            return c;
    }
}

// ========== String literals ==========

Token EnhancedLexer::lex_string() {
    std::string text;
    text += current();  // "
    advance();

    while (current() != '"' && !is_eof()) {
        if (current() == '\\') {
            text += current();
            advance();
            if (!is_eof()) {
                char escape = lex_escape_sequence();
                text += escape;
            }
        } else {
            text += current();
            advance();
        }
    }

    if (current() == '"') {
        text += current();
        advance();
    } else {
        add_error("Unterminated string literal");
    }

    Token token = make_token(TokenType::StringLiteral, text);

    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

Token EnhancedLexer::lex_wide_string() {
    std::string text = "L";
    advance();          // skip 'L'
    text += current();  // "
    advance();

    while (current() != '"' && !is_eof()) {
        if (current() == '\\') {
            text += current();
            advance();
            if (!is_eof()) {
                char escape = lex_escape_sequence();
                text += escape;
            }
        } else {
            text += current();
            advance();
        }
    }

    if (current() == '"') {
        text += current();
        advance();
    } else {
        add_error("Unterminated wide string literal");
    }

    Token token = make_token(TokenType::WideStringLiteral, text);

    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

Token EnhancedLexer::lex_utf8_string() {
    std::string text = "u8";
    advance();          // skip 'u'
    advance();          // skip '8'
    text += current();  // "
    advance();

    while (current() != '"' && !is_eof()) {
        if (current() == '\\') {
            text += current();
            advance();
            if (!is_eof()) {
                char escape = lex_escape_sequence();
                text += escape;
            }
        } else {
            text += current();
            advance();
        }
    }

    if (current() == '"') {
        text += current();
        advance();
    } else {
        add_error("Unterminated UTF-8 string literal");
    }

    Token token = make_token(TokenType::Utf8StringLiteral, text);

    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

Token EnhancedLexer::lex_utf16_string() {
    std::string text = "u";
    advance();          // skip 'u'
    text += current();  // "
    advance();

    while (current() != '"' && !is_eof()) {
        if (current() == '\\') {
            text += current();
            advance();
            if (!is_eof()) {
                char escape = lex_escape_sequence();
                text += escape;
            }
        } else {
            text += current();
            advance();
        }
    }

    if (current() == '"') {
        text += current();
        advance();
    } else {
        add_error("Unterminated UTF-16 string literal");
    }

    Token token = make_token(TokenType::Utf16StringLiteral, text);

    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

Token EnhancedLexer::lex_utf32_string() {
    std::string text = "U";
    advance();          // skip 'U'
    text += current();  // "
    advance();

    while (current() != '"' && !is_eof()) {
        if (current() == '\\') {
            text += current();
            advance();
            if (!is_eof()) {
                char escape = lex_escape_sequence();
                text += escape;
            }
        } else {
            text += current();
            advance();
        }
    }

    if (current() == '"') {
        text += current();
        advance();
    } else {
        add_error("Unterminated UTF-32 string literal");
    }

    Token token = make_token(TokenType::Utf32StringLiteral, text);

    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

Token EnhancedLexer::lex_raw_string() {
    std::string text = "R";
    advance();          // skip 'R'
    text += current();  // "
    advance();

    // Raw string: R"delimiter(content)delimiter"
    std::string delimiter = lex_raw_string_delimiter();
    text += "(";

    // Content
    std::string end_marker = ")" + delimiter + "\"";
    while (!is_eof()) {
        if (current() == ')') {
            // Check if we're at the end
            bool is_end = true;
            for (size_t i = 0; i < end_marker.size(); ++i) {
                if (peek(i) != end_marker[i]) {
                    is_end = false;
                    break;
                }
            }

            if (is_end) {
                text += end_marker;
                advance(end_marker.size());
                break;
            }
        }

        text += current();
        advance();
    }

    Token token = make_token(TokenType::RawStringLiteral, text);

    if (is_identifier_start(current())) {
        return lex_user_defined_literal(token);
    }

    return token;
}

std::string EnhancedLexer::lex_raw_string_delimiter() {
    std::string delimiter;

    // Read delimiter (up to 16 characters, can't contain parentheses, backslash, or whitespace)
    while (current() != '(' && !is_eof() && delimiter.size() < 16) {
        if (current() == ')' || current() == '\\' || is_whitespace(current())) {
            add_error("Invalid raw string delimiter");
            break;
        }
        delimiter += current();
        advance();
    }

    if (current() == '(') {
        // Don't consume the opening paren, it's part of the syntax
    }

    return delimiter;
}

// ========== User-defined literals ==========

Token EnhancedLexer::lex_user_defined_literal(Token base_literal) {
    std::string suffix;

    while (is_identifier_continue(current())) {
        suffix += current();
        advance();
    }

    Token token = base_literal;
    token.type = TokenType::UserDefinedLiteral;
    token.text += suffix;

    return token;
}

// ========== Operators and punctuation ==========

Token EnhancedLexer::lex_operator() {
    std::cerr << "[DEBUG] lex_operator: start" << std::endl;
    char c = current();
    std::cerr << "[DEBUG] lex_operator: current()='" << c << "'" << std::endl;
    char next = peek();
    std::cerr << "[DEBUG] lex_operator: peek()='" << next << "'" << std::endl;
    char next2 = peek(2);
    std::cerr << "[DEBUG] lex_operator: peek(2)='" << next2 << "'" << std::endl;

    // Three-character operators
    if (c == '<' && next == '<' && next2 == '=') {
        std::string text = "<<=";
        advance(3);
        return make_token(TokenType::LeftShiftAssign, text);
    } else if (c == '>' && next == '>' && next2 == '=') {
        std::string text = ">>=";
        advance(3);
        return make_token(TokenType::RightShiftAssign, text);
    } else if (c == '<' && next == '=' && next2 == '>') {
        std::string text = "<=>";
        advance(3);
        return make_token(TokenType::Spaceship, text);
    } else if (c == '.' && next == '.' && next2 == '.') {
        std::string text = "...";
        advance(3);
        return make_token(TokenType::Ellipsis, text);
    }

    // Two-character operators
    if (c == '+' && next == '+') {
        advance(2);
        return make_token(TokenType::PlusPlus, "++");
    } else if (c == '-' && next == '-') {
        advance(2);
        return make_token(TokenType::MinusMinus, "--");
    } else if (c == '=' && next == '=') {
        advance(2);
        return make_token(TokenType::Equal, "==");
    } else if (c == '!' && next == '=') {
        advance(2);
        return make_token(TokenType::NotEqual, "!=");
    } else if (c == '<' && next == '=') {
        advance(2);
        return make_token(TokenType::LessEqual, "<=");
    } else if (c == '>' && next == '=') {
        advance(2);
        return make_token(TokenType::GreaterEqual, ">=");
    } else if (c == '&' && next == '&') {
        advance(2);
        return make_token(TokenType::LogicalAnd, "&&");
    } else if (c == '|' && next == '|') {
        advance(2);
        return make_token(TokenType::LogicalOr, "||");
    } else if (c == '<' && next == '<') {
        advance(2);
        return make_token(TokenType::LeftShift, "<<");
    } else if (c == '>' && next == '>') {
        advance(2);
        return make_token(TokenType::RightShift, ">>");
    } else if (c == '+' && next == '=') {
        advance(2);
        return make_token(TokenType::PlusAssign, "+=");
    } else if (c == '-' && next == '=') {
        advance(2);
        return make_token(TokenType::MinusAssign, "-=");
    } else if (c == '*' && next == '=') {
        advance(2);
        return make_token(TokenType::StarAssign, "*=");
    } else if (c == '/' && next == '=') {
        advance(2);
        return make_token(TokenType::SlashAssign, "/=");
    } else if (c == '%' && next == '=') {
        advance(2);
        return make_token(TokenType::PercentAssign, "%=");
    } else if (c == '&' && next == '=') {
        advance(2);
        return make_token(TokenType::AmpersandAssign, "&=");
    } else if (c == '|' && next == '=') {
        advance(2);
        return make_token(TokenType::PipeAssign, "|=");
    } else if (c == '^' && next == '=') {
        advance(2);
        return make_token(TokenType::CaretAssign, "^=");
    } else if (c == '-' && next == '>') {
        if (peek(2) == '*') {
            advance(3);
            return make_token(TokenType::ArrowStar, "->*");
        }
        advance(2);
        return make_token(TokenType::Arrow, "->");
    } else if (c == '.' && next == '*') {
        advance(2);
        return make_token(TokenType::DotStar, ".*");
    } else if (c == ':' && next == ':') {
        advance(2);
        return make_token(TokenType::DoubleColon, "::");
    } else if (c == '#' && next == '#') {
        advance(2);
        return make_token(TokenType::MacroConcat, "##");
    }

    // Single-character operators
    std::string text(1, c);
    advance();

    switch (c) {
        case '+':
            return make_token(TokenType::Plus, text);
        case '-':
            return make_token(TokenType::Minus, text);
        case '*':
            return make_token(TokenType::Star, text);
        case '/':
            return make_token(TokenType::Slash, text);
        case '%':
            return make_token(TokenType::Percent, text);
        case '<':
            return make_token(TokenType::Less, text);
        case '>':
            return make_token(TokenType::Greater, text);
        case '!':
            return make_token(TokenType::LogicalNot, text);
        case '&':
            return make_token(TokenType::Ampersand, text);
        case '|':
            return make_token(TokenType::Pipe, text);
        case '^':
            return make_token(TokenType::Caret, text);
        case '~':
            return make_token(TokenType::Tilde, text);
        case '=':
            return make_token(TokenType::Assign, text);
        case '.':
            return make_token(TokenType::Dot, text);
        case '?':
            return make_token(TokenType::Question, text);
        case ':':
            return make_token(TokenType::Colon, text);
        case ';':
            return make_token(TokenType::Semicolon, text);
        case ',':
            return make_token(TokenType::Comma, text);
        case '(':
            return make_token(TokenType::LeftParen, text);
        case ')':
            return make_token(TokenType::RightParen, text);
        case '{':
            return make_token(TokenType::LeftBrace, text);
        case '}':
            return make_token(TokenType::RightBrace, text);
        case '[':
            return make_token(TokenType::LeftBracket, text);
        case ']':
            return make_token(TokenType::RightBracket, text);
        case '#':
            return make_token(TokenType::MacroStringify, text);
        default:
            add_error("Unknown character: " + text);
            return make_token(TokenType::Unknown, text);
    }
}

// ========== Comments ==========

Token EnhancedLexer::lex_line_comment() {
    std::string text;
    text += current();  // /
    advance();
    text += current();  // /
    advance();

    while (!is_newline(current()) && !is_eof()) {
        text += current();
        advance();
    }

    return make_token(TokenType::LineComment, text);
}

Token EnhancedLexer::lex_block_comment() {
    std::string text;
    text += current();  // /
    advance();
    text += current();  // *
    advance();

    int nesting_level = 1;

    while (!is_eof() && nesting_level > 0) {
        if (current() == '/' && peek() == '*') {
            // Nested comment (C++23 feature, but we support it for robustness)
            text += current();
            advance();
            text += current();
            advance();
            nesting_level++;
        } else if (current() == '*' && peek() == '/') {
            text += current();
            advance();
            text += current();
            advance();
            nesting_level--;
        } else {
            text += current();
            advance();
        }
    }

    if (nesting_level > 0) {
        add_error("Unterminated block comment");
    }

    return make_token(TokenType::BlockComment, text);
}

// ========== Preprocessor ==========

Token EnhancedLexer::lex_preprocessor_directive() {
    std::string text;
    text += current();  // #
    advance();

    // Skip whitespace after #
    while (is_whitespace(current())) {
        text += current();
        advance();
    }

    // Read directive name
    std::string directive;
    while (is_identifier_continue(current())) {
        directive += current();
        text += current();
        advance();
    }

    // Read rest of line
    while (!is_newline(current()) && !is_eof()) {
        text += current();
        advance();
    }

    // Determine directive type
    TokenType type = TokenType::PPDirective;

    if (directive == "include") {
        type = TokenType::PPInclude;
    } else if (directive == "define") {
        type = TokenType::PPDefine;
    } else if (directive == "undef") {
        type = TokenType::PPUndef;
    } else if (directive == "if") {
        type = TokenType::PPIf;
    } else if (directive == "ifdef") {
        type = TokenType::PPIfdef;
    } else if (directive == "ifndef") {
        type = TokenType::PPIfndef;
    } else if (directive == "elif") {
        type = TokenType::PPElif;
    } else if (directive == "else") {
        type = TokenType::PPElse;
    } else if (directive == "endif") {
        type = TokenType::PPEndif;
    } else if (directive == "pragma") {
        type = TokenType::PPPragma;
    } else if (directive == "error") {
        type = TokenType::PPError;
    } else if (directive == "warning") {
        type = TokenType::PPWarning;
    } else if (directive == "line") {
        type = TokenType::PPLine;
    }

    return make_token(type, text);
}

// ========== Attributes ==========

Token EnhancedLexer::lex_attribute() {
    std::string text = "[[";
    advance();  // skip '['
    advance();  // skip '['

    int depth = 1;

    while (!is_eof() && depth > 0) {
        if (current() == '[' && peek() == '[') {
            text += "[[";
            advance(2);
            depth++;
        } else if (current() == ']' && peek() == ']') {
            text += "]]";
            advance(2);
            depth--;
        } else {
            text += current();
            advance();
        }
    }

    if (depth > 0) {
        add_error("Unterminated attribute");
    }

    return make_token(TokenType::AttributeStart, text);
}

// ========== Whitespace ==========

void EnhancedLexer::skip_whitespace() {
    while (is_whitespace(current())) {
        advance();
    }
}

void EnhancedLexer::skip_line() {
    while (!is_newline(current()) && !is_eof()) {
        advance();
    }
    if (is_newline(current())) {
        advance();
    }
}

// ========== Utilities ==========

void EnhancedLexer::add_error(const std::string& message) {
    std::ostringstream oss;
    oss << filename_ << ":" << line_ << ":" << column_ << ": " << message;
    errors_.push_back(oss.str());
}

Token EnhancedLexer::make_token(TokenType type, const std::string& text) {
    Token token;
    token.type = type;
    token.text = text;
    token.value = text;
    token.line = line_;
    token.column = column_ - text.size();
    token.has_whitespace_before = had_whitespace_before_;
    token.is_at_start_of_line = at_start_of_line_;
    return token;
}

Token EnhancedLexer::make_token(TokenType type, const std::string& text, const std::string& value) {
    Token token;
    token.type = type;
    token.text = text;
    token.value = value;
    token.line = line_;
    token.column = column_ - text.size();
    token.has_whitespace_before = had_whitespace_before_;
    token.is_at_start_of_line = at_start_of_line_;
    return token;
}

}  // namespace parser
}  // namespace cclint
