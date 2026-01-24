#include "rules/builtin/naming_convention.hpp"
#include "parser/ast.hpp"
#include "utils/string_utils.hpp"

#include <sstream>

namespace cclint {
namespace rules {
namespace builtin {

NamingConventionRule::NamingConventionRule() {
    // デフォルトのパターン
    // 関数名: snake_case
    function_pattern_ = std::regex(R"([a-z][a-z0-9_]*)");

    // クラス名: PascalCase
    class_pattern_ = std::regex(R"([A-Z][a-zA-Z0-9]*)");

    // 変数名: snake_case
    variable_pattern_ = std::regex(R"([a-z][a-z0-9_]*)");

    // 定数名: UPPER_CASE
    constant_pattern_ = std::regex(R"([A-Z][A-Z0-9_]*)");

    // アクセス指定子ごとのメソッド命名（デフォルトは全て snake_case）
    public_method_pattern_ = std::regex(R"([a-z][a-z0-9_]*)");
    protected_method_pattern_ = std::regex(R"([a-z][a-z0-9_]*)");
    private_method_pattern_ = std::regex(R"([a-z][a-z0-9_]*)");
}

void NamingConventionRule::initialize(const RuleParameters& params) {
    // パラメータから設定を読み込む
    if (params.count("check_functions")) {
        check_functions_ = params.at("check_functions") == "true";
    }
    if (params.count("check_classes")) {
        check_classes_ = params.at("check_classes") == "true";
    }
    if (params.count("check_variables")) {
        check_variables_ = params.at("check_variables") == "true";
    }
    if (params.count("check_constants")) {
        check_constants_ = params.at("check_constants") == "true";
    }

    // カスタムパターンの設定
    if (params.count("function_pattern")) {
        function_pattern_ = std::regex(params.at("function_pattern"));
    }
    if (params.count("class_pattern")) {
        class_pattern_ = std::regex(params.at("class_pattern"));
    }

    // アクセス指定子ごとのメソッド命名パターン
    if (params.count("public_method_pattern")) {
        public_method_pattern_ = std::regex(params.at("public_method_pattern"));
        check_method_access_ = true;
    }
    if (params.count("protected_method_pattern")) {
        protected_method_pattern_ = std::regex(params.at("protected_method_pattern"));
        check_method_access_ = true;
    }
    if (params.count("private_method_pattern")) {
        private_method_pattern_ = std::regex(params.at("private_method_pattern"));
        check_method_access_ = true;
    }
}

void NamingConventionRule::check_file(
    const std::string& file_path,
    const std::string& content,
    diagnostic::DiagnosticEngine& diag_engine) {

    if (check_functions_) {
        check_function_names(file_path, content, diag_engine);
    }

    if (check_classes_) {
        check_class_names(file_path, content, diag_engine);
    }

    if (check_variables_) {
        check_variable_names(file_path, content, diag_engine);
    }

    if (check_constants_) {
        check_constant_names(file_path, content, diag_engine);
    }
}

void NamingConventionRule::check_function_names(
    const std::string& file_path,
    const std::string& content,
    diagnostic::DiagnosticEngine& diag_engine) {

    // 簡易的な関数名検出（正規表現ベース）
    // より正確にはAST解析が必要
    std::regex func_decl_pattern(
        R"(\b(void|int|bool|char|float|double|auto|[A-Za-z_][A-Za-z0-9_:<>]*)\s+([A-Za-z_][A-Za-z0-9_]*)\s*\()");

    auto lines = utils::StringUtils::split(content, '\n');
    int line_num = 0;

    for (const auto& line : lines) {
        line_num++;

        std::smatch match;
        std::string line_str(line);
        if (std::regex_search(line_str, match, func_decl_pattern)) {
            std::string func_name = match[2].str();

            // 特殊な名前は除外（コンストラクタ、デストラクタ、main等）
            if (func_name == "main" || func_name[0] == '~') {
                continue;
            }

            // snake_case チェック
            if (!std::regex_match(func_name, function_pattern_)) {
                std::string message =
                    "Function name '" + func_name +
                    "' does not follow snake_case convention";
                report_diagnostic(diag_engine, file_path, line_num, 1,
                                  message);
            }
        }
    }
}

void NamingConventionRule::check_class_names(
    const std::string& file_path,
    const std::string& content,
    diagnostic::DiagnosticEngine& diag_engine) {

    // 簡易的なクラス名検出
    std::regex class_decl_pattern(R"(\b(class|struct)\s+([A-Za-z_][A-Za-z0-9_]*))");

    auto lines = utils::StringUtils::split(content, '\n');
    int line_num = 0;

    for (const auto& line : lines) {
        line_num++;

        std::smatch match;
        std::string line_str(line);
        if (std::regex_search(line_str, match, class_decl_pattern)) {
            std::string class_name = match[2].str();

            // PascalCase チェック
            if (!std::regex_match(class_name, class_pattern_)) {
                std::string message =
                    "Class name '" + class_name +
                    "' does not follow PascalCase convention";
                report_diagnostic(diag_engine, file_path, line_num, 1,
                                  message);
            }
        }
    }
}

void NamingConventionRule::check_variable_names(
    const std::string& file_path,
    const std::string& content,
    diagnostic::DiagnosticEngine& diag_engine) {

    // 簡易的な変数名検出
    // 変数宣言のパターン: 型 変数名; または 型 変数名 = ...;
    std::regex var_decl_pattern(
        R"(\b(int|bool|char|float|double|auto|std::\w+|[A-Za-z_][A-Za-z0-9_:<>]*)\s+([a-z_][A-Za-z0-9_]*)\s*[;=])");

    auto lines = utils::StringUtils::split(content, '\n');
    int line_num = 0;

    for (const auto& line : lines) {
        line_num++;

        // コメント行はスキップ
        std::string trimmed = utils::StringUtils::trim(line);
        if (trimmed.rfind("//", 0) == 0 || trimmed.rfind("/*", 0) == 0) {
            continue;
        }

        std::smatch match;
        std::string line_str(line);
        auto it = std::sregex_iterator(line_str.begin(), line_str.end(),
                                        var_decl_pattern);
        auto end = std::sregex_iterator();

        for (; it != end; ++it) {
            std::string var_name = (*it)[2].str();

            // 特殊なキーワードは除外
            if (var_name == "if" || var_name == "for" ||
                var_name == "while" || var_name == "return") {
                continue;
            }

            // snake_case チェック
            if (!std::regex_match(var_name, variable_pattern_)) {
                std::string message =
                    "Variable name '" + var_name +
                    "' does not follow snake_case convention";
                report_diagnostic(diag_engine, file_path, line_num, 1,
                                  message);
            }
        }
    }
}

void NamingConventionRule::check_constant_names(
    const std::string& file_path,
    const std::string& content,
    diagnostic::DiagnosticEngine& diag_engine) {

    // 簡易的な定数名検出
    // const/constexpr 定数のパターン
    std::regex const_decl_pattern(
        R"(\b(const|constexpr|#define)\s+(?:[A-Za-z_][A-Za-z0-9_:<>]*\s+)?([A-Z_][A-Z0-9_]*))");

    auto lines = utils::StringUtils::split(content, '\n');
    int line_num = 0;

    for (const auto& line : lines) {
        line_num++;

        std::smatch match;
        std::string line_str(line);
        if (std::regex_search(line_str, match, const_decl_pattern)) {
            std::string const_name = match[2].str();

            // 除外する名前（型名等）
            if (const_name.length() < 2) {
                continue;
            }

            // UPPER_CASE チェック
            if (!std::regex_match(const_name, constant_pattern_)) {
                std::string message =
                    "Constant name '" + const_name +
                    "' does not follow UPPER_CASE convention";
                report_diagnostic(diag_engine, file_path, line_num, 1,
                                  message);
            }
        }
    }
}

// ASTベースのチェック（アクセス指定子対応）
void NamingConventionRule::check_ast(
    const std::string& file_path,
    std::shared_ptr<parser::TranslationUnitNode> ast,
    diagnostic::DiagnosticEngine& diag_engine) {

    if (!check_method_access_) {
        return;  // アクセス指定子チェックが無効な場合はスキップ
    }

    check_ast_recursive(file_path, ast, diag_engine);
}

void NamingConventionRule::check_ast_recursive(
    const std::string& file_path,
    std::shared_ptr<parser::ASTNode> node,
    diagnostic::DiagnosticEngine& diag_engine) {

    if (!node) {
        return;
    }

    // クラスノードの場合、メソッドをチェック
    if (node->type == parser::ASTNodeType::Class) {
        auto class_node = std::dynamic_pointer_cast<parser::ClassNode>(node);
        if (class_node) {
            check_class_methods(file_path, class_node, diag_engine);
        }
    }

    // 子ノードを再帰的にチェック
    for (const auto& child : node->children) {
        check_ast_recursive(file_path, child, diag_engine);
    }
}

void NamingConventionRule::check_class_methods(
    const std::string& file_path,
    std::shared_ptr<parser::ClassNode> class_node,
    diagnostic::DiagnosticEngine& diag_engine) {

    for (const auto& child : class_node->children) {
        // メソッドノードの場合
        if (child->type == parser::ASTNodeType::Function ||
            child->type == parser::ASTNodeType::Method) {

            auto func_node = std::dynamic_pointer_cast<parser::FunctionNode>(child);
            if (!func_node) {
                continue;
            }

            std::string method_name = func_node->name;

            // コンストラクタ・デストラクタは除外
            if (method_name == class_node->name ||
                method_name[0] == '~') {
                continue;
            }

            // operator オーバーロードは除外
            if (method_name.find("operator") == 0) {
                continue;
            }

            // アクセス指定子に応じたパターンチェック
            std::regex* pattern = nullptr;
            std::string access_name;

            switch (func_node->access) {
                case parser::AccessSpecifier::Public:
                    pattern = &public_method_pattern_;
                    access_name = "public";
                    break;
                case parser::AccessSpecifier::Protected:
                    pattern = &protected_method_pattern_;
                    access_name = "protected";
                    break;
                case parser::AccessSpecifier::Private:
                    pattern = &private_method_pattern_;
                    access_name = "private";
                    break;
                default:
                    continue;  // Noneの場合はスキップ
            }

            if (pattern && !std::regex_match(method_name, *pattern)) {
                std::ostringstream msg;
                msg << access_name << " method '" << method_name
                    << "' in class '" << class_node->name
                    << "' does not follow the configured naming pattern";

                report_diagnostic(diag_engine, file_path,
                                func_node->position.line,
                                func_node->position.column, msg.str());
            }
        }
    }
}

} // namespace builtin
} // namespace rules
} // namespace cclint
