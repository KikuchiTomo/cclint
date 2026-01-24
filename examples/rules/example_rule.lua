-- Example Lua Rule
-- This is a simple example showing how to write a custom Lua rule

rule_description = "Example rule demonstrating basic Lua rule structure"
rule_category = "example"

-- This function is called for each file
-- @param file_path: Path to the file being checked
function check_file(file_path)
    -- Access file content
    local content = file_content

    -- Access individual lines
    local lines = file_lines

    -- Example 1: Check for long lines
    for line_num, line in ipairs(lines) do
        if #line > 120 then
            cclint.report_warning(
                line_num,
                1,
                "Line is too long: " .. #line .. " characters"
            )
        end
    end

    -- Example 2: Check for specific patterns
    for line_num, line in ipairs(lines) do
        local matched, groups = cclint.match_pattern(line, "printf%(")
        if matched then
            cclint.report_info(
                line_num,
                1,
                "Consider using std::cout instead of printf"
            )
        end
    end

    -- Example 3: Check for missing includes
    local has_iostream = false
    local uses_cout = false

    for line_num, line in ipairs(lines) do
        if cclint.match_pattern(line, "#include%s+<iostream>") then
            has_iostream = true
        end
        if cclint.match_pattern(line, "std::cout") then
            uses_cout = true
        end
    end

    if uses_cout and not has_iostream then
        cclint.report_error(
            1,
            1,
            "Using std::cout but <iostream> is not included"
        )
    end

    -- Example 4: Access rule parameters
    if rule_params and rule_params.strict_mode == "true" then
        -- Additional strict checks
        for line_num, line in ipairs(lines) do
            if cclint.match_pattern(line, "goto") then
                cclint.report_error(
                    line_num,
                    1,
                    "Use of 'goto' is not allowed in strict mode"
                )
            end
        end
    end
end

-- Optional: Metadata that can be accessed by cclint
-- These global variables are read during rule loading
print("Example rule loaded successfully")
