#pragma once

#include <memory>

#include "parser/ast.hpp"
#include "semantic/symbol_table.hpp"
#include "semantic/type_system.hpp"

namespace cclint {
namespace semantic {

/// セマンティック解析器
/// ASTを走査してシンボルテーブルと型情報を構築する
class SemanticAnalyzer {
public:
    SemanticAnalyzer();

    /// ASTを解析
    /// @param ast 解析対象のAST
    void analyze(std::shared_ptr<parser::TranslationUnitNode> ast);

    /// シンボルテーブルを取得
    std::shared_ptr<SymbolTable> symbol_table() { return symbol_table_; }

    /// 型システムを取得
    std::shared_ptr<TypeSystem> type_system() { return type_system_; }

    /// エラーメッセージを取得
    const std::vector<std::string>& errors() const { return errors_; }

    /// エラーがあるか
    bool has_errors() const { return !errors_.empty(); }

private:
    std::shared_ptr<SymbolTable> symbol_table_;
    std::shared_ptr<TypeSystem> type_system_;
    std::vector<std::string> errors_;

    // AST走査
    void analyze_node(std::shared_ptr<parser::ASTNode> node);
    void analyze_namespace(std::shared_ptr<parser::NamespaceNode> node);
    void analyze_class(std::shared_ptr<parser::ClassNode> node);
    void analyze_function(std::shared_ptr<parser::FunctionNode> node);
    void analyze_variable(std::shared_ptr<parser::VariableNode> node);

    // ヘルパー
    void add_error(const std::string& message);
    std::shared_ptr<Type> resolve_type_from_string(const std::string& type_str);
};

}  // namespace semantic
}  // namespace cclint
