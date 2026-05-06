-- ルール: 自由関数は snake_case (メンバ関数・コンストラクタは対象外)
cclint.register("function-snake-case", {
  description = "自由関数 (FunctionDecl) は snake_case であること",
  severity = "warning",
  match = function(n) return n.kind == "FunctionDecl" end,
  check = function(n)
    local name = n.name or ""
    if name == "" then return end
    if name == "main" then return end
    if name:match("^operator") then return end
    if not name:match("^[a-z][a-z0-9_]*$") then
      cclint.report_warn(n, string.format("関数名 `%s` は snake_case ではありません", name))
    end
  end,
})
