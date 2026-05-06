-- ルール: 単一引数のコンストラクタは explicit 必須 (暗黙変換防止)
-- libclang は is_explicit_method がラッパに無い場合があるので、display_name を見る。
cclint.register("explicit-single-arg-ctor", {
  description = "単一引数のコンストラクタは explicit 必須",
  severity = "warning",
  match = function(n) return n.kind == "Constructor" end,
  check = function(n)
    -- 引数数を数える: 子ノードのうち ParmDecl の数
    local params = 0
    local has_body = false
    for _, c in ipairs(n.children or {}) do
      if c.kind == "ParmDecl" then params = params + 1 end
      if c.kind == "CompoundStmt" then has_body = true end
    end
    if params ~= 1 then return end
    -- クラス外定義 (Foo::Foo(int x) {}) は対象外: explicit はクラス内宣言にのみ書ける。
    if has_body then return end

    -- explicit かどうかをソース文字列から判定 (簡易)．
    -- ctor の cursor の range には explicit が含まれる場合・含まれない場合があるため，
    -- byte_start の前後 32 byte を併せて見る．
    local span = n.span
    if not span or not span.file then return end
    local f = io.open(span.file, "r")
    if not f then return end
    local content = f:read("*a"); f:close()
    local s = math.max(1, (span.byte_start or 0) - 32 + 1)
    local e = math.min(#content, (span.byte_start or 0) + 32)
    local snippet = content:sub(s, e)
    if not snippet:match("explicit%s") then
      cclint.report_warn(n, string.format(
        "単一引数のコンストラクタ `%s` は explicit を付けてください", n.name or "?"))
    end
  end,
})
