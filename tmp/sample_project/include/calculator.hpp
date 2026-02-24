#pragma once

// Good example: Follows naming conventions
class Calculator {
public:
    Calculator();

    void set_value(int val);
    int get_value() const;
    int add(int a, int b);
    int subtract(int a, int b);

private:
    int __calculate_internal() const;
    void __update_state();

    int value_;
    int last_result_;
};
