-- 名前空間の閉じ括弧にコメントを付けるべき
local rule = {name = "namespace-closing-comment", description = "名前空間の閉じ括弧にコメントを付けるべき", severity = "info"}
function rule:init(params) self.min_lines = params.min_lines or 20 end
function rule:visit_namespace_decl(ns_decl)
    local name = ns_decl:get_name()
    if name == "" then return end
    local lines = ns_decl:get_end_location():get_line() - ns_decl:get_begin_location():get_line()
    if lines >= self.min_lines then
        -- 閉じ括弧付近にコメントがあるかチェック
    end
end
return rule
