-- ルール: `init_secret` という名前の関数は main からのみ呼び出せる．
-- AST を walk して CallExpr を集め，referenced_name で呼び出し先を判定する．
-- caller は祖先方向に FunctionDecl を辿って取得する (parent 利用)．

local TARGET = "init_secret"

local function find_enclosing_function(node)
  local cur = node.parent
  while cur do
    if cur.kind == "FunctionDecl" or cur.kind == "Method"
       or cur.kind == "CXXMethodDecl" or cur.kind == "Constructor"
       or cur.kind == "Destructor" then
      return cur
    end
    cur = cur.parent
  end
  return nil
end

cclint.register("call-only-from-main", {
  description = string.format("`%s` は main 関数からのみ呼び出せる", TARGET),
  severity = "error",

  match = function(n) return n.kind == "CallExpr" end,

  check = function(n)
    if n.referenced_name ~= TARGET then return end
    local caller = find_enclosing_function(n)
    local caller_name = caller and caller.name or "<global>"
    if caller_name ~= "main" then
      cclint.report_error(n, string.format(
        "`%s` は main からのみ呼び出せます (呼出元: `%s`)", TARGET, caller_name))
    end
  end,
})
