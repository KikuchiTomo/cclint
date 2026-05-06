-- ルール: クラス・構造体名は PascalCase
cclint.register("class-pascal-case", {
  description = "class/struct 名は PascalCase であること",
  severity = "warning",
  match = function(n) return n.kind == "ClassDecl" or n.kind == "StructDecl" end,
  check = function(n)
    if not n.is_definition then return end
    local name = n.name or ""
    if name == "" then return end
    if not name:match("^[A-Z][A-Za-z0-9]*$") then
      cclint.report_warn(n, string.format("クラス名 `%s` は PascalCase ではありません", name))
    end
  end,
})
