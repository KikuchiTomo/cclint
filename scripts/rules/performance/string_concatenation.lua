-- 文字列連結はstringstreamを使用すべき
local rule = {name = "string-concatenation", description = "文字列連結はstringstream使用推奨", severity = "info"}
function rule:init(params) self.min_count = params.min_count or 3 end
function rule:visit_compound_stmt(stmt)
    local concat_count = 0
    for _, child in ipairs(stmt:get_children()) do
        if child:is_binary_operator() and child:get_operator() == "+=" then
            local lhs_type = child:get_lhs():get_type()
            if lhs_type:is_string() then
                concat_count = concat_count + 1
            end
        end
    end
    if concat_count >= self.min_count then
        self:report_diagnostic({severity = self.severity, message = string.format("%d回の文字列連結があります", concat_count), location = stmt:get_location(), notes = {{severity = "info", message = "std::stringstream または std::format の使用を検討してください"}}})
    end
end
return rule
