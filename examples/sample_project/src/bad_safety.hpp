// Intentional violations: safety rules
// Expected: destructor_virtual, constructor_explicit, no_throw_in_destructor

#pragma once

#include <stdexcept>
#include <string>

// BAD: base class destructor not virtual
class Base {
public:
    Base() = default;
    ~Base() = default;

    std::string name() const { return name_; }

protected:
    std::string name_ = "base";
};

class Derived : public Base {
public:
    Derived() { name_ = "derived"; }
    ~Derived() = default;
};

// BAD: single-arg constructor not explicit
class Connection {
public:
    Connection(int port) : port_(port) {}

    // BAD: throw in destructor
    ~Connection() noexcept(false) {
        if (port_ < 0) {
            throw std::runtime_error("invalid port");
        }
    }

    int port() const { return port_; }

private:
    int port_;
};
