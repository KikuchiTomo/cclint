#include "parser/enhanced_lexer.hpp"
#include "parser/token_types_enhanced.hpp"

#include <cassert>
#include <iostream>

using namespace cclint::parser;

void test_keywords() {
    EnhancedLexer lexer("class int void auto const constexpr", "test.cpp");
    auto tokens = lexer.tokenize();

    assert(tokens.size() == 7);  // 6 keywords + EOF
    assert(tokens[0].type == TokenType::Class);
    assert(tokens[1].type == TokenType::Int);
    assert(tokens[2].type == TokenType::Void);
    assert(tokens[3].type == TokenType::Auto);
    assert(tokens[4].type == TokenType::Const);
    assert(tokens[5].type == TokenType::Constexpr);

    std::cout << "✓ Keywords test passed\n";
}

void test_operators() {
    EnhancedLexer lexer("+ - * / ++ -- == != <= >= <=> && || << >>", "test.cpp");
    auto tokens = lexer.tokenize();

    assert(tokens[0].type == TokenType::Plus);
    assert(tokens[1].type == TokenType::Minus);
    assert(tokens[2].type == TokenType::Star);
    assert(tokens[3].type == TokenType::Slash);
    assert(tokens[4].type == TokenType::PlusPlus);
    assert(tokens[5].type == TokenType::MinusMinus);
    assert(tokens[6].type == TokenType::Equal);
    assert(tokens[7].type == TokenType::NotEqual);
    assert(tokens[8].type == TokenType::LessEqual);
    assert(tokens[9].type == TokenType::GreaterEqual);
    assert(tokens[10].type == TokenType::Spaceship);
    assert(tokens[11].type == TokenType::LogicalAnd);
    assert(tokens[12].type == TokenType::LogicalOr);
    assert(tokens[13].type == TokenType::LeftShift);
    assert(tokens[14].type == TokenType::RightShift);

    std::cout << "✓ Operators test passed\n";
}

void test_numeric_literals() {
    EnhancedLexer lexer("123 0x7F 0b1010 0777 3.14 1e-5 0x1.2p3", "test.cpp");
    auto tokens = lexer.tokenize();

    assert(tokens[0].type == TokenType::IntegerLiteral);
    assert(tokens[0].text == "123");

    assert(tokens[1].type == TokenType::IntegerLiteral);
    assert(tokens[1].text == "0x7F");

    assert(tokens[2].type == TokenType::IntegerLiteral);
    assert(tokens[2].text == "0b1010");

    assert(tokens[3].type == TokenType::IntegerLiteral);
    assert(tokens[3].text == "0777");

    assert(tokens[4].type == TokenType::FloatingLiteral);
    assert(tokens[4].text == "3.14");

    assert(tokens[5].type == TokenType::FloatingLiteral);
    assert(tokens[5].text == "1e-5");

    assert(tokens[6].type == TokenType::FloatingLiteral);
    assert(tokens[6].text == "0x1.2p3");

    std::cout << "✓ Numeric literals test passed\n";
}

void test_string_literals() {
    EnhancedLexer lexer(R"("hello" L"wide" u8"utf8" u"utf16" U"utf32")", "test.cpp");
    auto tokens = lexer.tokenize();

    assert(tokens[0].type == TokenType::StringLiteral);
    assert(tokens[1].type == TokenType::WideStringLiteral);
    assert(tokens[2].type == TokenType::Utf8StringLiteral);
    assert(tokens[3].type == TokenType::Utf16StringLiteral);
    assert(tokens[4].type == TokenType::Utf32StringLiteral);

    std::cout << "✓ String literals test passed\n";
}

void test_character_literals() {
    EnhancedLexer lexer(R"('a' L'b' u8'c' u'd' U'e')", "test.cpp");
    auto tokens = lexer.tokenize();

    assert(tokens[0].type == TokenType::CharLiteral);
    assert(tokens[1].type == TokenType::WideCharLiteral);
    assert(tokens[2].type == TokenType::Utf8CharLiteral);
    assert(tokens[3].type == TokenType::Utf16CharLiteral);
    assert(tokens[4].type == TokenType::Utf32CharLiteral);

    std::cout << "✓ Character literals test passed\n";
}

void test_comments() {
    EnhancedLexer lexer("// line comment\n/* block comment */", "test.cpp");
    auto tokens = lexer.tokenize();

    assert(tokens[0].type == TokenType::LineComment);
    assert(tokens[1].type == TokenType::BlockComment);

    std::cout << "✓ Comments test passed\n";
}

void test_preprocessor() {
    EnhancedLexer lexer("#include <iostream>\n#define MAX 100\n#ifdef DEBUG", "test.cpp");
    auto tokens = lexer.tokenize();

    assert(tokens[0].type == TokenType::PPInclude);
    assert(tokens[1].type == TokenType::PPDefine);
    assert(tokens[2].type == TokenType::PPIfdef);

    std::cout << "✓ Preprocessor test passed\n";
}

void test_raw_string() {
    std::string code = "R\"(raw string)\"";
    EnhancedLexer lexer(code, "test.cpp");
    auto tokens = lexer.tokenize();

    assert(tokens[0].type == TokenType::RawStringLiteral);

    std::cout << "✓ Raw string test passed\n";
}

void test_user_defined_literal() {
    EnhancedLexer lexer("123_km 3.14_m", "test.cpp");
    auto tokens = lexer.tokenize();

    assert(tokens[0].type == TokenType::UserDefinedLiteral);
    assert(tokens[0].text == "123_km");

    assert(tokens[1].type == TokenType::UserDefinedLiteral);
    assert(tokens[1].text == "3.14_m");

    std::cout << "✓ User-defined literal test passed\n";
}

int main() {
    std::cout << "Testing Enhanced Lexer...\n\n";

    test_keywords();
    test_operators();
    test_numeric_literals();
    test_string_literals();
    test_character_literals();
    test_comments();
    test_preprocessor();
    test_raw_string();
    test_user_defined_literal();

    std::cout << "\n✓ All Enhanced Lexer tests passed!\n";
    return 0;
}
