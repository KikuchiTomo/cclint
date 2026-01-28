-- Use C++ style casts instead of C style casts
rule_description = "Use C++ style casts (static_cast, etc.)"
rule_category = "readability"

function check_file()
    for line_num, line in ipairs(file_lines) do
        -- Match C-style cast: (type)expression or (type*)expression
        if line:match("%(([%w_:]+%s*%*?)%)%s*[%w_]") then
            -- Exclude function calls and sizeof
            if not line:match("sizeof%s*%(") and
               not line:match("return%s*%(") and
               not line:match("if%s*%(") and
               not line:match("while%s*%(") then
                cclint.report_warning(
                    line_num, 1,
                    "Use C++ style casts (static_cast, dynamic_cast, const_cast, reinterpret_cast) instead of C-style casts"
                )
            end
        end
    end
end
