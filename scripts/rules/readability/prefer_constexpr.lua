-- Prefer constexpr over const for compile-time constants
rule_description = "Prefer constexpr over const for compile-time constants"
rule_category = "readability"

function check_file()
    for line_num, line in ipairs(file_lines) do
        -- Match: const type var = literal_value;
        if line:match("const%s+[%w_:]+%s+[%w_]+%s*=%s*[%d\"']") then
            -- Don't suggest for pointers or references
            if not line:match("%*") and not line:match("&") then
                cclint.report_info(
                    line_num, 1,
                    "Consider using 'constexpr' instead of 'const' for compile-time constants"
                )
            end
        end
    end
end
