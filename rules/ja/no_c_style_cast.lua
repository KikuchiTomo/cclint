-- Cスタイルキャストを避ける（C++キャストを使用すること）
rule_description = "Cスタイルキャストを避ける"
rule_category = "readability"

function check_ast()
    local file_info = cclint.get_file_info()
    if not file_info or not file_info.lines then return end

    for line_num, line_info in pairs(file_info.lines) do
        local line = line_info.text

        -- Cスタイルキャストを検出: (Type)expression
        -- 簡易的なヒューリスティック
        if line:match("%(([%w_:]+)%)%s*[%w_(]") then
            local cast_match = line:match("%(([%w_:]+)%)[%s]*")
            if cast_match and not line:match("if%s*%(") and not line:match("while%s*%(") and
               not line:match("for%s*%(") and not line:match("switch%s*%(") then
                cclint.report_info(
                    line_num, 1,
                    "Cスタイルキャストの代わりにC++キャスト (static_cast, const_cast, reinterpret_cast) の使用を検討してください"
                )
            end
        end
    end
end
