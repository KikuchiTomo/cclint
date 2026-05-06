// 違反: _internal_helper を別ファイルから呼んでいる
extern void engine_internal_helper();

void caller_in_a() {
    engine_internal_helper();
}
