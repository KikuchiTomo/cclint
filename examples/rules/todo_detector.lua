-- TODO Comment Detector Rule
-- Detects TODO, FIXME, HACK, XXX comments in source code

rule_description = "Detects TODO/FIXME/HACK/XXX comments"
rule_category = "documentation"

-- Keywords to detect
local keywords = {"TODO", "FIXME", "HACK", "XXX", "TEMP", "BUG"}

-- Check file for TODO comments
function check_file(file_path)
    -- Iterate through each line
    for line_num, line in ipairs(file_lines) do
        -- Check for each keyword
        for _, keyword in ipairs(keywords) do
            -- Pattern: // TODO: ...  or  /* TODO: ... */  or  # TODO: ...
            if cclint.match_pattern(line, "%s*//%s*" .. keyword) or
               cclint.match_pattern(line, "%s*/%*%s*" .. keyword) or
               cclint.match_pattern(line, "%s*#%s*" .. keyword) then

                -- Report as info (not warning)
                cclint.report_info(
                    line_num,
                    1,
                    keyword .. " comment found"
                )
            end
        end
    end
end
