-- Prohibit 'using namespace' in global scope (header files especially)
rule_description = "Do not use 'using namespace' in global scope"
rule_category = "restrictions"

function check_file()
    local in_namespace = 0
    local in_class = 0
    local in_function = 0

    for line_num, line in ipairs(file_lines) do
        -- Track scope depth
        for _ in line:gmatch("namespace%s+[%w_]+%s*{") do
            in_namespace = in_namespace + 1
        end
        for _ in line:gmatch("class%s+[%w_]+") do
            in_class = in_class + 1
        end
        for _ in line:gmatch("^%s*[%w_:]+%s+[%w_]+%s*%(") do
            in_function = in_function + 1
        end

        -- Track closing braces (approximation)
        for _ in line:gmatch("}") do
            if in_function > 0 then
                in_function = in_function - 1
            elseif in_class > 0 then
                in_class = in_class - 1
            elseif in_namespace > 0 then
                in_namespace = in_namespace - 1
            end
        end

        -- Check for 'using namespace' in global scope
        if line:match("^%s*using%s+namespace") then
            if in_namespace == 0 and in_class == 0 and in_function == 0 then
                cclint.report_warning(
                    line_num, 1,
                    "Do not use 'using namespace' in global scope. It pollutes the global namespace."
                )
            end
        end
    end
end
