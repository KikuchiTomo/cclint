#include <iostream>

class my_bad_class {  // Bad: should be PascalCase
public:
    void GetValue() { std::cout << "test"; }  // Bad: should be snake_case, no cout
    int* ptr = new int(5);  // Bad: no new in class

private:
    int data;  // Bad: should have trailing underscore
};

void BadFunction() {  // Bad: should be snake_case
    int magic = 42;  // Bad: magic number
}
