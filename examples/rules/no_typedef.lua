-- ルール: typedef 禁止 (using を使う)
cclint.register("no-typedef", {
  description = "typedef は禁止 (using を使うこと)",
  severity = "warning",
  match = function(n) return n.kind == "TypedefDecl" end,
  check = function(n)
    cclint.report_warn(n, string.format(
      "typedef `%s` は禁止: `using %s = ...;` を使ってください", n.name or "?", n.name or "T"))
  end,
})
