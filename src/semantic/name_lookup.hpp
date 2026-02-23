#pragma once

#include <memory>
#include <string>
#include <vector>

#include "semantic/symbol_table.hpp"
#include "semantic/type_system.hpp"

namespace cclint {
namespace semantic {

/// 名前検索ユーティリティ
/// ADL (Argument-Dependent Lookup)などの高度な名前検索機能を提供
class NameLookup {
public:
    NameLookup(std::shared_ptr<SymbolTable> symbol_table,
               std::shared_ptr<TypeSystem> type_system);

    /// ADL (Argument-Dependent Lookup) を実行
    /// @param function_name 関数名
    /// @param argument_types 引数の型リスト
    /// @return 見つかった関数シンボル（複数の候補がある場合は最初の1つ）
    std::shared_ptr<Symbol> lookup_with_adl(const std::string& function_name,
                                             const std::vector<std::shared_ptr<Type>>& argument_types);

    /// 引数の型から関連する名前空間を取得
    /// @param argument_types 引数の型リスト
    /// @return 関連する名前空間のリスト
    std::vector<std::string> get_associated_namespaces(
        const std::vector<std::shared_ptr<Type>>& argument_types);

    /// 型が定義されている名前空間を取得
    /// @param type 型
    /// @return 名前空間名（グローバルスコープの場合は空文字列）
    std::string get_namespace_of_type(const std::shared_ptr<Type>& type);

private:
    std::shared_ptr<SymbolTable> symbol_table_;
    std::shared_ptr<TypeSystem> type_system_;
};

}  // namespace semantic
}  // namespace cclint
