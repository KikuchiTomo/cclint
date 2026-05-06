-- ルール: ClassA で終わるクラスの private メンバは `private_` で始めること
-- 日本語のメッセージ・ルール名にも対応している例。

cclint.register("private-prefix-for-classA", {
  description = "ClassA suffix を持つクラスの private メンバは private_ プレフィックス必須",
  severity = "warning",

  match = function(node)
    return node.kind == "ClassDecl" or node.kind == "StructDecl"
  end,

  check = function(node, ctx)
    local n = node.name or ""
    if not n:match("ClassA$") then return end

    for _, child in ipairs(node.children) do
      if child.kind == "FieldDecl" and child.access == "private" then
        if not (child.name and child.name:match("^private_")) then
          cclint.report_warn(child, string.format(
            "クラス `%s` の private メンバ `%s` は `private_` で始めてください",
            n, child.name or "?"))
        end
      end
    end
  end,
})
