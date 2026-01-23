-- public関数にはドキュメントコメントが必要
--
-- 説明:
--   public関数やメソッドには、その機能を説明するドキュメントコメントが必要です。
--   Doxygen形式のコメント（/** ... */）を推奨します。
--
-- 設定可能なパラメータ:
--   - severity: エラーの重要度
--   - require_private: private関数にも必要とするか (デフォルト: false)
--   - min_lines: コメントを要求する最小関数行数 (デフォルト: 5)

local rule = {
    name = "require-function-comments",
    description = "public関数にはドキュメントコメントが必要",
    severity = "warning",
}

function rule:init(params)
    self.require_private = params.require_private or false
    self.min_lines = params.min_lines or 5
end

function rule:visit_function_decl(func_decl)
    -- 宣言のみの場合はスキップ
    if not func_decl:has_body() then
        return
    end

    -- private関数のチェック
    if not self.require_private and func_decl:get_access() == "private" then
        return
    end

    -- 小さい関数はスキップ
    local body = func_decl:get_body()
    local start_line = body:get_begin_location():get_line()
    local end_line = body:get_end_location():get_line()
    local lines = end_line - start_line + 1
    if lines < self.min_lines then
        return
    end

    -- コンストラクタ/デストラクタの簡単なチェック
    if func_decl:is_constructor() or func_decl:is_destructor() then
        if self:is_trivial_constructor_destructor(func_decl) then
            return
        end
    end

    -- ドキュメントコメントの存在をチェック
    local comment = func_decl:get_comment()
    if not comment or not self:is_documentation_comment(comment) then
        self:report_diagnostic({
            severity = self.severity,
            message = string.format(
                "関数 '%s' にドキュメントコメントがありません",
                func_decl:get_name()
            ),
            location = func_decl:get_location(),
            notes = {
                {
                    severity = "info",
                    message = "Doxygen形式のコメント（/** ... */）を追加してください",
                }
            }
        })
    end
end

-- ドキュメントコメントか判定
function rule:is_documentation_comment(comment)
    local text = comment:get_text()

    -- Doxygen形式のコメント
    if text:match("^/%*%*") or text:match("^///") then
        -- 空のコメントではないことを確認
        local content = text:gsub("^/%*%*+", ""):gsub("%*+/$", ""):gsub("^///", "")
        content = content:match("^%s*(.-)%s*$")
        return content ~= ""
    end

    return false
end

-- トリビアルなコンストラクタ/デストラクタか判定
function rule:is_trivial_constructor_destructor(func_decl)
    local body = func_decl:get_body()
    local statements = body:get_statements()

    -- 本体が空またはほぼ空
    return #statements == 0 or
           (#statements == 1 and statements[1]:is_null_statement())
end

return rule
