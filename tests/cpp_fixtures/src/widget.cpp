// 違反: widget_internal.h は widget_internal.cpp からのみ include 可だが
// ここから include している (include-restriction)．
#include "widget_internal.h"

void run_widget() {
    // 違反: init_secret は main からのみ呼び出せる (call-only-from-main)
    init_secret();
}
