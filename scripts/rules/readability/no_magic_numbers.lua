-- マジックナンバーを使用すべきでない
--
-- 説明:
--   コード内の数値リテラルは、意味のある名前付き定数にすべきです。
--   0, 1, -1などの一般的な値は除外されます。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - ignored_values: 無視する値のリスト (デフォルト: [0, 1, -1])
--   - allow_in_tests: テストコードでは許可 (デフォルト: true)

local rule = {
    name = "no-magic-numbers",
    description = "マジックナンバーを使用すべきでない",
    severity = "warning",
}

function rule:init(params)
    self.ignored_values = params.ignored_values or {0, 1, -1}
    self.allow_in_tests = params.allow_in_tests ~= false
end

function rule:visit_integer_literal(literal)
    local value = literal:get_value()

    -- 無視する値のチェック
    for _, ignored in ipairs(self.ignored_values) do
        if value == ignored then
            return
        end
    end

    -- テストコードのチェック
    if self.allow_in_tests and self:is_in_test_code(literal) then
        return
    end

    -- 列挙値の初期化など、特定のコンテキストでは許可
    if self:is_in_allowed_context(literal) then
        return
    end

    self:report_diagnostic({
        severity = self.severity,
        message = string.format(
            "マジックナンバー %d を使用しています",
            value
        ),
        location = literal:get_location(),
        notes = {
            {
                severity = "info",
                message = "意味のある名前付き定数を使用してください",
            }
        }
    })
end

function rule:visit_floating_literal(literal)
    local value = literal:get_value()

    -- 0.0などの一般的な値はスキップ
    if value == 0.0 or value == 1.0 or value == -1.0 then
        return
    end

    -- テストコードのチェック
    if self.allow_in_tests and self:is_in_test_code(literal) then
        return
    end

    self:report_diagnostic({
        severity = self.severity,
        message = string.format(
            "マジックナンバー %f を使用しています",
            value
        ),
        location = literal:get_location(),
        notes = {
            {
                severity = "info",
                message = "意味のある名前付き定数を使用してください",
            }
        }
    })
end

function rule:is_in_test_code(literal)
    -- テストファイルかチェック
    local file = literal:get_file()
    local filename = file:get_filename()

    return filename:match("_test%.") or
           filename:match("_unittest%.") or
           filename:match("/test/") or
           filename:match("/tests/")
end

function rule:is_in_allowed_context(literal)
    local parent = literal:get_parent()

    -- 配列のサイズ指定
    if parent:is_array_type() then
        return true
    end

    -- ビットシフト演算
    if parent:is_binary_operator() then
        local op = parent:get_operator()
        if op == "<<" or op == ">>" then
            return true
        end
    end

    return false
end

return rule
