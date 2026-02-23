#include "semantic/name_lookup.hpp"

#include <algorithm>
#include <unordered_set>

namespace cclint {
namespace semantic {

NameLookup::NameLookup(std::shared_ptr<SymbolTable> symbol_table,
                       std::shared_ptr<TypeSystem> type_system)
    : symbol_table_(symbol_table), type_system_(type_system) {}

std::shared_ptr<Symbol> NameLookup::lookup_with_adl(
    const std::string& function_name, const std::vector<std::shared_ptr<Type>>& argument_types) {

    // 1. 通常の名前検索を試みる
    auto symbol = symbol_table_->lookup(function_name);
    if (symbol && symbol->kind == SymbolKind::Function) {
        return symbol;
    }

    // 2. ADL: 引数の型から関連する名前空間を取得
    auto associated_namespaces = get_associated_namespaces(argument_types);

    // 3. 各名前空間で関数を検索
    for (const auto& ns_name : associated_namespaces) {
        if (ns_name.empty()) {
            // グローバルスコープは既に検索済み
            continue;
        }

        // 修飾名で検索: ns_name::function_name
        std::string qualified_name = ns_name + "::" + function_name;
        auto ns_symbol = symbol_table_->lookup_qualified(qualified_name);

        if (ns_symbol && ns_symbol->kind == SymbolKind::Function) {
            return ns_symbol;
        }
    }

    // 見つからない
    return nullptr;
}

std::vector<std::string> NameLookup::get_associated_namespaces(
    const std::vector<std::shared_ptr<Type>>& argument_types) {

    std::unordered_set<std::string> namespaces;

    for (const auto& type : argument_types) {
        if (!type) {
            continue;
        }

        // 型の名前空間を取得
        std::string ns = get_namespace_of_type(type);
        if (!ns.empty()) {
            namespaces.insert(ns);
        }

        // ポインタや参照の場合、指し示す型も確認
        if (type->kind() == TypeKind::Pointer) {
            auto ptr_type = std::dynamic_pointer_cast<PointerType>(type);
            if (ptr_type && ptr_type->pointee()) {
                std::string pointee_ns = get_namespace_of_type(ptr_type->pointee());
                if (!pointee_ns.empty()) {
                    namespaces.insert(pointee_ns);
                }
            }
        } else if (type->kind() == TypeKind::Reference ||
                   type->kind() == TypeKind::RValueReference) {
            auto ref_type = std::dynamic_pointer_cast<ReferenceType>(type);
            if (ref_type && ref_type->referenced()) {
                std::string ref_ns = get_namespace_of_type(ref_type->referenced());
                if (!ref_ns.empty()) {
                    namespaces.insert(ref_ns);
                }
            }
        }
    }

    return std::vector<std::string>(namespaces.begin(), namespaces.end());
}

std::string NameLookup::get_namespace_of_type(const std::shared_ptr<Type>& type) {
    if (!type) {
        return "";
    }

    // クラス型の場合
    if (type->kind() == TypeKind::Class) {
        auto class_type = std::dynamic_pointer_cast<ClassType>(type);
        if (class_type) {
            std::string type_name = class_type->to_string();

            // 型名に :: が含まれていれば、名前空間付き
            size_t last_colon = type_name.rfind("::");
            if (last_colon != std::string::npos) {
                return type_name.substr(0, last_colon);
            }
        }
    }

    // その他の型（組み込み型など）はグローバルスコープ
    return "";
}

std::shared_ptr<Symbol> NameLookup::resolve_overload(
    const std::vector<std::shared_ptr<Symbol>>& candidates,
    const std::vector<std::shared_ptr<Type>>& argument_types) {

    if (candidates.empty()) {
        return nullptr;
    }

    // 単一候補の場合はそれを返す
    if (candidates.size() == 1) {
        return candidates[0];
    }

    // 各候補の適合度を計算
    std::shared_ptr<Symbol> best_candidate = nullptr;
    int best_score = 0;

    for (const auto& candidate : candidates) {
        if (!candidate || candidate->kind != SymbolKind::Function) {
            continue;
        }

        int score = calculate_match_score(candidate, argument_types);

        if (score > best_score) {
            best_score = score;
            best_candidate = candidate;
        }
    }

    return best_candidate;
}

int NameLookup::calculate_match_score(const std::shared_ptr<Symbol>& function_symbol,
                                        const std::vector<std::shared_ptr<Type>>& argument_types) {

    if (!function_symbol || function_symbol->kind != SymbolKind::Function) {
        return 0;  // 不適合
    }

    // パラメータ数が一致しない場合は不適合
    if (function_symbol->parameters.size() != argument_types.size()) {
        return 0;
    }

    int total_score = 0;

    // 各引数の適合度を計算
    for (size_t i = 0; i < argument_types.size(); ++i) {
        if (!argument_types[i]) {
            continue;
        }

        // パラメータの型を取得
        std::string param_type_name = function_symbol->parameters[i];
        auto param_type = type_system_->resolve_type(param_type_name);

        if (!param_type) {
            // 型が解決できない場合はスキップ
            continue;
        }

        // 完全一致
        if (param_type->equals(*argument_types[i])) {
            total_score += 2;  // 完全一致は高スコア
        }
        // 型変換可能
        else if (argument_types[i]->convertible_to(*param_type)) {
            total_score += 1;  // 型変換可能は中スコア
        }
        // 不適合
        else {
            return 0;  // 1つでも不適合なら全体が不適合
        }
    }

    return total_score;
}

}  // namespace semantic
}  // namespace cclint
