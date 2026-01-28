-- Switch statements must have a default case
rule_description = "Switch statements must have a default case"
rule_category = "readability"

function check_file()
    local in_switch = false
    local switch_line = 0
    local has_default = false
    local brace_depth = 0

    for line_num, line in ipairs(file_lines) do
        -- Start of switch
        if line:match("switch%s*%(") then
            in_switch = true
            switch_line = line_num
            has_default = false
            brace_depth = 0
        end

        if in_switch then
            -- Count braces
            for _ in line:gmatch("{") do
                brace_depth = brace_depth + 1
            end
            for _ in line:gmatch("}") do
                brace_depth = brace_depth - 1
                if brace_depth == 0 then
                    -- End of switch
                    if not has_default then
                        cclint.report_warning(
                            switch_line, 1,
                            "Switch statement should have a default case"
                        )
                    end
                    in_switch = false
                end
            end

            -- Check for default
            if line:match("^%s*default%s*:") then
                has_default = true
            end
        end
    end
end
