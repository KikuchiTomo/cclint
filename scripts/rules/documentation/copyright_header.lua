-- 著作権ヘッダーが必要
local rule = {name = "copyright-header", description = "著作権ヘッダーが必要", severity = "warning"}
function rule:init(params) self.required_text = params.required_text or "Copyright" end
function rule:check_file(file)
    local source = file:get_source_text()
    local header = source:sub(1, 500)
    if not header:match(self.required_text) and not header:match("©") then
        self:report_diagnostic({severity = self.severity, message = "ファイルに著作権表示がありません", location = file:create_location(1, 1)})
    end
end
return rule
