-- 仮想関数のオーバーライドにはoverrideを使用すべき
--
-- 説明:
--   C++11以降、仮想関数をオーバーライドする際は明示的に
--   override指定子を使用すべきです。これにより誤ったオーバーライドを防げます。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - require_virtual_removal: virtual削除を要求 (デフォルト: true)

local rule = {
    name = "use-override",
    description = "仮想関数のオーバーライドにはoverrideを使用すべき",
    severity = "warning",
}

function rule:init(params)
    self.require_virtual_removal = params.require_virtual_removal ~= false
end

function rule:visit_method_decl(method_decl)
    -- 仮想関数でない、またはオーバーライドでない場合はスキップ
    if not method_decl:is_virtual() or not method_decl:overrides_parent() then
        return
    end

    -- デストラクタは除外（virtualが必要な場合がある）
    if method_decl:is_destructor() then
        return
    end

    -- overrideキーワードがあるかチェック
    local has_override = method_decl:has_override_specifier()
    local has_virtual = method_decl:has_virtual_specifier()

    if not has_override then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "オーバーライドメソッド '%s' にはoverride指定子を追加すべきです",
                method_decl:get_name()
            ),
            location = method_decl:get_location(),
            fix_hints = {
                {
                    range = method_decl:get_declaration_range(),
                    replacement = method_decl:get_declaration_text() .. " override",
                }
            }
        })
    elseif has_virtual and has_override and self.require_virtual_removal then
        -- override指定子があるときはvirtualは不要
        self:report_diagnostic({
            severity = "info",
            message = string.format(
                "メソッド '%s' はoverride指定子があるためvirtualは不要です",
                method_decl:get_name()
            ),
            location = method_decl:get_location(),
            notes = {
                {
                    severity = "info",
                    message = "virtualキーワードを削除してください",
                }
            }
        })
    end
end

return rule
