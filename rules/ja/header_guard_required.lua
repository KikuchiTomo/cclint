-- ヘッダファイルにはヘッダガード（#pragma once または #ifndef/#define/#endif）が必要
rule_description = "ヘッダファイルにはヘッダガードが必要"
rule_category = "structure"

function check_ast()
    -- ヘッダファイルのみチェック
    local ext = file_path:match("%.([^%.]+)$")
    if not (ext == "h" or ext == "hpp" or ext == "hxx" or ext == "hh") then
        return
    end

    local file_info = cclint.get_file_info()
    if not file_info or not file_info.lines then return end

    -- #pragma once の確認
    for line_num, line_info in pairs(file_info.lines) do
        if line_info.text:match("^%s*#%s*pragma%s+once") then
            return  -- OK
        end
    end

    -- 従来のヘッダガードの確認
    local has_ifndef = false
    local has_define = false
    local has_endif = false

    for line_num, line_info in pairs(file_info.lines) do
        local line = line_info.text
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
            "ヘッダファイルにはヘッダガードが必要です（#pragma once または #ifndef/#define/#endif）"
        )
    end
end
