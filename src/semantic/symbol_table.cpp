#include "semantic/symbol_table.hpp"

#include <sstream>

namespace cclint {
namespace semantic {

// ==================== Scope Implementation ====================

void Scope::add_symbol(const std::string& name, std::shared_ptr<Symbol> symbol) {
    symbols_[name] = symbol;
}

std::shared_ptr<Symbol> Scope::lookup_local(const std::string& name) const {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Symbol> Scope::lookup(const std::string& name) const {
    // まず現在のスコープで探す
    auto symbol = lookup_local(name);
    if (symbol) {
        return symbol;
    }

    // 親スコープで探す
    if (parent_) {
        return parent_->lookup(name);
    }

    return nullptr;
}

std::shared_ptr<Scope> Scope::create_child(const std::string& name) {
    auto child = std::make_shared<Scope>(this, name);
    children_.push_back(child);
    return child;
}

// ==================== SymbolTable Implementation ====================

SymbolTable::SymbolTable() {
    global_scope_ = std::make_shared<Scope>(nullptr, "global");
    current_scope_ = global_scope_;
    scope_stack_.push_back(global_scope_);
}

void SymbolTable::enter_scope(const std::string& name) {
    current_scope_ = current_scope_->create_child(name);
    scope_stack_.push_back(current_scope_);
}

void SymbolTable::exit_scope() {
    if (scope_stack_.size() > 1) {
        scope_stack_.pop_back();
        current_scope_ = scope_stack_.back();
    }
}

void SymbolTable::add_symbol(const std::string& name, std::shared_ptr<Symbol> symbol) {
    current_scope_->add_symbol(name, symbol);
}

std::shared_ptr<Symbol> SymbolTable::lookup(const std::string& name) const {
    return current_scope_->lookup(name);
}

std::shared_ptr<Symbol> SymbolTable::lookup_qualified(const std::string& qualified_name) const {
    // 修飾名を分割 (例: "std::vector" -> ["std", "vector"])
    std::vector<std::string> parts;
    std::stringstream ss(qualified_name);
    std::string part;

    while (std::getline(ss, part, ':')) {
        if (!part.empty() && part != ":") {
            parts.push_back(part);
        }
    }

    if (parts.empty()) {
        return nullptr;
    }

    // 最初の部分をグローバルスコープから探す
    auto symbol = global_scope_->lookup_local(parts[0]);
    if (!symbol) {
        return nullptr;
    }

    // 残りの部分を順に探す (完全実装)
    for (size_t i = 1; i < parts.size(); ++i) {
        // クラスや名前空間のメンバーを探す
        if (symbol->kind == SymbolKind::Class || symbol->kind == SymbolKind::Namespace) {
            // メンバーを検索
            symbol = symbol->lookup_member(parts[i]);
            if (!symbol) {
                return nullptr;  // メンバーが見つからない
            }
        } else {
            // クラスでも名前空間でもない場合、これ以上辿れない
            return nullptr;
        }
    }

    return symbol;
}

}  // namespace semantic
}  // namespace cclint
