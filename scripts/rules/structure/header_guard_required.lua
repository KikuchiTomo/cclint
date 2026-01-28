-- Header files must have header guards (#pragma once or #ifndef/#define/#endif)
rule_description = "Header files must have header guards"
rule_category = "structure"

function check_file()
    -- Only check header files
    local ext = file_path:match("%.([^%.]+)$")
    if not (ext == "h" or ext == "hpp" or ext == "hxx" or ext == "hh") then
        return
    end

    -- Check for #pragma once
    for line_num, line in ipairs(file_lines) do
        if line:match("^%s*#%s*pragma%s+once") then
            return  -- OK
        end
    end

    -- Check for traditional header guard
    local has_ifndef = false
    local has_define = false
    local has_endif = false

    for line_num, line in ipairs(file_lines) do
        if line:match("^%s*#%s*ifndef%s+%w+") then
            has_ifndef = true
        elseif line:match("^%s*#%s*define%s+%w+") and has_ifndef then
            has_define = true
        elseif line:match("^%s*#%s*endif") then
            has_endif = true
        end
    end

    if not (has_ifndef and has_define and has_endif) then
        cclint.report_warning(
            1, 1,
            "Header file must have a header guard (#pragma once or #ifndef/#define/#endif)"
        )
    end
end
