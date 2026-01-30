#pragma once

#include <string>

namespace cclint {
namespace parser {

/// Enhanced token types for complete C++ support
enum class TokenType {
    // End of file
    Eof,

    // ========== Keywords ==========
    // C++98/03 keywords
    Alignas,           // alignas
    Alignof,           // alignof
    Asm,               // asm
    Auto,              // auto
    Bool,              // bool
    Break,             // break
    Case,              // case
    Catch,             // catch
    Char,              // char
    Char8_t,           // char8_t (C++20)
    Char16_t,          // char16_t
    Char32_t,          // char32_t
    Class,             // class
    Const,             // const
    Consteval,         // consteval (C++20)
    Constexpr,         // constexpr
    Constinit,         // constinit (C++20)
    Const_cast,        // const_cast
    Continue,          // continue
    Co_await,          // co_await (C++20)
    Co_return,         // co_return (C++20)
    Co_yield,          // co_yield (C++20)
    Decltype,          // decltype
    Default,           // default
    Delete,            // delete
    Do,                // do
    Double,            // double
    Dynamic_cast,      // dynamic_cast
    Else,              // else
    Enum,              // enum
    Explicit,          // explicit
    Export,            // export
    Extern,            // extern
    False,             // false
    Float,             // float
    For,               // for
    Friend,            // friend
    Goto,              // goto
    If,                // if
    Inline,            // inline
    Int,               // int
    Long,              // long
    Mutable,           // mutable
    Namespace,         // namespace
    New,               // new
    Noexcept,          // noexcept
    Nullptr,           // nullptr
    Operator,          // operator
    Private,           // private
    Protected,         // protected
    Public,            // public
    Register,          // register
    Reinterpret_cast,  // reinterpret_cast
    Requires,          // requires (C++20)
    Return,            // return
    Short,             // short
    Signed,            // signed
    Sizeof,            // sizeof
    Static,            // static
    Static_assert,     // static_assert
    Static_cast,       // static_cast
    Struct,            // struct
    Switch,            // switch
    Template,          // template
    This,              // this
    Thread_local,      // thread_local
    Throw,             // throw
    True,              // true
    Try,               // try
    Typedef,           // typedef
    Typeid,            // typeid
    Typename,          // typename
    Union,             // union
    Unsigned,          // unsigned
    Using,             // using
    Virtual,           // virtual
    Void,              // void
    Volatile,          // volatile
    Wchar_t,           // wchar_t
    While,             // while

    // Contextual keywords (C++20 Concepts)
    Concept,  // concept

    // ========== Operators ==========
    // Arithmetic
    Plus,     // +
    Minus,    // -
    Star,     // *
    Slash,    // /
    Percent,  // %

    // Increment/Decrement
    PlusPlus,    // ++
    MinusMinus,  // --

    // Comparison
    Equal,         // ==
    NotEqual,      // !=
    Less,          // <
    Greater,       // >
    LessEqual,     // <=
    GreaterEqual,  // >=
    Spaceship,     // <=> (C++20)

    // Logical
    LogicalAnd,  // &&
    LogicalOr,   // ||
    LogicalNot,  // !

    // Bitwise
    Ampersand,   // &
    Pipe,        // |
    Caret,       // ^
    Tilde,       // ~
    LeftShift,   // <<
    RightShift,  // >>

    // Assignment
    Assign,            // =
    PlusAssign,        // +=
    MinusAssign,       // -=
    StarAssign,        // *=
    SlashAssign,       // /=
    PercentAssign,     // %=
    AmpersandAssign,   // &=
    PipeAssign,        // |=
    CaretAssign,       // ^=
    LeftShiftAssign,   // <<=
    RightShiftAssign,  // >>=

    // Member access
    Dot,          // .
    Arrow,        // ->
    DotStar,      // .*
    ArrowStar,    // ->*
    DoubleColon,  // ::

    // Other
    Question,   // ?
    Colon,      // :
    Semicolon,  // ;
    Comma,      // ,
    Ellipsis,   // ...

    // Delimiters
    LeftParen,     // (
    RightParen,    // )
    LeftBrace,     // {
    RightBrace,    // }
    LeftBracket,   // [
    RightBracket,  // ]

    // ========== Literals ==========
    // Identifiers
    Identifier,

    // Numeric literals
    IntegerLiteral,   // 123, 0x7F, 0b1010, 042
    FloatingLiteral,  // 3.14, 1e-5, 0x1.2p3

    // Character literals
    CharLiteral,       // 'a', '\n'
    WideCharLiteral,   // L'a'
    Utf8CharLiteral,   // u8'a' (C++17)
    Utf16CharLiteral,  // u'a'
    Utf32CharLiteral,  // U'a'

    // String literals
    StringLiteral,       // "hello"
    WideStringLiteral,   // L"hello"
    Utf8StringLiteral,   // u8"hello"
    Utf16StringLiteral,  // u"hello"
    Utf32StringLiteral,  // U"hello"
    RawStringLiteral,    // R"(raw)"

    // User-defined literals (handled in parser)
    UserDefinedLiteral,  // 123_km, "hello"_s

    // ========== Preprocessor ==========
    // Preprocessor directives (handled by preprocessor)
    PPDirective,  // #include, #define, etc.
    PPInclude,    // #include
    PPDefine,     // #define
    PPUndef,      // #undef
    PPIf,         // #if
    PPIfdef,      // #ifdef
    PPIfndef,     // #ifndef
    PPElif,       // #elif
    PPElse,       // #else
    PPEndif,      // #endif
    PPPragma,     // #pragma
    PPError,      // #error
    PPWarning,    // #warning
    PPLine,       // #line

    // Macro tokens
    MacroParameter,  // Macro parameter token
    MacroStringify,  // # (stringification)
    MacroConcat,     // ## (token pasting)

    // ========== Comments ==========
    LineComment,   // // comment
    BlockComment,  // /* comment */

    // ========== Attributes ==========
    AttributeStart,  // [[
    AttributeEnd,    // ]]

    // ========== Special ==========
    Whitespace,  // Space, tab (normally skipped)
    Newline,     // \n (important for preprocessor)
    Unknown,     // Unknown token
};

/// Token structure with location and value
struct Token {
    TokenType type;
    std::string text;
    std::string value;  // Processed value (e.g., escape sequences resolved)

    // Source location
    std::string filename;
    int line;
    int column;
    int offset;  // Byte offset from start of file

    // Additional info
    bool has_whitespace_before;  // For macro expansion
    bool is_at_start_of_line;    // For preprocessor

    Token()
        : type(TokenType::Eof),
          line(0),
          column(0),
          offset(0),
          has_whitespace_before(false),
          is_at_start_of_line(false) {}

    Token(TokenType t, const std::string& txt, int l, int c)
        : type(t),
          text(txt),
          value(txt),
          line(l),
          column(c),
          offset(0),
          has_whitespace_before(false),
          is_at_start_of_line(false) {}

    // Explicitly defaulted copy/move constructors and assignment operators
    // to ensure they work correctly across all platforms and optimization levels
    Token(const Token&) = default;
    Token(Token&&) = default;
    Token& operator=(const Token&) = default;
    Token& operator=(Token&&) = default;
};

/// Convert token type to string (for debugging)
const char* token_type_to_string(TokenType type);

/// Check if token is a keyword
bool is_keyword(TokenType type);

/// Check if token is an operator
bool is_operator(TokenType type);

/// Check if token is a literal
bool is_literal(TokenType type);

/// Check if token is a type keyword
bool is_type_keyword(TokenType type);

/// Check if token is a storage class specifier
bool is_storage_class(TokenType type);

/// Check if token is a cv-qualifier
bool is_cv_qualifier(TokenType type);

/// Get keyword from string
TokenType keyword_from_string(const std::string& str);

}  // namespace parser
}  // namespace cclint
