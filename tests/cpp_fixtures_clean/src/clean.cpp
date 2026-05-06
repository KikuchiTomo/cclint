#include "clean.h"

WidgetClassA::WidgetClassA(int initial)
    : private_count_(initial), private_value_(0) {}

WidgetClassA::~WidgetClassA() = default;

void WidgetClassA::draw() const {
}

void WidgetClassA::configure(int a, int b, int c, int d, int e) {
    private_count_ = a + b + c + d + e;
}

int compute_sum(int x, int y) {
    return x + y;
}

int main() {
    WidgetClassA widget(10);
    widget.draw();

    if (widget.get_count() > 0) {
        widget.configure(1, 2, 3, 4, 5);
    } else {
        widget.configure(0, 0, 0, 0, 0);
    }

    for (int i = 0; i < 3; ++i) {
        widget.configure(i, i, i, i, i);
    }

    int n = 10;
    while (n > 0) {
        --n;
    }

    int v = 5;
    [[maybe_unused]] double d = static_cast<double>(v);

    return 0;
}
