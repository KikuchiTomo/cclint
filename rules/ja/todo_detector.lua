-- TODOコメント検出ルール
-- ソースコード内のTODO, FIXME, HACK, XXXコメントを検出する

rule_description = "TODO/FIXME/HACK/XXXコメントを検出する"
rule_category = "documentation"

-- 検出するキーワード
local keywords = {"TODO", "FIXME", "HACK", "XXX", "TEMP", "BUG"}

-- ファイル内のTODOコメントを検査する
function check_file(file_path)
    -- 各行を走査
    for line_num, line in ipairs(file_lines) do
        -- 各キーワードをチェック
        for _, keyword in ipairs(keywords) do
            -- パターン: // TODO: ...  or  /* TODO: ... */  or  # TODO: ...
            if cclint.match_pattern(line, "%s*//%s*" .. keyword) or
               cclint.match_pattern(line, "%s*/%*%s*" .. keyword) or
               cclint.match_pattern(line, "%s*#%s*" .. keyword) then

                -- 情報として報告（警告ではない）
                cclint.report_info(
                    line_num,
                    1,
                    keyword .. " コメントが見つかりました"
                )
            end
        end
    end
end
