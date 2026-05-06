-- ルール: グローバルスコープでの素の new を禁止する例。
-- (簡易実装: NewExpr ノードを単に検出して警告)

cclint.register("forbid-global-new", {
  description = "素の new はスマートポインタの使用を推奨",
  severity = "warning",

  match = function(node)
    return node.kind == "NewExpr" or node.kind == "CXXNewExpr"
  end,

  check = function(node, ctx)
    cclint.report_warn(node,
      "`new` の直接使用は避け、std::make_unique / std::make_shared を検討してください")
  end,
})
