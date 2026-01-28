-- Prohibit 'using namespace' in global scope (header files especially)
rule_description = "Do not use 'using namespace' in global scope"
rule_category = "restrictions"

function check_ast()
    local usings = cclint.get_usings()
    if not usings then return end

    for _, using_decl in ipairs(usings) do
        -- グローバルスコープのusing namespaceを検出
        if using_decl.target and using_decl.target:match("namespace") then
            cclint.report_warning(
                using_decl.line, 1,
                "Do not use 'using namespace' in global scope. It pollutes the global namespace."
            )
        end
    end
end
