// Violation: Multiple classes in one file

#include <iostream>

class FirstClass {
public:
    void do_something() {
        std::cout << "First class" << std::endl;
    }

private:
    int data_;
};

// Violation: Second class in the same file
class SecondClass {
public:
    void do_another_thing() {
        std::cout << "Second class" << std::endl;
    }

private:
    int value_;
};

// This file should trigger the "one class per file" rule
