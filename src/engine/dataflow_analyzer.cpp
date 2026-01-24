#include "dataflow_analyzer.hpp"

#include <algorithm>
#include <functional>

#include "utils/logger.hpp"

namespace cclint {
namespace engine {

DataFlowAnalysisResult DataFlowAnalyzer::analyze(std::shared_ptr<parser::TranslationUnitNode> ast) {
    DataFlowAnalysisResult result;

    if (!ast) {
        return result;
    }

    // 未初期化変数の検出
    result.uninitialized_variables = detect_uninitialized_variables(ast);

    // null参照リスクの検出
    result.null_pointer_risks = detect_null_pointer_risks(ast);

    return result;
}

std::set<std::string>
DataFlowAnalyzer::detect_uninitialized_variables(std::shared_ptr<parser::TranslationUnitNode> ast) {
    std::set<std::string> uninitialized;

    if (!ast) {
        return uninitialized;
    }

    // ASTを走査して関数を検索
    std::function<void(std::shared_ptr<parser::ASTNode>)> traverse;
    traverse = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Function ||
            node->type == parser::ASTNodeType::Method) {
            auto func = std::dynamic_pointer_cast<parser::FunctionNode>(node);
            if (func) {
                // 変数宣言を収集（初期化フラグ付き）
                std::map<std::string, bool> variables;  // 変数名 -> 初期化済みフラグ
                collect_variable_declarations(func, variables);

                // 未初期化変数を抽出
                for (const auto& [var_name, initialized] : variables) {
                    if (!initialized) {
                        uninitialized.insert(var_name);
                    }
                }
            }
        }

        for (const auto& child : node->children) {
            traverse(child);
        }
    };

    traverse(ast);
    return uninitialized;
}

std::map<std::string, std::vector<int>>
DataFlowAnalyzer::detect_null_pointer_risks(std::shared_ptr<parser::TranslationUnitNode> ast) {
    std::map<std::string, std::vector<int>> risks;

    if (!ast) {
        return risks;
    }

    // 簡易実装: ポインタ型の変数で初期化されていないものを検出
    // より高度な実装では、制御フローグラフを構築して解析する

    std::function<void(std::shared_ptr<parser::ASTNode>)> traverse;
    traverse = [&](std::shared_ptr<parser::ASTNode> node) {
        if (!node)
            return;

        if (node->type == parser::ASTNodeType::Variable ||
            node->type == parser::ASTNodeType::Field) {
            auto var = std::dynamic_pointer_cast<parser::VariableNode>(node);
            if (var) {
                // ポインタ型かどうかをチェック（簡易的に '*' を含むかで判定）
                if (var->type_name.find('*') != std::string::npos) {
                    // 初期化されていない可能性がある場合、リスクとして記録
                    risks[var->name].push_back(var->position.line);
                }
            }
        }

        for (const auto& child : node->children) {
            traverse(child);
        }
    };

    traverse(ast);
    return risks;
}

void DataFlowAnalyzer::collect_variable_declarations(std::shared_ptr<parser::ASTNode> node,
                                                     std::map<std::string, bool>& variables) {
    if (!node)
        return;

    if (node->type == parser::ASTNodeType::Variable) {
        auto var = std::dynamic_pointer_cast<parser::VariableNode>(node);
        if (var) {
            // 簡易実装: const または constexpr は初期化済みと仮定
            bool initialized = var->is_const || var->is_constexpr;
            variables[var->name] = initialized;
        }
    }

    for (const auto& child : node->children) {
        collect_variable_declarations(child, variables);
    }
}

void DataFlowAnalyzer::collect_variable_usages(std::shared_ptr<parser::ASTNode> node,
                                               std::set<std::string>& usages) {
    // この関数は将来の拡張のためのスタブ
    // より詳細な解析では、変数の使用箇所を追跡する
    (void)node;
    (void)usages;
}

std::vector<diagnostic::Diagnostic>
DataFlowAnalyzer::generate_diagnostics(const DataFlowAnalysisResult& result,
                                       const std::string& filename) {
    std::vector<diagnostic::Diagnostic> diagnostics;

    // 未初期化変数の診断
    for (const auto& var_name : result.uninitialized_variables) {
        diagnostic::Diagnostic diag;
        diag.severity = diagnostic::Severity::Warning;
        diag.rule_name = "dataflow-uninitialized-variable";
        diag.message = "Variable '" + var_name + "' may be used uninitialized";
        diag.location.filename = filename;
        diag.location.line = 0;  // 実際の位置はASTから取得すべき
        diag.location.column = 0;
        diagnostics.push_back(diag);
    }

    // null参照リスクの診断
    for (const auto& [var_name, lines] : result.null_pointer_risks) {
        for (int line : lines) {
            diagnostic::Diagnostic diag;
            diag.severity = diagnostic::Severity::Warning;
            diag.rule_name = "dataflow-null-pointer-risk";
            diag.message = "Pointer '" + var_name + "' may be null when dereferenced";
            diag.location.filename = filename;
            diag.location.line = line;
            diag.location.column = 0;
            diagnostics.push_back(diag);
        }
    }

    return diagnostics;
}

}  // namespace engine
}  // namespace cclint
