-- ルール: ヘッダ (.h/.hpp) には #pragma once があること。
-- libclang の AST には pragma 情報が普通含まれないため、
-- このルールは「TranslationUnit 直下のファイルが .h/.hpp で、
--  かつ最初の子がプリプロセッサの InclusionDirective でも
--  PragmaOnce でもない」ことを大雑把にチェックする例。
--
-- 厳密には Lua 側でファイル本文を読んで `#pragma once` の有無を
-- 確認するのが確実なので、その方法で実装する。

cclint.register("header-pragma-once", {
  description = "ヘッダファイルに #pragma once が必要",
  severity = "warning",

  match = function(node)
    return node.kind == "TranslationUnit"
  end,

  check = function(node, ctx)
    local span = node.span
    if not span or not span.file then return end
    local file = span.file
    if not (file:match("%.h$") or file:match("%.hpp$")) then return end

    local f = io.open(file, "r")
    if not f then return end
    local content = f:read("*a")
    f:close()

    if not content:find("#pragma%s+once") then
      cclint.report_warn({ span = { file = file, line = 1, column = 1, byte_start = 0, byte_end = 0 } },
        string.format("ヘッダ `%s` に #pragma once がありません", file))
    end
  end,
})
