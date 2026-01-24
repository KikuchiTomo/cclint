#include <iostream>

class Calculator {
public:
    Calculator() = default;
    ~Calculator() = default;

    int add(int a, int b) const {
        return a + b;
    }

    int subtract(int a, int b) const {
        return a - b;
    }

    int multiply(int a, int b) const {
        return a * b;
    }

    double divide(int a, int b) const {
        if (b == 0) {
            throw std::invalid_argument("Division by zero");
        }
        return static_cast<double>(a) / b;
    }
};

int main() {
    Calculator calc;

    std::cout << "10 + 5 = " << calc.add(10, 5) << std::endl;
    std::cout << "10 - 5 = " << calc.subtract(10, 5) << std::endl;
    std::cout << "10 * 5 = " << calc.multiply(10, 5) << std::endl;
    std::cout << "10 / 5 = " << calc.divide(10, 5) << std::endl;

    return 0;
}
