#include "constexpr_evaluator.hpp"

#include <cctype>
#include <charconv>
#include <regex>
#include <sstream>

#include "../parser/ast.hpp"

namespace cclint {
namespace semantic {

ConstexprValue ConstexprEvaluator::evaluate_expression(const std::string& expr) {
    if (expr.empty()) {
        return std::monostate{};
    }

    // 前後の空白を削除
    std::string trimmed = expr;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

    // 整数リテラル
    if (auto value = evaluate_integer_literal(trimmed)) {
        return *value;
    }

    // 浮動小数点リテラル
    if (auto value = evaluate_float_literal(trimmed)) {
        return *value;
    }

    // 真偽値リテラル
    if (auto value = evaluate_bool_literal(trimmed)) {
        return *value;
    }

    // 文字列リテラル
    if (trimmed.size() >= 2 && trimmed.front() == '"' && trimmed.back() == '"') {
        return trimmed.substr(1, trimmed.size() - 2);
    }

    // TODO: 複雑な式の評価（演算子を含む式など）
    // 将来的には式パーサーと統合して評価

    return std::monostate{};
}

ConstexprValue ConstexprEvaluator::evaluate_node(const std::shared_ptr<parser::ASTNode>& node) {
    // 基本実装: VariableNodeの初期化式を評価
    // TODO: より多くのノードタイプに対応
    return std::monostate{};
}

std::optional<int64_t> ConstexprEvaluator::evaluate_integer_literal(const std::string& literal) {
    if (literal.empty()) {
        return std::nullopt;
    }

    std::string str = literal;

    // サフィックスを削除 (l, L, ll, LL, u, U, ul, UL, ull, ULL)
    while (!str.empty()) {
        char last = str.back();
        if (last == 'l' || last == 'L' || last == 'u' || last == 'U') {
            str.pop_back();
        } else {
            break;
        }
    }

    if (str.empty()) {
        return std::nullopt;
    }

    int64_t value = 0;
    int base = 10;

    // 進数判定
    if (str.size() >= 2) {
        if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
            // 16進数
            base = 16;
            str = str.substr(2);
        } else if (str[0] == '0' && (str[1] == 'b' || str[1] == 'B')) {
            // 2進数
            base = 2;
            str = str.substr(2);
        } else if (str[0] == '0' && std::isdigit(str[1])) {
            // 8進数
            base = 8;
            str = str.substr(1);
        }
    }

    // 数値桁区切り文字 (') を削除 (C++14)
    str.erase(std::remove(str.begin(), str.end(), '\''), str.end());

    // std::from_chars を使用してパース
    auto result = std::from_chars(str.data(), str.data() + str.size(), value, base);

    if (result.ec == std::errc{} && result.ptr == str.data() + str.size()) {
        return value;
    }

    return std::nullopt;
}

std::optional<double> ConstexprEvaluator::evaluate_float_literal(const std::string& literal) {
    if (literal.empty()) {
        return std::nullopt;
    }

    std::string str = literal;

    // サフィックスを削除 (f, F, l, L)
    if (!str.empty()) {
        char last = str.back();
        if (last == 'f' || last == 'F' || last == 'l' || last == 'L') {
            str.pop_back();
        }
    }

    // 数値桁区切り文字 (') を削除
    str.erase(std::remove(str.begin(), str.end(), '\''), str.end());

    try {
        size_t pos = 0;
        double value = std::stod(str, &pos);
        if (pos == str.size()) {
            return value;
        }
    } catch (...) {
        // パース失敗
    }

    return std::nullopt;
}

std::optional<bool> ConstexprEvaluator::evaluate_bool_literal(const std::string& literal) {
    if (literal == "true") {
        return true;
    } else if (literal == "false") {
        return false;
    }
    return std::nullopt;
}

std::string ConstexprEvaluator::value_to_string(const ConstexprValue& value) {
    return std::visit(
        [](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                return "<not evaluated>";
            } else if constexpr (std::is_same_v<T, int64_t>) {
                return std::to_string(arg);
            } else if constexpr (std::is_same_v<T, double>) {
                return std::to_string(arg);
            } else if constexpr (std::is_same_v<T, bool>) {
                return arg ? "true" : "false";
            } else if constexpr (std::is_same_v<T, std::string>) {
                return "\"" + arg + "\"";
            }
            return "<unknown>";
        },
        value);
}

bool ConstexprEvaluator::is_valid(const ConstexprValue& value) {
    return !std::holds_alternative<std::monostate>(value);
}

std::optional<int64_t> ConstexprEvaluator::get_integer(const ConstexprValue& value) {
    if (std::holds_alternative<int64_t>(value)) {
        return std::get<int64_t>(value);
    } else if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? 1 : 0;
    }
    return std::nullopt;
}

ConstexprValue ConstexprEvaluator::evaluate_binary_op(const std::string& op,
                                                      const ConstexprValue& left,
                                                      const ConstexprValue& right) {
    // 整数演算
    auto left_int = get_integer(left);
    auto right_int = get_integer(right);

    if (left_int && right_int) {
        if (op == "+")
            return *left_int + *right_int;
        if (op == "-")
            return *left_int - *right_int;
        if (op == "*")
            return *left_int * *right_int;
        if (op == "/" && *right_int != 0)
            return *left_int / *right_int;
        if (op == "%" && *right_int != 0)
            return *left_int % *right_int;
        if (op == "&")
            return *left_int & *right_int;
        if (op == "|")
            return *left_int | *right_int;
        if (op == "^")
            return *left_int ^ *right_int;
        if (op == "<<")
            return *left_int << *right_int;
        if (op == ">>")
            return *left_int >> *right_int;
        if (op == "==")
            return *left_int == *right_int;
        if (op == "!=")
            return *left_int != *right_int;
        if (op == "<")
            return *left_int < *right_int;
        if (op == "<=")
            return *left_int <= *right_int;
        if (op == ">")
            return *left_int > *right_int;
        if (op == ">=")
            return *left_int >= *right_int;
    }

    // 浮動小数点演算
    if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
        double left_val = std::get<double>(left);
        double right_val = std::get<double>(right);

        if (op == "+")
            return left_val + right_val;
        if (op == "-")
            return left_val - right_val;
        if (op == "*")
            return left_val * right_val;
        if (op == "/" && right_val != 0.0)
            return left_val / right_val;
    }

    return std::monostate{};
}

ConstexprValue ConstexprEvaluator::evaluate_unary_op(const std::string& op,
                                                     const ConstexprValue& operand) {
    auto int_val = get_integer(operand);
    if (int_val) {
        if (op == "+")
            return *int_val;
        if (op == "-")
            return -*int_val;
        if (op == "~")
            return ~*int_val;
        if (op == "!")
            return !*int_val;
    }

    if (std::holds_alternative<double>(operand)) {
        double val = std::get<double>(operand);
        if (op == "+")
            return val;
        if (op == "-")
            return -val;
    }

    return std::monostate{};
}

}  // namespace semantic
}  // namespace cclint
