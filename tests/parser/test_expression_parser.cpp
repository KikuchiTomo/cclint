#include <cassert>
#include <iostream>
#include <memory>

#include "parser/builtin_parser.hpp"

using namespace cclint::parser;

void test_arithmetic_expressions() {
    std::cout << "Testing arithmetic expressions...\n";

    // Test: 1 + 2 * 3
    {
        std::string code = "int x = 1 + 2 * 3;";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
        assert(!parser.has_errors());
    }

    // Test: (1 + 2) * 3
    {
        std::string code = "int x = (1 + 2) * 3;";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    // Test: a / b % c
    {
        std::string code = "int x = a / b % c;";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    std::cout << "  ✓ Arithmetic expressions passed\n";
}

void test_logical_expressions() {
    std::cout << "Testing logical expressions...\n";

    // Test: a && b || c
    {
        std::string code = "bool x = a && b || c;";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    // Test: !a && (b || c)
    {
        std::string code = "bool x = !a && (b || c);";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    std::cout << "  ✓ Logical expressions passed\n";
}

void test_comparison_expressions() {
    std::cout << "Testing comparison expressions...\n";

    // Test: a < b && c > d
    {
        std::string code = "bool x = a < b && c > d;";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    // Test: a <= b || c >= d
    {
        std::string code = "bool x = a <= b || c >= d;";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    // Test: a == b && c != d
    {
        std::string code = "bool x = a == b && c != d;";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    std::cout << "  ✓ Comparison expressions passed\n";
}

void test_assignment_expressions() {
    std::cout << "Testing assignment expressions...\n";

    // Test: a = b = c
    {
        std::string code = "int x = a = b = c;";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    // Test: a += b -= c
    {
        std::string code = "int x = a += b -= c;";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    std::cout << "  ✓ Assignment expressions passed\n";
}

void test_ternary_expressions() {
    std::cout << "Testing ternary expressions...\n";

    // Test: a ? b : c
    {
        std::string code = "int x = a ? b : c;";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    // Test: nested ternary
    {
        std::string code = "int x = a ? b : c ? d : e;";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    std::cout << "  ✓ Ternary expressions passed\n";
}

void test_postfix_expressions() {
    std::cout << "Testing postfix expressions...\n";

    // Test: function call
    {
        std::string code = "void test() { func(a, b, c); }";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    // Test: array subscript
    {
        std::string code = "int x = arr[0][1];";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    // Test: member access
    {
        std::string code = "int x = obj.member->field;";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    std::cout << "  ✓ Postfix expressions passed\n";
}

void test_lambda_expressions() {
    std::cout << "Testing lambda expressions...\n";

    // Test: simple lambda
    {
        std::string code = "auto f = [](int x) { return x * 2; };";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    // Test: lambda with capture
    {
        std::string code = "auto f = [&x, y](int z) { return x + y + z; };";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    std::cout << "  ✓ Lambda expressions passed\n";
}

int main() {
    std::cout << "Running expression parser tests...\n\n";

    test_arithmetic_expressions();
    test_logical_expressions();
    test_comparison_expressions();
    test_assignment_expressions();
    test_ternary_expressions();
    test_postfix_expressions();
    test_lambda_expressions();

    std::cout << "\n✅ All expression parser tests passed!\n";
    return 0;
}
