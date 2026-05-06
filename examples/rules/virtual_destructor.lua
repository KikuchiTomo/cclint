-- ルール: 仮想関数を持つクラスは virtual デストラクタ必須
cclint.register("virtual-destructor-required", {
  description = "polymorphic なクラスは virtual デストラクタを持つこと",
  severity = "error",
  match = function(n) return n.kind == "ClassDecl" or n.kind == "StructDecl" end,
  check = function(n)
    if not n.is_definition then return end
    local has_virtual = false
    local has_destructor = false
    local destructor_virtual = false
    for _, c in ipairs(n.children or {}) do
      if (c.kind == "CXXMethodDecl" or c.kind == "Method") and c.is_virtual then has_virtual = true end
      if c.kind == "Destructor" then
        has_destructor = true
        if c.is_virtual then destructor_virtual = true end
      end
    end
    if has_virtual and (not has_destructor or not destructor_virtual) then
      cclint.report_error(n, string.format(
        "クラス `%s` は仮想関数を持つので virtual デストラクタが必要です", n.name or "?"))
    end
  end,
})
