-- 1ファイルにクラス定義は1つまで
rule_description = "1ファイルにクラスは1つまで"
rule_category = "structure"

function check_ast()
    local classes = cclint.get_classes()
    if not classes then return end

    if #classes > 1 then
        cclint.report_warning(
            1, 1,
            string.format("ファイルに%d個のクラスが含まれています。整理のため1ファイル1クラスにしてください。", #classes)
        )
    end
end
