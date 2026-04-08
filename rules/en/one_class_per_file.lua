-- Each file should contain only one class definition
rule_description = "Each file should contain only one class"
rule_category = "structure"

function check_ast()
    local classes = cclint.get_classes()
    if not classes then return end

    if #classes > 1 then
        cclint.report_warning(
            1, 1,
            string.format("File contains %d classes. Keep one class per file for better organization.", #classes)
        )
    end
end
