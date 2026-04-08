-- マジックナンバーを避ける（名前付き定数を使用すること）
rule_description = "マジックナンバーを避ける"
rule_category = "readability"

function check_ast()
    local file_info = cclint.get_file_info()
    if not file_info or not file_info.lines then return end

    for line_num, line_info in pairs(file_info.lines) do
        local line = line_info.text

        -- const宣言の行はスキップ
        if line:match("const%s+") or line:match("constexpr%s+") or line:match("#define") then
            goto continue
        end

        -- 数値を検出（一般的に使用される0, 1, 2は除外）
        for num in line:gmatch("[^%w_](%d+)[^%w_]") do
            local n = tonumber(num)
            if n and n > 2 then
                cclint.report_warning(
                    line_num, 1,
                    string.format("マジックナンバー %d が見つかりました。名前付き定数の使用を検討してください。", n)
                )
            end
        end

        ::continue::
    end
end
