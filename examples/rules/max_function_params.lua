-- ルール: 関数の引数は 5 個まで
cclint.register("max-function-params", {
  description = "関数の引数は 5 個まで",
  severity = "warning",
  match = function(n)
    return n.kind == "FunctionDecl" or n.kind == "CXXMethodDecl"
        or n.kind == "Method" or n.kind == "Constructor"
  end,
  check = function(n)
    local count = 0
    for _, c in ipairs(n.children or {}) do
      if c.kind == "ParmDecl" then count = count + 1 end
    end
    if count > 5 then
      cclint.report_warn(n, string.format(
        "関数 `%s` の引数が %d 個 (上限 5)", n.name or "?", count))
    end
  end,
})
