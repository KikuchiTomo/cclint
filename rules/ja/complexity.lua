-- 循環的複雑度チェッカー
-- 簡易版 - 関数内の分岐点を計測する

rule_description = "関数の循環的複雑度を検査する"
rule_category = "readability"

-- 許容される最大複雑度（設定可能）
local max_complexity = 10
if rule_params and rule_params.max_complexity then
    max_complexity = tonumber(rule_params.max_complexity)
end

-- 関数内の分岐点を計測する
local function count_complexity(function_body)
    local complexity = 1  -- 基本複雑度

    -- 複雑度を増加させるキーワード
    local keywords = {
        "if", "else", "elif", "for", "while",
        "case", "catch", "&&", "||", "?", ":"
    }

    for _, keyword in ipairs(keywords) do
        -- 各キーワードの出現回数を計測
        local count = select(2, function_body:gsub("%f[%w]" .. keyword .. "%f[%W]", ""))
        complexity = complexity + count
    end

    return complexity
end

-- ファイル内の複雑な関数を検査する
function check_file(file_path)
    local content = file_content

    -- 関数を検出する簡易パターン
    -- これは簡易版であり、実際の実装ではASTを使用する
    local pattern = "(%w+)%s+(%w+)%s*%([^)]*%)%s*{([^}]+)}"

    local line_num = 1
    for return_type, func_name, func_body in content:gmatch(pattern) do
        local complexity = count_complexity(func_body)

        if complexity > max_complexity then
            -- おおよその行番号を取得
            local _, line_count = content:sub(1, content:find(func_name)):gsub("\n", "")

            cclint.report_warning(
                line_count + 1,
                1,
                "関数 '" .. func_name .. "' の循環的複雑度は " ..
                complexity .. " です (上限: " .. max_complexity .. ")"
            )
        end
    end
end
