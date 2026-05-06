-- ルール: ヘッダで `using namespace` を使わない (汚染防止)
cclint.register("no-using-namespace-in-header", {
  description = "ヘッダファイルで using namespace は禁止",
  severity = "error",
  match = function(n) return n.kind == "UsingDirective" end,
  check = function(n)
    local span = n.span
    if not span then return end
    local f = span.file or ""
    if f:match("%.h$") or f:match("%.hpp$") or f:match("%.hxx$") then
      cclint.report_error(n, string.format(
        "ヘッダ内の `using namespace %s` は禁止です", n.name or "?"))
    end
  end,
})
