#include "semantic/semantic_analyzer.hpp"

namespace cclint {
namespace semantic {

SemanticAnalyzer::SemanticAnalyzer() {
    symbol_table_ = std::make_shared<SymbolTable>();
    type_system_ = std::make_shared<TypeSystem>();
}

void SemanticAnalyzer::analyze(std::shared_ptr<parser::TranslationUnitNode> ast) {
    if (!ast) {
        return;
    }

    // AST全体を走査
    for (const auto& child : ast->children) {
        analyze_node(child);
    }
}

void SemanticAnalyzer::analyze_node(std::shared_ptr<parser::ASTNode> node) {
    if (!node) {
        return;
    }

    switch (node->type) {
        case parser::ASTNodeType::Namespace: {
            auto ns = std::dynamic_pointer_cast<parser::NamespaceNode>(node);
            if (ns) analyze_namespace(ns);
            break;
        }
        case parser::ASTNodeType::Class: {
            auto cls = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (cls) analyze_class(cls);
            break;
        }
        case parser::ASTNodeType::Function:
        case parser::ASTNodeType::Method:
        case parser::ASTNodeType::Constructor:
        case parser::ASTNodeType::Destructor: {
            auto func = std::dynamic_pointer_cast<parser::FunctionNode>(node);
            if (func) analyze_function(func);
            break;
        }
        case parser::ASTNodeType::Variable:
        case parser::ASTNodeType::Field: {
            auto var = std::dynamic_pointer_cast<parser::VariableNode>(node);
            if (var) analyze_variable(var);
            break;
        }
        default:
            // その他のノードは子ノードを再帰的に解析
            for (const auto& child : node->children) {
                analyze_node(child);
            }
            break;
    }
}

void SemanticAnalyzer::analyze_namespace(std::shared_ptr<parser::NamespaceNode> node) {
    // 名前空間シンボルを追加
    auto symbol = std::make_shared<Symbol>(node->name, SymbolKind::Namespace);
    symbol->line = node->position.line;
    symbol->column = node->position.column;
    symbol_table_->add_symbol(node->name, symbol);

    // 名前空間のスコープに入る
    symbol_table_->enter_scope(node->name);

    // 子ノードを解析
    for (const auto& child : node->children) {
        analyze_node(child);
    }

    // スコープから出る
    symbol_table_->exit_scope();
}

void SemanticAnalyzer::analyze_class(std::shared_ptr<parser::ClassNode> node) {
    // クラスシンボルを追加
    auto symbol = std::make_shared<Symbol>(node->name, SymbolKind::Class);
    symbol->line = node->position.line;
    symbol->column = node->position.column;

    // 基底クラス情報を追加
    for (const auto& base : node->base_classes) {
        symbol->base_classes.push_back(base.base_class_name);
    }

    symbol_table_->add_symbol(node->name, symbol);

    // クラス型を作成
    auto class_type = type_system_->create_class_type(node->name);

    // クラスのスコープに入る
    symbol_table_->enter_scope(node->name);

    // メンバを解析
    for (const auto& child : node->children) {
        analyze_node(child);
    }

    // スコープから出る
    symbol_table_->exit_scope();
}

void SemanticAnalyzer::analyze_function(std::shared_ptr<parser::FunctionNode> node) {
    // 関数シンボルを追加
    auto symbol = std::make_shared<Symbol>(node->name, SymbolKind::Function);
    symbol->line = node->position.line;
    symbol->column = node->position.column;
    symbol->type = node->return_type;
    symbol->is_const = node->is_const;
    symbol->is_static = node->is_static;
    symbol->is_virtual = node->is_virtual;

    symbol_table_->add_symbol(node->name, symbol);

    // 関数のスコープに入る
    symbol_table_->enter_scope(node->name);

    // パラメータを解析（実装は省略 - ASTにパラメータ情報があれば追加）

    // 関数本体を解析
    for (const auto& child : node->children) {
        analyze_node(child);
    }

    // スコープから出る
    symbol_table_->exit_scope();
}

void SemanticAnalyzer::analyze_variable(std::shared_ptr<parser::VariableNode> node) {
    // 変数シンボルを追加
    auto symbol = std::make_shared<Symbol>(node->name, SymbolKind::Variable);
    symbol->line = node->position.line;
    symbol->column = node->position.column;
    symbol->type = node->type_name;
    symbol->is_const = node->is_const;
    symbol->is_static = node->is_static;

    symbol_table_->add_symbol(node->name, symbol);

    // 型を解決
    auto resolved_type = resolve_type_from_string(node->type_name);
    // 型情報をASTに付加（拡張ASTの場合）
    // node->resolved_type = resolved_type;
}

void SemanticAnalyzer::add_error(const std::string& message) {
    errors_.push_back(message);
}

std::shared_ptr<Type> SemanticAnalyzer::resolve_type_from_string(const std::string& type_str) {
    if (type_str.empty()) {
        return nullptr;
    }

    return type_system_->resolve_type(type_str);
}

}  // namespace semantic
}  // namespace cclint
