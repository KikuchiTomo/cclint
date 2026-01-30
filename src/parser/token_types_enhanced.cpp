#include "parser/token_types_enhanced.hpp"

#include <unordered_map>

namespace cclint {
namespace parser {

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TokenType::Eof:
            return "Eof";

        // Keywords
        case TokenType::Alignas:
            return "alignas";
        case TokenType::Alignof:
            return "alignof";
        case TokenType::Asm:
            return "asm";
        case TokenType::Auto:
            return "auto";
        case TokenType::Bool:
            return "bool";
        case TokenType::Break:
            return "break";
        case TokenType::Case:
            return "case";
        case TokenType::Catch:
            return "catch";
        case TokenType::Char:
            return "char";
        case TokenType::Char8_t:
            return "char8_t";
        case TokenType::Char16_t:
            return "char16_t";
        case TokenType::Char32_t:
            return "char32_t";
        case TokenType::Class:
            return "class";
        case TokenType::Const:
            return "const";
        case TokenType::Consteval:
            return "consteval";
        case TokenType::Constexpr:
            return "constexpr";
        case TokenType::Constinit:
            return "constinit";
        case TokenType::Const_cast:
            return "const_cast";
        case TokenType::Continue:
            return "continue";
        case TokenType::Co_await:
            return "co_await";
        case TokenType::Co_return:
            return "co_return";
        case TokenType::Co_yield:
            return "co_yield";
        case TokenType::Decltype:
            return "decltype";
        case TokenType::Default:
            return "default";
        case TokenType::Delete:
            return "delete";
        case TokenType::Do:
            return "do";
        case TokenType::Double:
            return "double";
        case TokenType::Dynamic_cast:
            return "dynamic_cast";
        case TokenType::Else:
            return "else";
        case TokenType::Enum:
            return "enum";
        case TokenType::Explicit:
            return "explicit";
        case TokenType::Export:
            return "export";
        case TokenType::Extern:
            return "extern";
        case TokenType::False:
            return "false";
        case TokenType::Float:
            return "float";
        case TokenType::For:
            return "for";
        case TokenType::Friend:
            return "friend";
        case TokenType::Goto:
            return "goto";
        case TokenType::If:
            return "if";
        case TokenType::Inline:
            return "inline";
        case TokenType::Int:
            return "int";
        case TokenType::Long:
            return "long";
        case TokenType::Mutable:
            return "mutable";
        case TokenType::Namespace:
            return "namespace";
        case TokenType::New:
            return "new";
        case TokenType::Noexcept:
            return "noexcept";
        case TokenType::Nullptr:
            return "nullptr";
        case TokenType::Operator:
            return "operator";
        case TokenType::Private:
            return "private";
        case TokenType::Protected:
            return "protected";
        case TokenType::Public:
            return "public";
        case TokenType::Register:
            return "register";
        case TokenType::Reinterpret_cast:
            return "reinterpret_cast";
        case TokenType::Requires:
            return "requires";
        case TokenType::Return:
            return "return";
        case TokenType::Short:
            return "short";
        case TokenType::Signed:
            return "signed";
        case TokenType::Sizeof:
            return "sizeof";
        case TokenType::Static:
            return "static";
        case TokenType::Static_assert:
            return "static_assert";
        case TokenType::Static_cast:
            return "static_cast";
        case TokenType::Struct:
            return "struct";
        case TokenType::Switch:
            return "switch";
        case TokenType::Template:
            return "template";
        case TokenType::This:
            return "this";
        case TokenType::Thread_local:
            return "thread_local";
        case TokenType::Throw:
            return "throw";
        case TokenType::True:
            return "true";
        case TokenType::Try:
            return "try";
        case TokenType::Typedef:
            return "typedef";
        case TokenType::Typeid:
            return "typeid";
        case TokenType::Typename:
            return "typename";
        case TokenType::Union:
            return "union";
        case TokenType::Unsigned:
            return "unsigned";
        case TokenType::Using:
            return "using";
        case TokenType::Virtual:
            return "virtual";
        case TokenType::Void:
            return "void";
        case TokenType::Volatile:
            return "volatile";
        case TokenType::Wchar_t:
            return "wchar_t";
        case TokenType::While:
            return "while";
        case TokenType::Concept:
            return "concept";

        // Operators
        case TokenType::Plus:
            return "+";
        case TokenType::Minus:
            return "-";
        case TokenType::Star:
            return "*";
        case TokenType::Slash:
            return "/";
        case TokenType::Percent:
            return "%";
        case TokenType::PlusPlus:
            return "++";
        case TokenType::MinusMinus:
            return "--";
        case TokenType::Equal:
            return "==";
        case TokenType::NotEqual:
            return "!=";
        case TokenType::Less:
            return "<";
        case TokenType::Greater:
            return ">";
        case TokenType::LessEqual:
            return "<=";
        case TokenType::GreaterEqual:
            return ">=";
        case TokenType::Spaceship:
            return "<=>";
        case TokenType::LogicalAnd:
            return "&&";
        case TokenType::LogicalOr:
            return "||";
        case TokenType::LogicalNot:
            return "!";
        case TokenType::Ampersand:
            return "&";
        case TokenType::Pipe:
            return "|";
        case TokenType::Caret:
            return "^";
        case TokenType::Tilde:
            return "~";
        case TokenType::LeftShift:
            return "<<";
        case TokenType::RightShift:
            return ">>";
        case TokenType::Assign:
            return "=";
        case TokenType::PlusAssign:
            return "+=";
        case TokenType::MinusAssign:
            return "-=";
        case TokenType::StarAssign:
            return "*=";
        case TokenType::SlashAssign:
            return "/=";
        case TokenType::PercentAssign:
            return "%=";
        case TokenType::AmpersandAssign:
            return "&=";
        case TokenType::PipeAssign:
            return "|=";
        case TokenType::CaretAssign:
            return "^=";
        case TokenType::LeftShiftAssign:
            return "<<=";
        case TokenType::RightShiftAssign:
            return ">>=";
        case TokenType::Dot:
            return ".";
        case TokenType::Arrow:
            return "->";
        case TokenType::DotStar:
            return ".*";
        case TokenType::ArrowStar:
            return "->*";
        case TokenType::DoubleColon:
            return "::";
        case TokenType::Question:
            return "?";
        case TokenType::Colon:
            return ":";
        case TokenType::Semicolon:
            return ";";
        case TokenType::Comma:
            return ",";
        case TokenType::Ellipsis:
            return "...";
        case TokenType::LeftParen:
            return "(";
        case TokenType::RightParen:
            return ")";
        case TokenType::LeftBrace:
            return "{";
        case TokenType::RightBrace:
            return "}";
        case TokenType::LeftBracket:
            return "[";
        case TokenType::RightBracket:
            return "]";

        // Literals
        case TokenType::Identifier:
            return "Identifier";
        case TokenType::IntegerLiteral:
            return "IntegerLiteral";
        case TokenType::FloatingLiteral:
            return "FloatingLiteral";
        case TokenType::CharLiteral:
            return "CharLiteral";
        case TokenType::WideCharLiteral:
            return "WideCharLiteral";
        case TokenType::Utf8CharLiteral:
            return "Utf8CharLiteral";
        case TokenType::Utf16CharLiteral:
            return "Utf16CharLiteral";
        case TokenType::Utf32CharLiteral:
            return "Utf32CharLiteral";
        case TokenType::StringLiteral:
            return "StringLiteral";
        case TokenType::WideStringLiteral:
            return "WideStringLiteral";
        case TokenType::Utf8StringLiteral:
            return "Utf8StringLiteral";
        case TokenType::Utf16StringLiteral:
            return "Utf16StringLiteral";
        case TokenType::Utf32StringLiteral:
            return "Utf32StringLiteral";
        case TokenType::RawStringLiteral:
            return "RawStringLiteral";
        case TokenType::UserDefinedLiteral:
            return "UserDefinedLiteral";

        // Preprocessor
        case TokenType::PPDirective:
            return "PPDirective";
        case TokenType::PPInclude:
            return "#include";
        case TokenType::PPDefine:
            return "#define";
        case TokenType::PPUndef:
            return "#undef";
        case TokenType::PPIf:
            return "#if";
        case TokenType::PPIfdef:
            return "#ifdef";
        case TokenType::PPIfndef:
            return "#ifndef";
        case TokenType::PPElif:
            return "#elif";
        case TokenType::PPElse:
            return "#else";
        case TokenType::PPEndif:
            return "#endif";
        case TokenType::PPPragma:
            return "#pragma";
        case TokenType::PPError:
            return "#error";
        case TokenType::PPWarning:
            return "#warning";
        case TokenType::PPLine:
            return "#line";
        case TokenType::MacroParameter:
            return "MacroParameter";
        case TokenType::MacroStringify:
            return "#";
        case TokenType::MacroConcat:
            return "##";

        // Comments
        case TokenType::LineComment:
            return "LineComment";
        case TokenType::BlockComment:
            return "BlockComment";

        // Attributes
        case TokenType::AttributeStart:
            return "[[";
        case TokenType::AttributeEnd:
            return "]]";

        // Special
        case TokenType::Whitespace:
            return "Whitespace";
        case TokenType::Newline:
            return "Newline";
        case TokenType::Unknown:
            return "Unknown";
    }
    return "Unknown";
}

bool is_keyword(TokenType type) {
    return type >= TokenType::Alignas && type <= TokenType::Concept;
}

bool is_operator(TokenType type) {
    return type >= TokenType::Plus && type <= TokenType::RightBracket;
}

bool is_literal(TokenType type) {
    return type >= TokenType::IntegerLiteral && type <= TokenType::UserDefinedLiteral;
}

bool is_type_keyword(TokenType type) {
    switch (type) {
        case TokenType::Bool:
        case TokenType::Char:
        case TokenType::Char8_t:
        case TokenType::Char16_t:
        case TokenType::Char32_t:
        case TokenType::Double:
        case TokenType::Float:
        case TokenType::Int:
        case TokenType::Long:
        case TokenType::Short:
        case TokenType::Signed:
        case TokenType::Unsigned:
        case TokenType::Void:
        case TokenType::Wchar_t:
        case TokenType::Auto:
        case TokenType::Decltype:
            return true;
        default:
            return false;
    }
}

bool is_storage_class(TokenType type) {
    switch (type) {
        case TokenType::Static:
        case TokenType::Extern:
        case TokenType::Thread_local:
        case TokenType::Register:
        case TokenType::Mutable:
            return true;
        default:
            return false;
    }
}

bool is_cv_qualifier(TokenType type) {
    return type == TokenType::Const || type == TokenType::Volatile;
}

TokenType keyword_from_string(const std::string& str) {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"alignas", TokenType::Alignas},
        {"alignof", TokenType::Alignof},
        {"asm", TokenType::Asm},
        {"auto", TokenType::Auto},
        {"bool", TokenType::Bool},
        {"break", TokenType::Break},
        {"case", TokenType::Case},
        {"catch", TokenType::Catch},
        {"char", TokenType::Char},
        {"char8_t", TokenType::Char8_t},
        {"char16_t", TokenType::Char16_t},
        {"char32_t", TokenType::Char32_t},
        {"class", TokenType::Class},
        {"const", TokenType::Const},
        {"consteval", TokenType::Consteval},
        {"constexpr", TokenType::Constexpr},
        {"constinit", TokenType::Constinit},
        {"const_cast", TokenType::Const_cast},
        {"continue", TokenType::Continue},
        {"co_await", TokenType::Co_await},
        {"co_return", TokenType::Co_return},
        {"co_yield", TokenType::Co_yield},
        {"decltype", TokenType::Decltype},
        {"default", TokenType::Default},
        {"delete", TokenType::Delete},
        {"do", TokenType::Do},
        {"double", TokenType::Double},
        {"dynamic_cast", TokenType::Dynamic_cast},
        {"else", TokenType::Else},
        {"enum", TokenType::Enum},
        {"explicit", TokenType::Explicit},
        {"export", TokenType::Export},
        {"extern", TokenType::Extern},
        {"false", TokenType::False},
        {"float", TokenType::Float},
        {"for", TokenType::For},
        {"friend", TokenType::Friend},
        {"goto", TokenType::Goto},
        {"if", TokenType::If},
        {"inline", TokenType::Inline},
        {"int", TokenType::Int},
        {"long", TokenType::Long},
        {"mutable", TokenType::Mutable},
        {"namespace", TokenType::Namespace},
        {"new", TokenType::New},
        {"noexcept", TokenType::Noexcept},
        {"nullptr", TokenType::Nullptr},
        {"operator", TokenType::Operator},
        {"private", TokenType::Private},
        {"protected", TokenType::Protected},
        {"public", TokenType::Public},
        {"register", TokenType::Register},
        {"reinterpret_cast", TokenType::Reinterpret_cast},
        {"requires", TokenType::Requires},
        {"return", TokenType::Return},
        {"short", TokenType::Short},
        {"signed", TokenType::Signed},
        {"sizeof", TokenType::Sizeof},
        {"static", TokenType::Static},
        {"static_assert", TokenType::Static_assert},
        {"static_cast", TokenType::Static_cast},
        {"struct", TokenType::Struct},
        {"switch", TokenType::Switch},
        {"template", TokenType::Template},
        {"this", TokenType::This},
        {"thread_local", TokenType::Thread_local},
        {"throw", TokenType::Throw},
        {"true", TokenType::True},
        {"try", TokenType::Try},
        {"typedef", TokenType::Typedef},
        {"typeid", TokenType::Typeid},
        {"typename", TokenType::Typename},
        {"union", TokenType::Union},
        {"unsigned", TokenType::Unsigned},
        {"using", TokenType::Using},
        {"virtual", TokenType::Virtual},
        {"void", TokenType::Void},
        {"volatile", TokenType::Volatile},
        {"wchar_t", TokenType::Wchar_t},
        {"while", TokenType::While},
        {"concept", TokenType::Concept},
    };

    auto it = keywords.find(str);
    if (it != keywords.end()) {
        return it->second;
    }
    return TokenType::Identifier;
}

}  // namespace parser
}  // namespace cclint
