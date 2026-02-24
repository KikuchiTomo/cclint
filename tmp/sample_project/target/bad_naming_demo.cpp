#include "bad_naming.hpp"
#include <iostream>

int main() {
    WrongName obj;
    obj.SetValue(42);

    std::cout << "Value: " << obj.GetValue() << std::endl;

    return 0;
}
