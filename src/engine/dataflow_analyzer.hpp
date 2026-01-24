#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>

#include "diagnostic/diagnostic.hpp"
#include "parser/ast.hpp"

namespace cclint {
namespace engine {

/// データフロー解析結果
struct DataFlowAnalysisResult {
    std::set<std::string> uninitialized_variables;  // 未初期化変数
    std::set<std::string> unused_variables;         // 未使用変数
    std::map<std::string, std::vector<int>>
        null_pointer_risks;  // null参照リスク（変数名 -> 行番号）
};

/// データフロー解析クラス
class DataFlowAnalyzer {
public:
    DataFlowAnalyzer() = default;

    /// ASTに対してデータフロー解析を実行
    /// @param ast 解析対象のAST
    /// @return 解析結果
    DataFlowAnalysisResult analyze(std::shared_ptr<parser::TranslationUnitNode> ast);

    /// 未初期化変数を検出
    /// @param ast 解析対象のAST
    /// @return 未初期化変数のセット
    std::set<std::string>
    detect_uninitialized_variables(std::shared_ptr<parser::TranslationUnitNode> ast);

    /// null参照リスクを検出
    /// @param ast 解析対象のAST
    /// @return null参照リスクのマップ（変数名 -> 行番号のリスト）
    std::map<std::string, std::vector<int>>
    detect_null_pointer_risks(std::shared_ptr<parser::TranslationUnitNode> ast);

    /// 診断メッセージを生成
    /// @param result 解析結果
    /// @param filename ファイル名
    /// @return 診断メッセージのリスト
    std::vector<diagnostic::Diagnostic> generate_diagnostics(const DataFlowAnalysisResult& result,
                                                             const std::string& filename);

private:
    /// 関数内の変数を解析
    void analyze_function(std::shared_ptr<parser::FunctionNode> func,
                          DataFlowAnalysisResult& result);

    /// 変数宣言を収集
    void collect_variable_declarations(std::shared_ptr<parser::ASTNode> node,
                                       std::map<std::string, bool>& variables);

    /// 変数使用を収集
    void collect_variable_usages(std::shared_ptr<parser::ASTNode> node,
                                 std::set<std::string>& usages);
};

}  // namespace engine
}  // namespace cclint
