-- Avoid magic numbers (use named constants instead)
rule_description = "Avoid magic numbers"
rule_category = "readability"

function check_ast()
    local file_info = cclint.get_file_info()
    if not file_info or not file_info.lines then return end

    for line_num, line_info in pairs(file_info.lines) do
        local line = line_info.text

        -- Skip lines with const declarations
        if line:match("const%s+") or line:match("constexpr%s+") or line:match("#define") then
            goto continue
        end

        -- Find numbers (excluding 0, 1, 2 which are commonly used)
        for num in line:gmatch("[^%w_](%d+)[^%w_]") do
            local n = tonumber(num)
            if n and n > 2 then
                cclint.report_warning(
                    line_num, 1,
                    string.format("Magic number %d found. Consider using a named constant.", n)
                )
            end
        end

        ::continue::
    end
end
