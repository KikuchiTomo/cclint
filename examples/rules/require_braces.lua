-- ルール: if/else/for/while の本体は必ず波括弧 `{}` で囲む
-- 単文 (CompoundStmt 以外) のときに警告。
--
-- libclang の AST では IfStmt の子は [cond, then, else?] の順で並ぶ。
-- then/else が CompoundStmt でなければ波括弧無しと判断する。
-- for/while/do-while も同様にループ本体が末尾の子になる。

local function last_non_empty_child_kind(children)
  for i = #children, 1, -1 do
    if children[i] and children[i].kind ~= "" then
      return children[i].kind
    end
  end
  return nil
end

cclint.register("require-braces", {
  description = "if/else/for/while の本体は必ず波括弧 {} で囲むこと",
  severity = "warning",

  match = function(n)
    return n.kind == "IfStmt" or n.kind == "ForStmt"
        or n.kind == "WhileStmt" or n.kind == "DoStmt"
        or n.kind == "CXXForRangeStmt"
  end,

  check = function(n)
    local kids = n.children or {}
    if n.kind == "IfStmt" then
      -- [cond, then, else?]
      local then_b = kids[2]
      local else_b = kids[3]
      if then_b and then_b.kind ~= "CompoundStmt" then
        cclint.report_warn(then_b, "if の本体は `{}` で囲んでください")
      end
      if else_b and else_b.kind ~= "CompoundStmt" and else_b.kind ~= "IfStmt" then
        cclint.report_warn(else_b, "else の本体は `{}` で囲んでください")
      end
    else
      local body_kind = last_non_empty_child_kind(kids)
      if body_kind and body_kind ~= "CompoundStmt" then
        cclint.report_warn(n, string.format(
          "%s の本体は `{}` で囲んでください", n.kind))
      end
    end
  end,
})
