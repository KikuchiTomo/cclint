-- Rule: One class per file (excluding nested classes)
-- Check that each .cpp file should contain exactly one top-level class definition

rule_description = "Each .cpp file should contain exactly one top-level class definition"
rule_category = "structure"

function check_ast()
    -- Get all classes in the file with detailed info
    local classes = cclint.get_classes_with_info()

    if not classes then
        return
    end

    local class_count = #classes

    if class_count == 0 then
        cclint.report_warning(
            1, 1,
            "No class definition found in .cpp file"
        )
    elseif class_count > 1 then
        -- Report error for each additional class
        for i = 2, class_count do
            local class_info = classes[i]
            local line = class_info.line or 1

            cclint.report_error(
                line, 1,
                string.format(
                    "Multiple classes in one file (found %d classes). Each .cpp file should contain exactly one class.",
                    class_count
                )
            )
        end
    end
end
