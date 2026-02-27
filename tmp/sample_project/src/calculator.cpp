#include "calculator.hpp"

Calculator::Calculator() : value_(0), last_result_(0) {}

void Calculator::set_value(int val) {
    value_ = val;
    __update_state();
}

int Calculator::get_value() const {
    return __calculate_internal();
}

int Calculator::add(int a, int b) {
    last_result_ = a + b;
    return last_result_;
}

int Calculator::subtract(int a, int b) {
    last_result_ = a - b;
    return last_result_;
}

int Calculator::__calculate_internal() const {
    return value_ + last_result_;
}

void Calculator::__update_state() {
    last_result_ = value_;
}
