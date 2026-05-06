-- ルール: グローバル変数禁止 (const/constexpr は OK)
cclint.register("no-global-variable", {
  description = "グローバルなミュータブル変数は禁止",
  severity = "warning",
  match = function(n) return n.kind == "TranslationUnit" end,
  check = function(n)
    for _, c in ipairs(n.children or {}) do
      if c.kind == "VarDecl" then
        local ty = c.type_name or ""
        if not ty:match("^const") and not ty:match("constexpr") then
          cclint.report_warn(c, string.format(
            "グローバル変数 `%s` は禁止 (const/constexpr のみ可)", c.name or "?"))
        end
      end
    end
  end,
})
