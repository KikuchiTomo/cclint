#include "violations.h"

// グローバル変数 (no-global-variable)
int g_counter = 0;

// 関数名が CamelCase (function-snake-case)
int BadFunctionName(int x) { return x + 1; }

// C スタイルキャスト (no-c-style-cast)
double convert_value(int x) {
    return (double)x;
}

// require-braces: 中括弧無し if / for / while
void run_things(int n) {
    if (n > 0) g_counter = 1;
    else g_counter = 2;

    for (int i = 0; i < n; ++i) g_counter += i;

    while (n > 0) --n;
}
