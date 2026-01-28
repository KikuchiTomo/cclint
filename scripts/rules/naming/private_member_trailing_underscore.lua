-- Private member variables should have trailing underscore (e.g., data_, count_)
rule_description = "Private member variables should have trailing underscore"
rule_category = "naming"

function check_file()
    local in_class = false
    local in_private = false
    local brace_depth = 0

    for line_num, line in ipairs(file_lines) do
        -- Track class
        if line:match("^%s*class%s+") then
            in_class = true
            in_private = true  -- class defaults to private
            brace_depth = 0
        end

        -- Track access specifiers
        if in_class then
            if line:match("^%s*public%s*:") then
                in_private = false
            elseif line:match("^%s*private%s*:") then
                in_private = true
            elseif line:match("^%s*protected%s*:") then
                in_private = false
            end
        end

        -- Track braces
        for _ in line:gmatch("{") do
            brace_depth = brace_depth + 1
        end
        for _ in line:gmatch("}") do
            brace_depth = brace_depth - 1
            if in_class and brace_depth == 0 then
                in_class = false
                in_private = false
            end
        end

        -- Check private member variable declarations
        if in_class and in_private then
            -- Match: type variable_name;
            local var_name = line:match("^%s*[%w_:]+%s+([%w_]+)%s*[;=]")
            if var_name and not line:match("^%s*static") then
                -- Should end with underscore
                if not var_name:match("_$") then
                    cclint.report_warning(
                        line_num, 1,
                        string.format("Private member variable '%s' should have trailing underscore (e.g., %s_)", var_name, var_name)
                    )
                end
            end
        end
    end
end
