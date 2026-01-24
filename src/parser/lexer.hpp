#pragma once

#include <string>
#include <vector>

namespace cclint {
namespace parser {

/// トークンの種類
enum class TokenType {
    // リテラル
    Identifier,      // 識別子
    IntLiteral,      // 整数リテラル
    StringLiteral,   // 文字列リテラル
    CharLiteral,     // 文字リテラル

    // キーワード
    Class, Struct, Enum, Namespace, Using, Typedef,
    Public, Protected, Private,
    Virtual, Override, Final, Static, Const, Constexpr, Mutable,
    Void, Int, Bool, Char, Float, Double, Auto,
    Unsigned, Signed, Long, Short,
    If, Else, For, While, Do, Switch, Case, Default,
    Return, Break, Continue,
    Template, Typename,

    // 演算子・記号
    LeftParen,       // (
    RightParen,      // )
    LeftBrace,       // {
    RightBrace,      // }
    LeftBracket,     // [
    RightBracket,    // ]
    Semicolon,       // ;
    Comma,           // ,
    Colon,           // :
    Scope,           // ::
    Arrow,           // ->
    Dot,             // .
    Asterisk,        // *
    Ampersand,       // &
    Less,            // <
    Greater,         // >
    Equal,           // =
    Plus,            // +
    Minus,           // -
    Slash,           // /
    Percent,         // %

    // その他
    Comment,         // コメント
    Preprocessor,    // プリプロセッサディレクティブ
    Eof,             // ファイル終端
    Unknown
};

/// トークン
struct Token {
    TokenType type;
    std::string text;
    int line = 0;
    int column = 0;

    Token() : type(TokenType::Unknown) {}
    Token(TokenType t, const std::string& txt, int ln, int col)
        : type(t), text(txt), line(ln), column(col) {}

    bool is(TokenType t) const { return type == t; }
    bool is_keyword() const;
    std::string get_type_name() const;
};

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

} // namespace parser
} // namespace cclint
