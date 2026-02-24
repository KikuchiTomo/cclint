#include "semantic/semantic_analyzer.hpp"

#include "semantic/constexpr_evaluator.hpp"

namespace cclint {
namespace semantic {

SemanticAnalyzer::SemanticAnalyzer() {
    symbol_table_ = std::make_shared<SymbolTable>();
    type_system_ = std::make_shared<TypeSystem>();
    constexpr_evaluator_ = std::make_shared<ConstexprEvaluator>();
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
            if (ns)
                analyze_namespace(ns);
            break;
        }
        case parser::ASTNodeType::Class: {
            auto cls = std::dynamic_pointer_cast<parser::ClassNode>(node);
            if (cls)
                analyze_class(cls);
            break;
        }
        case parser::ASTNodeType::Function:
        case parser::ASTNodeType::Method:
        case parser::ASTNodeType::Constructor:
        case parser::ASTNodeType::Destructor: {
            auto func = std::dynamic_pointer_cast<parser::FunctionNode>(node);
            if (func)
                analyze_function(func);
            break;
        }
        case parser::ASTNodeType::Variable:
        case parser::ASTNodeType::Field: {
            auto var = std::dynamic_pointer_cast<parser::VariableNode>(node);
            if (var)
                analyze_variable(var);
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

    // 現在のスコープのシンボルをメンバーとして登録
    auto current_scope = symbol_table_->current_scope();
    for (const auto& [name, member_symbol] : current_scope->symbols()) {
        symbol->add_member(name, member_symbol);
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

    // ASTノードにセマンティック情報を設定
    node->resolved_type_name = node->name;
    node->defining_scope = symbol_table_->current_scope()->name();
    node->is_type_checked = true;
    node->is_semantically_valid = true;

    // 基底クラスの型を解決
    for (const auto& base : node->base_classes) {
        auto base_type = type_system_->resolve_type(base.base_class_name);
        if (base_type && base_type->kind() == TypeKind::Class) {
            auto base_class_type = std::dynamic_pointer_cast<ClassType>(base_type);
            if (base_class_type) {
                class_type->add_base(base_class_type);
            }
        } else {
            // 基底クラスが見つからない
            add_error("Class '" + node->name + "': Base class '" + base.base_class_name +
                      "' not found");
        }
    }

    // クラスのスコープに入る
    symbol_table_->enter_scope(node->name);

    // メンバを解析
    for (const auto& child : node->children) {
        analyze_node(child);
    }

    // 現在のスコープのシンボルをメンバーとして登録
    auto current_scope = symbol_table_->current_scope();
    for (const auto& [name, member_symbol] : current_scope->symbols()) {
        symbol->add_member(name, member_symbol);
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

    // ASTノードにセマンティック情報を設定
    node->resolved_type_name = node->return_type;
    node->defining_scope = symbol_table_->current_scope()->name();
    node->is_const_type = node->is_const;
    node->is_type_checked = true;

    // 戻り値型を解決
    auto return_type = type_system_->resolve_type(node->return_type);
    if (return_type) {
        node->is_const_type = return_type->is_const();
    }

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

    // ASTノードにセマンティック情報を設定
    node->resolved_type_name = node->type_name;
    node->defining_scope = symbol_table_->current_scope()->name();
    node->is_const_type = node->is_const;
    node->is_type_checked = true;

    // 型を解決
    auto resolved_type = resolve_type_from_string(node->type_name);
    if (resolved_type) {
        // 型の詳細情報をASTに反映
        node->is_const_type = resolved_type->is_const();
        node->is_semantically_valid = true;

        // constexpr変数の場合、定数値を評価
        if (node->is_constexpr && constexpr_evaluator_) {
            // 変数初期化式を評価（基本実装）
            // TODO: より高度な式評価のためにASTノードベースの評価に拡張
            // 現在は簡単なリテラルのみ対応
        }
    } else {
        // 型解決失敗
        node->is_semantically_valid = false;
        node->semantic_error = "Failed to resolve type: " + node->type_name;
        add_error("Variable '" + node->name + "': " + node->semantic_error);
    }
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
