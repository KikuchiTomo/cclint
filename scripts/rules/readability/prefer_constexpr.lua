-- Prefer constexpr over const for compile-time constants
rule_description = "Prefer constexpr over const for compile-time constants"
rule_category = "readability"

function check_ast()
    local file_info = cclint.get_file_info()
    if not file_info or not file_info.lines then return end

    for line_num, line_info in pairs(file_info.lines) do
        local line = line_info.text

        -- Check for const initialization with literal
        if line:match("const%s+") and not line:match("constexpr%s+") then
            -- Check if initialized with a literal value
            if line:match("=%s*%d+") or line:match("=%s*\"") or line:match("=%s*'") or 
               line:match("=%s*true") or line:match("=%s*false") then
                cclint.report_info(
                    line_num, 1,
                    "Consider using constexpr instead of const for compile-time constants"
                )
            end
        end
    end
end
