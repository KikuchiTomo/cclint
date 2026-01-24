-- 変数は1行に1つ宣言すべき
local rule = {name = "variables-one-per-line", description = "変数は1行に1つ宣言すべき", severity = "info"}
function rule:init(params) end
function rule:check_file(file)
    local source = file:get_source_text()
    for line_num, line in ipairs(source:gmatch("([^\n]*)\n?")) do
        local comma_count = 0
        for _ in line:gmatch(",") do comma_count = comma_count + 1 end
        if comma_count > 1 and line:match("^%s*[%w_]+%s+[%w_]+%s*,") then
            self:report_diagnostic({severity = self.severity, message = "変数は1行に1つ宣言すべきです", location = file:create_location(line_num, 1)})
        end
    end
end
return rule
