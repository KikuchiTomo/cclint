#include "foo.h"

int main() {
    int* p = new int(42);   // 違反: グローバルスコープでの素の new
    delete p;
    return 0;
}
