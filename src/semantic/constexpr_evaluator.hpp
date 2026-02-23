#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>

namespace cclint {
namespace parser {
class ASTNode;
}  // namespace parser

namespace semantic {

/// Constexpr評価結果の値型
using ConstexprValue = std::variant<
    std::monostate,  // 評価不可
    int64_t,         // 整数値
    double,          // 浮動小数点数値
    bool,            // 真偽値
    std::string      // 文字列リテラル
>;

/// Constexpr評価器
/// C++のconstexpr式を評価して定数値を計算する
class ConstexprEvaluator {
public:
    ConstexprEvaluator() = default;
    ~ConstexprEvaluator() = default;

    /// 変数初期化式を評価
    /// @param init_expr 初期化式（文字列）
    /// @return 評価結果（失敗時は std::monostate）
    ConstexprValue evaluate_expression(const std::string& expr);

    /// ASTノードから初期化式を評価
    /// @param node ASTノード
    /// @return 評価結果
    ConstexprValue evaluate_node(const std::shared_ptr<parser::ASTNode>& node);

    /// 整数リテラルを評価
    std::optional<int64_t> evaluate_integer_literal(const std::string& literal);

    /// 浮動小数点リテラルを評価
    std::optional<double> evaluate_float_literal(const std::string& literal);

    /// 真偽値リテラルを評価
    std::optional<bool> evaluate_bool_literal(const std::string& literal);

    /// ConstexprValueを文字列に変換
    static std::string value_to_string(const ConstexprValue& value);

    /// ConstexprValueが有効な値か（std::monostateでないか）
    static bool is_valid(const ConstexprValue& value);

    /// ConstexprValueから整数値を取得（可能なら）
    static std::optional<int64_t> get_integer(const ConstexprValue& value);

private:
    /// 単純な2項演算を評価
    ConstexprValue evaluate_binary_op(const std::string& op, const ConstexprValue& left,
                                       const ConstexprValue& right);

    /// 単純な単項演算を評価
    ConstexprValue evaluate_unary_op(const std::string& op, const ConstexprValue& operand);
};

}  // namespace semantic
}  // namespace cclint
