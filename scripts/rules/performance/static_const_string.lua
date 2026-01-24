-- 定数文字列はstatic constにすべき
--
-- 説明:
--   関数内で繰り返し使用される定数文字列は、
--   static constにすることでパフォーマンスが向上します。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "static-const-string",
    description = "定数文字列はstatic constにすべき",
    severity = "info",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_function_decl(func_decl)
    if not func_decl:has_body() then
        return
    end

    local body = func_decl:get_body()

    -- 関数内の文字列リテラルをカウント
    local string_literals = self:find_string_literals(body)

    for literal_value, locations in pairs(string_literals) do
        if #locations >= 2 and #literal_value > 10 then  -- 2回以上使用され、10文字以上
            self:report_diagnostic({
                severity = self.severity,
                message = string.format(
                    "文字列 \"%s...\" は%d回使用されています",
                    literal_value:sub(1, 20), #locations
                ),
                location = locations[1],
                notes = {
                    {
                        severity = "info",
                        message = "static const std::string で定義することを検討してください",
                    }
                }
            })
        end
    end
end

function rule:find_string_literals(stmt)
    local result = {}

    for _, child in ipairs(stmt:get_children()) do
        if child:is_string_literal() then
            local value = child:get_value()
            if not result[value] then
                result[value] = {}
            end
            table.insert(result[value], child:get_location())
        end

        -- 再帰的に検索
        local nested = self:find_string_literals(child)
        for value, locations in pairs(nested) do
            if not result[value] then
                result[value] = {}
            end
            for _, loc in ipairs(locations) do
                table.insert(result[value], loc)
            end
        end
    end

    return result
end

return rule
