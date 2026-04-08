-- Cyclomatic Complexity Checker
-- Simplified version - counts decision points in functions

rule_description = "Checks cyclomatic complexity of functions"
rule_category = "readability"

-- Maximum allowed complexity (can be configured)
local max_complexity = 10
if rule_params and rule_params.max_complexity then
    max_complexity = tonumber(rule_params.max_complexity)
end

-- Count decision points in a function
local function count_complexity(function_body)
    local complexity = 1  -- Base complexity

    -- Keywords that increase complexity
    local keywords = {
        "if", "else", "elif", "for", "while",
        "case", "catch", "&&", "||", "?", ":"
    }

    for _, keyword in ipairs(keywords) do
        -- Count occurrences of each keyword
        local count = select(2, function_body:gsub("%f[%w]" .. keyword .. "%f[%W]", ""))
        complexity = complexity + count
    end

    return complexity
end

-- Check file for complex functions
function check_file(file_path)
    local content = file_content

    -- Simple pattern to detect functions
    -- This is a simplified version; real implementation would use AST
    local pattern = "(%w+)%s+(%w+)%s*%([^)]*%)%s*{([^}]+)}"

    local line_num = 1
    for return_type, func_name, func_body in content:gmatch(pattern) do
        local complexity = count_complexity(func_body)

        if complexity > max_complexity then
            -- Find approximate line number
            local _, line_count = content:sub(1, content:find(func_name)):gsub("\n", "")

            cclint.report_warning(
                line_count + 1,
                1,
                "Function '" .. func_name .. "' has cyclomatic complexity of " ..
                complexity .. " (max: " .. max_complexity .. ")"
            )
        end
    end
end
