#pragma once

// using namespace in header (no-using-namespace-in-header)
#include <vector>
namespace foo_bar { struct X {}; }
using namespace foo_bar;

// 素の enum (forbid-plain-enum)
enum Color { Red, Green, Blue };

// PascalCase 違反 (class-pascal-case)
class lower_case_class {
public:
    virtual void f();          // 仮想関数あるが virtual デストラクタなし (virtual-destructor-required)
    ~lower_case_class();
};

// 単一引数コンストラクタ explicit 無し (explicit-single-arg-ctor)
class Foo {
public:
    Foo(int x);
};

// typedef 禁止 (no-typedef)
typedef int MyInt;

// 引数 6 個 (max-function-params)
inline void too_many_params(int a, int b, int c, int d, int e, int f) { (void)a;(void)b;(void)c;(void)d;(void)e;(void)f; }
