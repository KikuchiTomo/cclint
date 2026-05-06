-- ルール: `_internal.h` で終わるヘッダは同名の `_internal.cpp` からしか include できない．
-- 例: `widget_internal.h` は `widget_internal.cpp` からのみ可．
--
-- AST 上の InclusionDirective ノードを使う．`included_file` に解決後のパスが入る．

cclint.register("include-restriction", {
  description = "_internal.h は対応する _internal.cpp からのみ include 可",
  severity = "error",

  match = function(n) return n.kind == "InclusionDirective" end,

  check = function(n)
    local incl = n.included_file
    if not incl then return end
    local incl_base = incl:match("([^/]+)%.h$")
    if not incl_base or not incl_base:match("_internal$") then return end

    local from = n.span and n.span.file or ""
    local from_base = from:match("([^/]+)%.cpp$") or from:match("([^/]+)%.cc$")
    if from_base ~= incl_base then
      cclint.report_error(n, string.format(
        "`%s` は `%s.cpp` からのみ include 可能です (現在の include 元: `%s`)",
        incl, incl_base, from))
    end
  end,
})
