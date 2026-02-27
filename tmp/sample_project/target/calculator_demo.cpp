#include "calculator.hpp"
#include <iostream>

int main() {
    Calculator calc;
    calc.set_value(10);

    std::cout << "Calculator value: " << calc.get_value() << std::endl;
    std::cout << "10 + 5 = " << calc.add(10, 5) << std::endl;
    std::cout << "20 - 8 = " << calc.subtract(20, 8) << std::endl;

    return 0;
}
