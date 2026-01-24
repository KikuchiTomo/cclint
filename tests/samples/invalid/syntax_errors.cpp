// This file intentionally contains syntax errors for testing error handling

#include <iostream>

// Missing semicolon
class MyClass {
public:
    void method()  // Missing semicolon
}

// Mismatched braces
void function1() {
    if (true) {
        std::cout << "test" << std::endl;
    // Missing closing brace

// Undefined type
void function2() {
    UndefinedType var;  // Type not defined
}

// Missing return statement
int function3() {
    int x = 10;
    // Missing return statement
}

int main() {
    // Unclosed string
    std::cout << "Hello World << std::endl;

    // Undefined variable
    std::cout << undefined_var << std::endl;

    return 0
// Missing semicolon and closing brace
