-- Prohibit 'delete' keyword in class methods (use smart pointers instead)
rule_description = "Do not use 'delete' in class methods"
rule_category = "restrictions"

function check_file()
    local in_class = false
    local class_name = nil
    local brace_depth = 0

    for line_num, line in ipairs(file_lines) do
        -- Track class definition
        local found_class = line:match("^%s*class%s+([%w_]+)")
        if found_class then
            in_class = true
            class_name = found_class
            brace_depth = 0
        end

        -- Track braces
        for _ in line:gmatch("{") do
            brace_depth = brace_depth + 1
        end
        for _ in line:gmatch("}") do
            brace_depth = brace_depth - 1
            if in_class and brace_depth == 0 then
                in_class = false
                class_name = nil
            end
        end

        -- Check for 'delete' keyword
        if in_class and line:match("[^%w_]delete%s+[%w_%[]") then
            cclint.report_warning(
                line_num, 1,
                string.format("Do not use 'delete' in class '%s'. Use smart pointers with automatic cleanup.", class_name or "unknown")
            )
        end
    end
end
