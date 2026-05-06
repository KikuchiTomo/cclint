-- ルール: 名前に `_internal_` を含むシンボルは，定義ファイル外から参照不可．
-- プロジェクトインデックス (cclint.project_definitions / project_references) を
-- 使ってクロス TU で判定する．

cclint.register("no-external-internal-ref", {
  description = "`_internal_` を名前に含むシンボルは定義ファイル外から参照禁止",
  severity = "error",

  -- TU ルートで一回だけ実行する．プロジェクト全体を見るので個別ノード相手の
  -- 走査は不要．
  match = function(n)
    return n.kind == "TranslationUnit" and n.span and n.span.file
  end,

  check = function(tu)
    local this_file = tu.span.file
    -- 定義群の中から，名前に _internal_ を含むものだけ抽出する．
    -- プロジェクトインデックスから直接 USR を引きたいが API がないので，
    -- 自分の TU の AST を walk して USR を集める．
    local function walk(n, out)
      if n.is_definition and n.usr ~= ""
         and n.name and n.name:find("_internal_") then
        out[n.usr] = { name = n.name, file = n.span and n.span.file or this_file }
      end
      for _, c in ipairs(n.children or {}) do walk(c, out) end
    end
    local internals = {}
    walk(tu, internals)

    for usr, info in pairs(internals) do
      local refs = cclint.project_references(usr)
      for _, r in ipairs(refs) do
        if r.span.file ~= info.file then
          cclint.report_error(r, string.format(
            "`%s` は定義ファイル `%s` の外 (`%s`) から参照されています",
            info.name, info.file, r.span.file))
        end
      end
    end
  end,
})
