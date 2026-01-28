-- Prohibit throwing exceptions in destructors (causes std::terminate)
rule_description = "Do not throw exceptions in destructors"
rule_category = "restrictions"

function check_file()
    local in_destructor = false
    local destructor_name = nil
    local brace_depth = 0

    for line_num, line in ipairs(file_lines) do
        -- Match destructor definition: ~ClassName()
        local dtor = line:match("~([%w_]+)%s*%(")
        if dtor then
            in_destructor = true
            destructor_name = dtor
            brace_depth = 0
        end

        -- Track braces
        for _ in line:gmatch("{") do
            if in_destructor then
                brace_depth = brace_depth + 1
            end
        end
        for _ in line:gmatch("}") do
            if in_destructor then
                brace_depth = brace_depth - 1
                if brace_depth == 0 then
                    in_destructor = false
                    destructor_name = nil
                end
            end
        end

        -- Check for throw statement
        if in_destructor and line:match("[^%w_]throw%s+") then
            cclint.report_error(
                line_num, 1,
                string.format("Do not throw exceptions in destructor '~%s'. This will call std::terminate.", destructor_name or "unknown")
            )
        end
    end
end
