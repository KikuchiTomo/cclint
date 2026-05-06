-- ルール: 単一引数のコンストラクタは explicit 必須 (暗黙変換防止)
-- libclang は is_explicit_method がラッパに無い場合があるので、display_name を見る。
cclint.register("explicit-single-arg-ctor", {
  description = "単一引数のコンストラクタは explicit 必須",
  severity = "warning",
  match = function(n) return n.kind == "Constructor" end,
  check = function(n)
    -- 引数数を数える: 子ノードのうち ParmDecl の数
    local params = 0
    for _, c in ipairs(n.children or {}) do
      if c.kind == "ParmDecl" then params = params + 1 end
    end
    if params ~= 1 then return end

    -- explicit かどうかをソース文字列から判定 (簡易)
    local span = n.span
    if not span or not span.file then return end
    local f = io.open(span.file, "r")
    if not f then return end
    local content = f:read("*a"); f:close()
    -- byte_start 周辺の前 64 byte を見て explicit があるか
    local start = math.max(1, (span.byte_start or 0) - 64)
    local snippet = content:sub(start, (span.byte_start or 0) + 1)
    if not snippet:match("explicit%s") then
      cclint.report_warn(n, string.format(
        "単一引数のコンストラクタ `%s` は explicit を付けてください", n.name or "?"))
    end
  end,
})
