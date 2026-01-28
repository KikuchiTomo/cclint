// Test file for new AST APIs

#include <iostream>
#include <vector>

// Test: Constructor API (should detect non-explicit single-arg constructor)
class BadConstructor {
public:
    BadConstructor(int x) {}  // Missing explicit
};

// Test: Constructor API (good)
class GoodConstructor {
public:
    explicit GoodConstructor(int x) {}
    GoodConstructor() = default;
    GoodConstructor(const GoodConstructor&) = delete;
};

// Test: Destructor API (should detect non-virtual destructor in base class)
class BaseClass {
public:
    ~BaseClass() {}  // Should be virtual
};

class DerivedClass : public BaseClass {
public:
    virtual ~DerivedClass() {}
};

// Test: Destructor API (good)
class GoodBase {
public:
    virtual ~GoodBase() {}
};

// Test: Friend declaration API
class FriendUser {
    friend class OtherClass;
    friend void helper_function();
private:
    int secret;
};

// Test: Operator overload API
class Point {
public:
    int x, y;

    Point operator+(const Point& other) const {
        return Point{x + other.x, y + other.y};
    }

    friend std::ostream& operator<<(std::ostream& os, const Point& p);
};

// Test: Template API
template<typename T>
class Container {
    T value;
};

template<typename T, typename U>
class Pair {
    T first;
    U second;
};

// Test: Template specialization
template<>
class Container<int> {
    int value;
    int extra;
};

// Test: Lambda API
void test_lambdas() {
    int x = 42;

    // Bad: capture by value [=]
    auto bad_lambda = [=]() { return x + 1; };

    // Good: explicit capture
    auto good_lambda = [x]() { return x + 1; };

    // Mutable lambda
    auto mutable_lambda = [x]() mutable { return ++x; };
}

// Test: Static assert API
static_assert(sizeof(int) == 4, "int must be 4 bytes");

// Test: Call graph API - restricted function
void restricted_function() {
    std::cout << "This should only be called from main\n";
}

void unauthorized_caller() {
    restricted_function();  // Should be detected
}

void helper() {
    std::cout << "Helper\n";
}

int main() {
    restricted_function();  // OK
    helper();
    unauthorized_caller();

    test_lambdas();

    return 0;
}
