-- メンバ変数には接頭辞を付けるべき
--
-- 説明:
--   メンバ変数には接頭辞（m_やm、_など）を付けることで、
--   ローカル変数と区別しやすくなります。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - prefix: 接頭辞 (デフォルト: "m_")
--   - apply_to_static: static変数にも適用 (デフォルト: false)

local rule = {
    name = "member-variable-prefix",
    description = "メンバ変数には接頭辞を付けるべき",
    severity = "warning",
}

function rule:init(params)
    self.prefix = params.prefix or "m_"
    self.apply_to_static = params.apply_to_static or false
end

function rule:visit_field_decl(field_decl)
    -- static変数のチェック
    if field_decl:is_static() and not self.apply_to_static then
        return
    end

    local name = field_decl:get_name()

    -- 接頭辞があるかチェック
    if not self:has_prefix(name) then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "メンバ変数 '%s' には接頭辞 '%s' を付けるべきです",
                name, self.prefix
            ),
            location = field_decl:get_location(),
            fix_hints = {
                {
                    range = field_decl:get_name_range(),
                    replacement = self.prefix .. name,
                }
            }
        })
    end
end

function rule:has_prefix(name)
    return name:sub(1, #self.prefix) == self.prefix
end

return rule
