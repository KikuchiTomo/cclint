-- Test: Template API
rule_description = "Detect templates"
rule_category = "test"

function check_ast()
    local templates = cclint.get_templates()

    if not templates then
        print("[TEST] get_templates() returned nil")
        return
    end

    print("[TEST] Found " .. #templates .. " templates")

    for _, tmpl in ipairs(templates) do
        print("[TEST] Template at line " .. tmpl.line)
        cclint.report_info(
            tmpl.line, 1,
            "Template declaration detected"
        )
    end
end
