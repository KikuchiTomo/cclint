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

int main() {
    return 0;
}
