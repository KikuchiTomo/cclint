-- Prohibit malloc/calloc/realloc in class methods (use C++ containers instead)
rule_description = "Do not use malloc/calloc/realloc in class methods"
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

        -- Check for malloc/calloc/realloc
        if in_class then
            if line:match("[^%w_]malloc%s*%(") or
               line:match("[^%w_]calloc%s*%(") or
               line:match("[^%w_]realloc%s*%(") then
                cclint.report_warning(
                    line_num, 1,
                    string.format("Do not use malloc/calloc/realloc in class '%s'. Use std::vector or std::array instead.", class_name or "unknown")
                )
            end
        end
    end
end
