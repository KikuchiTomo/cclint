-- Header Guard Rule (Lua version, replaces C++ builtin)
rule_description = "Check for header guards in header files"
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
            return  -- #pragma once found, OK
        end
    end

    -- Check for traditional header guard (#ifndef, #define, #endif)
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
            "Header file should have a header guard (#ifndef/#define/#endif) or #pragma once"
        )
    end
end
