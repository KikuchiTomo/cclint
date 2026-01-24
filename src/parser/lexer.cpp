#include "parser/lexer.hpp"

#include <cctype>
#include <unordered_map>

namespace cclint {
namespace parser {

bool Token::is_keyword() const {
    return type >= TokenType::Class && type <= TokenType::Typename;
}

std::string Token::get_type_name() const {
    static const char* names[] = {"Identifier",  "IntLiteral", "StringLiteral",
                                  "CharLiteral", "class",      "struct",
                                  "enum",        "namespace",  "using",
                                  "typedef",     "public",     "protected",
                                  "private",     "virtual",    "override",
                                  "final",       "static",     "const",
                                  "constexpr",   "mutable",    "void",
                                  "int",         "bool",       "char",
                                  "float",       "double",     "auto",
                                  "if",          "else",       "for",
                                  "while",       "do",         "switch",
                                  "case",        "default",    "return",
                                  "break",       "continue",   "template",
                                  "typename",    "(",          ")",
                                  "{",           "}",          "[",
                                  "]",           ";",          ",",
                                  ":",           "::",         "->",
                                  ".",           "*",          "&",
                                  "<",           ">",          "=",
                                  "+",           "-",          "/",
                                  "%",           "Comment",    "Preprocessor",
                                  "EOF",         "Unknown"};
    int index = static_cast<int>(type);
    if (index >= 0 && index < static_cast<int>(sizeof(names) / sizeof(names[0]))) {
        return names[index];
    }
    return "Unknown";
}

Lexer::Lexer(const std::string& source) : source_(source) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token token;

    do {
        token = next_token();
        tokens.push_back(token);
    } while (token.type != TokenType::Eof);

    return tokens;
}

Token Lexer::next_token() {
    skip_whitespace();

    if (pos_ >= source_.length()) {
        return Token(TokenType::Eof, "", line_, column_);
    }

    int start_line = line_;
    int start_column = column_;

    char c = current_char();

    // コメント
    if (c == '/' && peek_char() == '/') {
        return read_comment();
    }
    if (c == '/' && peek_char() == '*') {
        return read_comment();
    }

    // プリプロセッサ
    if (c == '#') {
        return read_preprocessor();
    }

    // 文字列リテラル
    if (c == '"' || c == '\'') {
        return read_string(c);
    }

    // 数値リテラル
    if (is_digit(c)) {
        return read_number();
    }

    // 識別子またはキーワード
    if (is_alpha(c) || c == '_') {
        return read_identifier();
    }

    // 2文字演算子
    if (c == ':' && peek_char() == ':') {
        advance();
        advance();
        return Token(TokenType::Scope, "::", start_line, start_column);
    }
    if (c == '-' && peek_char() == '>') {
        advance();
        advance();
        return Token(TokenType::Arrow, "->", start_line, start_column);
    }

    // 1文字トークン
    TokenType type = TokenType::Unknown;
    switch (c) {
        case '(':
            type = TokenType::LeftParen;
            break;
        case ')':
            type = TokenType::RightParen;
            break;
        case '{':
            type = TokenType::LeftBrace;
            break;
        case '}':
            type = TokenType::RightBrace;
            break;
        case '[':
            type = TokenType::LeftBracket;
            break;
        case ']':
            type = TokenType::RightBracket;
            break;
        case ';':
            type = TokenType::Semicolon;
            break;
        case ',':
            type = TokenType::Comma;
            break;
        case ':':
            type = TokenType::Colon;
            break;
        case '.':
            type = TokenType::Dot;
            break;
        case '*':
            type = TokenType::Asterisk;
            break;
        case '&':
            type = TokenType::Ampersand;
            break;
        case '<':
            type = TokenType::Less;
            break;
        case '>':
            type = TokenType::Greater;
            break;
        case '=':
            type = TokenType::Equal;
            break;
        case '+':
            type = TokenType::Plus;
            break;
        case '-':
            type = TokenType::Minus;
            break;
        case '/':
            type = TokenType::Slash;
            break;
        case '%':
            type = TokenType::Percent;
            break;
        default:
            type = TokenType::Unknown;
            break;
    }

    std::string text(1, c);
    advance();
    return Token(type, text, start_line, start_column);
}

char Lexer::current_char() const {
    if (pos_ >= source_.length()) {
        return '\0';
    }
    return source_[pos_];
}

char Lexer::peek_char(int offset) const {
    size_t peek_pos = pos_ + offset;
    if (peek_pos >= source_.length()) {
        return '\0';
    }
    return source_[peek_pos];
}

void Lexer::advance() {
    if (pos_ < source_.length()) {
        if (source_[pos_] == '\n') {
            line_++;
            column_ = 1;
        } else {
            column_++;
        }
        pos_++;
    }
}

void Lexer::skip_whitespace() {
    while (pos_ < source_.length() && std::isspace(current_char())) {
        advance();
    }
}

Token Lexer::read_identifier() {
    int start_line = line_;
    int start_column = column_;
    std::string text;

    while (is_alnum(current_char()) || current_char() == '_') {
        text += current_char();
        advance();
    }

    TokenType type = get_keyword_type(text);
    if (type == TokenType::Unknown) {
        type = TokenType::Identifier;
    }

    return Token(type, text, start_line, start_column);
}

Token Lexer::read_number() {
    int start_line = line_;
    int start_column = column_;
    std::string text;

    while (is_digit(current_char()) || current_char() == '.') {
        text += current_char();
        advance();
    }

    // サフィックス (L, U, F など)
    if (is_alpha(current_char())) {
        text += current_char();
        advance();
    }

    return Token(TokenType::IntLiteral, text, start_line, start_column);
}

Token Lexer::read_string(char quote) {
    int start_line = line_;
    int start_column = column_;
    std::string text;

    text += current_char();  // 開始のクォート
    advance();

    while (current_char() != '\0' && current_char() != quote) {
        if (current_char() == '\\') {
            text += current_char();
            advance();
            if (current_char() != '\0') {
                text += current_char();
                advance();
            }
        } else {
            text += current_char();
            advance();
        }
    }

    if (current_char() == quote) {
        text += current_char();  // 終了のクォート
        advance();
    }

    TokenType type = (quote == '"') ? TokenType::StringLiteral : TokenType::CharLiteral;
    return Token(type, text, start_line, start_column);
}

Token Lexer::read_comment() {
    int start_line = line_;
    int start_column = column_;
    std::string text;

    if (current_char() == '/' && peek_char() == '/') {
        // 行コメント
        while (current_char() != '\0' && current_char() != '\n') {
            text += current_char();
            advance();
        }
    } else if (current_char() == '/' && peek_char() == '*') {
        // ブロックコメント
        text += current_char();
        advance();
        text += current_char();
        advance();

        while (current_char() != '\0') {
            if (current_char() == '*' && peek_char() == '/') {
                text += current_char();
                advance();
                text += current_char();
                advance();
                break;
            }
            text += current_char();
            advance();
        }
    }

    return Token(TokenType::Comment, text, start_line, start_column);
}

Token Lexer::read_preprocessor() {
    int start_line = line_;
    int start_column = column_;
    std::string text;

    while (current_char() != '\0' && current_char() != '\n') {
        text += current_char();
        advance();
    }

    return Token(TokenType::Preprocessor, text, start_line, start_column);
}

TokenType Lexer::get_keyword_type(const std::string& word) const {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"class", TokenType::Class},         {"struct", TokenType::Struct},
        {"enum", TokenType::Enum},           {"namespace", TokenType::Namespace},
        {"using", TokenType::Using},         {"typedef", TokenType::Typedef},
        {"public", TokenType::Public},       {"protected", TokenType::Protected},
        {"private", TokenType::Private},     {"virtual", TokenType::Virtual},
        {"override", TokenType::Override},   {"final", TokenType::Final},
        {"static", TokenType::Static},       {"const", TokenType::Const},
        {"constexpr", TokenType::Constexpr}, {"mutable", TokenType::Mutable},
        {"void", TokenType::Void},           {"int", TokenType::Int},
        {"bool", TokenType::Bool},           {"char", TokenType::Char},
        {"float", TokenType::Float},         {"double", TokenType::Double},
        {"auto", TokenType::Auto},           {"unsigned", TokenType::Unsigned},
        {"signed", TokenType::Signed},       {"long", TokenType::Long},
        {"short", TokenType::Short},         {"if", TokenType::If},
        {"else", TokenType::Else},           {"for", TokenType::For},
        {"while", TokenType::While},         {"do", TokenType::Do},
        {"switch", TokenType::Switch},       {"case", TokenType::Case},
        {"default", TokenType::Default},     {"return", TokenType::Return},
        {"break", TokenType::Break},         {"continue", TokenType::Continue},
        {"template", TokenType::Template},   {"typename", TokenType::Typename},
    };

    auto it = keywords.find(word);
    if (it != keywords.end()) {
        return it->second;
    }

    return TokenType::Unknown;
}

bool Lexer::is_alpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::is_digit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::is_alnum(char c) const {
    return is_alpha(c) || is_digit(c);
}

}  // namespace parser
}  // namespace cclint
