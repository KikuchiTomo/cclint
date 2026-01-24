#include <iostream>

// Bad: Class name should be CamelCase, not snake_case
class my_class {
public:
    // Bad: Function name should be snake_case, not CamelCase
    void DoSomething() {
        std::cout << "Doing something" << std::endl;
    }

    // Bad: Member variable should have trailing underscore
    int memberVariable;

    // Bad: Constant should be UPPER_CASE
    static const int maxValue = 100;
};

// Bad: Function name should be snake_case
void CalculateSum(int a, int b) {
    int result = a + b;
    std::cout << result << std::endl;
}

int main() {
    my_class obj;
    obj.DoSomething();
    CalculateSum(10, 20);
    return 0;
}
