-- ルール: 素の `enum` 禁止、`enum class`/`enum struct` を使う。
-- libclang のラッパに is_scoped() が露出していないので、
-- ソースの `enum` キーワード直後を読んで判定する。

cclint.register("forbid-plain-enum", {
  description = "素の enum は禁止 (enum class を使うこと)",
  severity = "warning",
  match = function(n) return n.kind == "EnumDecl" end,
  check = function(n)
    if not n.is_definition then return end
    if not n.name or n.name == "" then return end
    local span = n.span
    if not span or not span.file then return end

    local fp = io.open(span.file, "r")
    if not fp then return end
    local content = fp:read("*a"); fp:close()

    local s = math.max(1, (span.byte_start or 0) + 1)
    local e = math.min(#content, s + 64)
    local snippet = content:sub(s, e)
    -- "enum class" / "enum struct" が含まれていれば scoped enum (OK)
    if snippet:match("^%s*enum%s+class%s") or snippet:match("^%s*enum%s+struct%s") then
      return
    end
    cclint.report_warn(n, string.format(
      "素の enum `%s` は禁止: enum class を使ってください", n.name))
  end,
})
