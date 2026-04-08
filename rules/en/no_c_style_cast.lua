-- Avoid C-style casts (use C++ casts instead)
rule_description = "Avoid C-style casts"
rule_category = "readability"

function check_ast()
    local file_info = cclint.get_file_info()
    if not file_info or not file_info.lines then return end

    for line_num, line_info in pairs(file_info.lines) do
        local line = line_info.text

        -- Detect C-style cast: (Type)expression
        -- This is a simple heuristic
        if line:match("%(([%w_:]+)%)%s*[%w_(]") then
            local cast_match = line:match("%(([%w_:]+)%)[%s]*")
            if cast_match and not line:match("if%s*%(") and not line:match("while%s*%(") and 
               not line:match("for%s*%(") and not line:match("switch%s*%(") then
                cclint.report_info(
                    line_num, 1,
                    "Consider using C++ casts (static_cast, const_cast, reinterpret_cast) instead of C-style casts"
                )
            end
        end
    end
end
