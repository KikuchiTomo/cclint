-- ルール: C スタイルキャスト禁止 (static_cast / reinterpret_cast 等を使う)
cclint.register("no-c-style-cast", {
  description = "C スタイルキャスト `(T)x` は禁止",
  severity = "warning",
  match = function(n) return n.kind == "CStyleCastExpr" end,
  check = function(n)
    cclint.report_warn(n, "C スタイルキャストは禁止: static_cast / reinterpret_cast / const_cast を使ってください")
  end,
})
