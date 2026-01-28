#include <iostream>
#include <new>

class NameHello {
public:
    NameHello() {}
    ~NameHello() {}

    void hello() { std::cout << "hello\n"; }
    void _hello() { std::cout << "_hello\n"; }

private:
    void world() { std::cout << "world\n"; }
    void _world() { std::cout << "_world\n"; }

protected:
    int helloWorld = 0;
};

int main(void) {
    std::cout << "hello world\n";

    NameHello *nh = new NameHello();

    nh->_hello();
    nh->hello();
    
    delete nh;
    return 0;
}