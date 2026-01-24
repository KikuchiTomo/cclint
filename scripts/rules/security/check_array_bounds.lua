-- 配列の境界チェック
--
-- 説明:
--   配列アクセスが境界内であることを確認すべきです。
--   固定サイズ配列への定数インデックスアクセスをチェックします。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度

local rule = {
    name = "check-array-bounds",
    description = "配列の境界チェック",
    severity = "error",
}

function rule:init(params)
    -- パラメータなし
end

function rule:visit_array_subscript_expr(subscript)
    local base = subscript:get_base()
    local index = subscript:get_index()

    -- インデックスが定数でない場合はスキップ
    if not index:is_integer_literal() then
        return
    end

    local index_value = index:get_value()

    -- 配列のサイズを取得
    local array_type = base:get_type()
    if not array_type:is_array_type() then
        return
    end

    local array_size = array_type:get_array_size()

    -- サイズが不明な場合はスキップ
    if array_size < 0 then
        return
    end

    -- 境界チェック
    if index_value < 0 then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "配列インデックスが負の値です（インデックス: %d）",
                index_value
            ),
            location = subscript:get_location(),
        })
    elseif index_value >= array_size then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "配列インデックスが境界外です（インデックス: %d、サイズ: %d）",
                index_value, array_size
            ),
            location = subscript:get_location(),
        })
    end
end

return rule
