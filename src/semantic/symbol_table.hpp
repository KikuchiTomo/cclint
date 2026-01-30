#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cclint {
namespace semantic {

/// シンボルの種類
enum class SymbolKind {
    Variable,
    Function,
    Class,
    Namespace,
    Typedef,
    Enum,
    EnumConstant,
    Template,
    Parameter
};

/// シンボル情報
struct Symbol {
    std::string name;
    SymbolKind kind;
    std::string type;  // 型名（変数・関数の場合）
    int line = 0;
    int column = 0;
    std::shared_ptr<Symbol> parent = nullptr;  // 親スコープのシンボル

    // 関数の場合
    std::vector<std::string> parameters;
    bool is_const = false;
    bool is_static = false;
    bool is_virtual = false;

    // クラスの場合
    std::vector<std::string> base_classes;

    Symbol(const std::string& n, SymbolKind k) : name(n), kind(k) {}
};

/// スコープ (名前空間、クラス、関数などのスコープ)
class Scope {
public:
    explicit Scope(Scope* parent = nullptr, const std::string& name = "")
        : parent_(parent), name_(name) {}

    /// シンボルを追加
    void add_symbol(const std::string& name, std::shared_ptr<Symbol> symbol);

    /// シンボルを検索 (このスコープのみ)
    std::shared_ptr<Symbol> lookup_local(const std::string& name) const;

    /// シンボルを検索 (親スコープも含む)
    std::shared_ptr<Symbol> lookup(const std::string& name) const;

    /// 子スコープを作成
    std::shared_ptr<Scope> create_child(const std::string& name = "");

    /// 親スコープを取得
    Scope* parent() const { return parent_; }

    /// スコープ名を取得
    const std::string& name() const { return name_; }

    /// このスコープ内の全シンボルを取得
    const std::unordered_map<std::string, std::shared_ptr<Symbol>>& symbols() const {
        return symbols_;
    }

private:
    Scope* parent_;
    std::string name_;
    std::unordered_map<std::string, std::shared_ptr<Symbol>> symbols_;
    std::vector<std::shared_ptr<Scope>> children_;
};

/// シンボルテーブル (グローバルスコープ管理)
class SymbolTable {
public:
    SymbolTable();

    /// グローバルスコープを取得
    std::shared_ptr<Scope> global_scope() { return global_scope_; }

    /// 現在のスコープを取得
    std::shared_ptr<Scope> current_scope() { return current_scope_; }

    /// 新しいスコープに入る
    void enter_scope(const std::string& name = "");

    /// 現在のスコープから出る
    void exit_scope();

    /// シンボルを現在のスコープに追加
    void add_symbol(const std::string& name, std::shared_ptr<Symbol> symbol);

    /// シンボルを検索
    std::shared_ptr<Symbol> lookup(const std::string& name) const;

    /// 修飾名でシンボルを検索 (例: std::vector)
    std::shared_ptr<Symbol> lookup_qualified(const std::string& qualified_name) const;

private:
    std::shared_ptr<Scope> global_scope_;
    std::shared_ptr<Scope> current_scope_;
    std::vector<std::shared_ptr<Scope>> scope_stack_;  // スコープスタック
};

}  // namespace semantic
}  // namespace cclint
