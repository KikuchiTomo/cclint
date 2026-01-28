-- Boolean variables should have descriptive names (is_, has_, can_, should_, etc.)
rule_description = "Boolean variables should have descriptive prefixes"
rule_category = "naming"

function check_file()
    for line_num, line in ipairs(file_lines) do
        -- Match: bool variable_name
        local bool_var = line:match("bool%s+([%w_]+)")
        if bool_var then
            -- Check if it has a good prefix
            local good_prefixes = {"is_", "has_", "can_", "should_", "will_", "was_", "did_", "needs_", "enable", "disable", "allow"}
            local has_good_prefix = false

            for _, prefix in ipairs(good_prefixes) do
                if bool_var:match("^" .. prefix) then
                    has_good_prefix = true
                    break
                end
            end

            if not has_good_prefix then
                cclint.report_info(
                    line_num, 1,
                    string.format("Boolean variable '%s' should have descriptive prefix (is_, has_, can_, should_, etc.)", bool_var)
                )
            end
        end
    end
end
