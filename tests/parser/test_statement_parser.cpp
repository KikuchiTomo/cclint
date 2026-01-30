#include <cassert>
#include <iostream>
#include <memory>

#include "parser/builtin_parser.hpp"

using namespace cclint::parser;

void test_if_statements() {
    std::cout << "Testing if statements...\n";

    // Test: simple if
    {
        std::string code = R"(
            void test() {
                if (x > 0) {
                    return;
                }
            }
        )";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    // Test: if-else
    {
        std::string code = R"(
            void test() {
                if (x > 0) {
                    return 1;
                } else {
                    return 0;
                }
            }
        )";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    // Test: if-else if-else
    {
        std::string code = R"(
            void test() {
                if (x > 0) {
                    return 1;
                } else if (x < 0) {
                    return -1;
                } else {
                    return 0;
                }
            }
        )";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    std::cout << "  ✓ If statements passed\n";
}

void test_loop_statements() {
    std::cout << "Testing loop statements...\n";

    // Test: for loop
    {
        std::string code = R"(
            void test() {
                for (int i = 0; i < 10; i++) {
                    sum += i;
                }
            }
        )";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    // Test: range-based for
    {
        std::string code = R"(
            void test() {
                for (auto& item : container) {
                    process(item);
                }
            }
        )";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    // Test: while loop
    {
        std::string code = R"(
            void test() {
                while (condition) {
                    work();
                }
            }
        )";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    // Test: do-while loop
    {
        std::string code = R"(
            void test() {
                do {
                    work();
                } while (condition);
            }
        )";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    std::cout << "  ✓ Loop statements passed\n";
}

void test_switch_statements() {
    std::cout << "Testing switch statements...\n";

    {
        std::string code = R"(
            void test() {
                switch (value) {
                    case 1:
                        return 1;
                    case 2:
                        return 2;
                    default:
                        return 0;
                }
            }
        )";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    std::cout << "  ✓ Switch statements passed\n";
}

void test_try_catch_statements() {
    std::cout << "Testing try-catch statements...\n";

    {
        std::string code = R"(
            void test() {
                try {
                    risky();
                } catch (const std::exception& e) {
                    handle(e);
                } catch (...) {
                    cleanup();
                }
            }
        )";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    std::cout << "  ✓ Try-catch statements passed\n";
}

void test_jump_statements() {
    std::cout << "Testing jump statements...\n";

    // Test: return
    {
        std::string code = R"(
            int test() {
                return 42;
            }
        )";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    // Test: break and continue
    {
        std::string code = R"(
            void test() {
                for (int i = 0; i < 10; i++) {
                    if (i == 5) continue;
                    if (i == 8) break;
                }
            }
        )";
        BuiltinParser parser(code, "test.cpp", false);
        auto ast = parser.parse();
        assert(ast != nullptr);
    }

    std::cout << "  ✓ Jump statements passed\n";
}

int main() {
    std::cout << "Running statement parser tests...\n\n";

    test_if_statements();
    test_loop_statements();
    test_switch_statements();
    test_try_catch_statements();
    test_jump_statements();

    std::cout << "\n✅ All statement parser tests passed!\n";
    return 0;
}
