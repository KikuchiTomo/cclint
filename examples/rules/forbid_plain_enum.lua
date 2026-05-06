-- ルール: 素の `enum` 禁止、`enum class` を使う
cclint.register("forbid-plain-enum", {
  description = "素の enum は禁止 (enum class を使うこと)",
  severity = "warning",
  match = function(n) return n.kind == "EnumDecl" end,
  check = function(n)
    if not n.is_definition then return end
    -- type_name が "enum X" だけだと素の enum、"enum class X" なら scoped
    local ty = n.type_name or ""
    -- 確実な手段が無いので display_name に依存。`enum class Foo` は display_name が "Foo"
    -- 素の enum は children[0] が integral 型のみで scoped 情報が出ない。
    -- ここでは型表記が "enum X" でクラスでないものを警告する簡易判定。
    -- libclang には is_scoped() があるが Rust ラッパで未公開な場合があるため、
    -- ヒューリスティックとして「name が空 or kind が EnumDecl で type が "enum X" 形式」を警告。
    if n.name and n.name ~= "" and not (ty:match("^enum class")) then
      cclint.report_warn(n, string.format(
        "素の enum `%s` は禁止: enum class を使ってください", n.name))
    end
  end,
})
