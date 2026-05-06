// pragma once が無いのでルール違反 (header_pragma_once)
#ifndef FOO_H
#define FOO_H

class FooClassA {
public:
    int x;       // 違反: ClassA suffix のクラスで private_ プレフィックス無し
private:
    int bad_y;   // 違反: private なのに private_ で始まらない
    int private_z; // OK
};

class Bar {
public:
    int anything; // ClassA suffix ではないので対象外
};

#endif
