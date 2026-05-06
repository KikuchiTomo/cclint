-- ルール例: 関数内で `new` した順序と逆順に `delete` されているかをチェック。
-- 同一スコープにおける単純な LIFO 検査の雛形。

cclint.register("lifo-new-delete", {
  description = "同一スコープでの new/delete は LIFO 順序であること",
  severity = "warning",

  match = function(node)
    return node.kind == "FunctionDecl" or node.kind == "CXXMethodDecl"
  end,

  check = function(node, ctx)
    local stack = {}
    local function visit(n)
      if n.kind == "DeclStmt" then
        -- 簡易: VarDecl の初期化に NewExpr があれば push
        for _, c in ipairs(n.children) do
          if c.kind == "VarDecl" then
            for _, cc in ipairs(c.children or {}) do
              if cc.kind == "CXXNewExpr" or cc.kind == "NewExpr" then
                table.insert(stack, c.name)
              end
            end
          end
        end
      elseif n.kind == "CXXDeleteExpr" or n.kind == "DeleteExpr" then
        local last = stack[#stack]
        -- `delete x;` の対象名は children[1].name 付近にあることが多い
        local target = (n.children[1] and n.children[1].name) or ""
        if last and target ~= "" and last ~= target then
          cclint.report_warn(n, string.format(
            "delete 順序違反: 期待 `%s`, 実際 `%s` (LIFO 違反)", last, target))
        end
        table.remove(stack)
      end
      for _, c in ipairs(n.children or {}) do visit(c) end
    end
    visit(node)
  end,
})
